#include <crow.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

#include "database.h"
#include "auth.h"
#include "content_filter.h"
#include "websocket_manager.h"

using json = nlohmann::json;

// 全局对象
std::unique_ptr<Database> db;
std::unique_ptr<AuthManager> auth_manager;
std::unique_ptr<ContentFilter> content_filter;
std::unique_ptr<WebSocketManager> ws_manager;

// 辅助函数：添加CORS头
crow::response create_cors_response(int code, const std::string& body) {
    crow::response res(code, body);
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res.add_header("Content-Type", "application/json");
    return res;
}

// 定时任务：清理过期数据
void cleanup_task() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
        
        if (auth_manager) {
            auth_manager->cleanup_expired_sessions();
        }
        
        if (db) {
            db->cleanup_old_messages(3); // 清理3天前的消息
        }
    }
}

int main() {
    // 初始化全局对象
    db = std::make_unique<Database>("chatroom.db");
    auth_manager = std::make_unique<AuthManager>();
    content_filter = std::make_unique<ContentFilter>();
    ws_manager = std::make_unique<WebSocketManager>();
    
    if (!db->initialize()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    // 启动清理任务
    std::thread cleanup_thread(cleanup_task);
    cleanup_thread.detach();
    
    crow::SimpleApp app;
    
    // 健康检查
    CROW_ROUTE(app, "/api/health")
    ([](const crow::request& req) {
        json response;
        response["status"] = "ok";
        response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        response["online_users"] = ws_manager->get_online_count();
        return create_cors_response(200, response.dump());
    });
    
    // 用户注册
    CROW_ROUTE(app, "/api/register").methods("POST"_method)
    ([](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string username = body["username"];
            std::string password = body["password"];
            std::string email = body.value("email", "");
            
            if (username.empty() || password.empty()) {
                json error;
                error["error"] = "Username and password are required";
                return crow::response(400, error.dump());
            }
            
            // 检查用户名是否已存在
            auto existing_user = db->get_user_by_username(username);
            if (existing_user) {
                json error;
                error["error"] = "Username already exists";
                return crow::response(409, error.dump());
            }
            
            // 创建用户
            std::string password_hash = auth_manager->hash_user_password(password);
            if (db->create_user(username, password_hash, email)) {
                json response;
                response["message"] = "User created successfully";
                return crow::response(201, response.dump());
            } else {
                json error;
                error["error"] = "Failed to create user";
                return crow::response(500, error.dump());
            }
        } catch (const std::exception& e) {
            json error;
            error["error"] = "Invalid JSON format";
            return crow::response(400, error.dump());
        }
    });
    
    // 用户登录
    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([](const crow::request& req) {
        try {
            auto body = json::parse(req.body);
            std::string username = body["username"];
            std::string password = body["password"];
            
            if (username.empty() || password.empty()) {
                json error;
                error["error"] = "Username and password are required";
                return crow::response(400, error.dump());
            }
            
            auto user = db->get_user_by_username(username);
            if (!user || !auth_manager->verify_user_password(password, user->password_hash)) {
                json error;
                error["error"] = "Invalid username or password";
                return crow::response(401, error.dump());
            }
            
            if (!user->is_active) {
                json error;
                error["error"] = "Account is deactivated";
                return crow::response(403, error.dump());
            }
            
            // 更新最后登录时间
            db->update_last_login(user->id);
            
            // 创建会话
            std::string token = auth_manager->create_session(user->id, user->username);
            
            json response;
            response["token"] = token;
            response["user"] = {
                {"id", user->id},
                {"username", user->username},
                {"email", user->email},
                {"accept_messages", user->accept_messages}
            };
            
            return crow::response(200, response.dump());
        } catch (const std::exception& e) {
            json error;
            error["error"] = "Invalid JSON format";
            return crow::response(400, error.dump());
        }
    });
    
    // 用户登出
    CROW_ROUTE(app, "/api/logout").methods("POST"_method)
    ([](const crow::request& req) {
        std::string auth_header = req.get_header_value("Authorization");
        if (!auth_header.empty() && auth_header.substr(0, 7) == "Bearer ") {
            std::string token = auth_header.substr(7);
            auth_manager->destroy_session(token);
        }
        
        json response;
        response["message"] = "Logged out successfully";
        return crow::response(200, response.dump());
    });
    
    // 获取用户设置
    CROW_ROUTE(app, "/api/user/settings")
    ([](const crow::request& req) {
        // 手动验证认证
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            json error;
            error["error"] = "Authentication required";
            return crow::response(401, error.dump());
        }
        
        std::string token = auth_header.substr(7);
        if (!auth_manager->validate_session(token)) {
            json error;
            error["error"] = "Invalid or expired token";
            return crow::response(401, error.dump());
        }
        
        int user_id = auth_manager->get_user_id_from_session(token);
        auto user = db->get_user_by_id(user_id);
        if (!user) {
            json error;
            error["error"] = "User not found";
            return crow::response(404, error.dump());
        }
        
        json response;
        response["accept_messages"] = user->accept_messages;
        return crow::response(200, response.dump());
    });
    
    // 更新用户设置
    CROW_ROUTE(app, "/api/user/settings").methods("PUT"_method)
    ([](const crow::request& req) {
        // 手动验证认证
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            json error;
            error["error"] = "Authentication required";
            return crow::response(401, error.dump());
        }
        
        std::string token = auth_header.substr(7);
        if (!auth_manager->validate_session(token)) {
            json error;
            error["error"] = "Invalid or expired token";
            return crow::response(401, error.dump());
        }
        
        int user_id = auth_manager->get_user_id_from_session(token);
        
        try {
            auto body = json::parse(req.body);
            bool accept_messages = body.value("accept_messages", true);
            
            if (db->update_user_settings(user_id, accept_messages)) {
                json response;
                response["message"] = "Settings updated successfully";
                return crow::response(200, response.dump());
            } else {
                json error;
                error["error"] = "Failed to update settings";
                return crow::response(500, error.dump());
            }
        } catch (const std::exception& e) {
            json error;
            error["error"] = "Invalid JSON format";
            return crow::response(400, error.dump());
        }
    });
    
    // 获取历史消息
    CROW_ROUTE(app, "/api/messages")
    ([](const crow::request& req) {
        // 手动验证认证
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            json error;
            error["error"] = "Authentication required";
            return crow::response(401, error.dump());
        }
        
        std::string token = auth_header.substr(7);
        if (!auth_manager->validate_session(token)) {
            json error;
            error["error"] = "Invalid or expired token";
            return crow::response(401, error.dump());
        }
        
        std::string limit_str = req.url_params.get("limit");
        int limit = limit_str.empty() ? 50 : std::stoi(limit_str);
        
        auto messages = db->get_recent_messages(limit);
        
        json response = json::array();
        for (const auto& msg : messages) {
            json msg_json;
            msg_json["id"] = msg.id;
            msg_json["sender_id"] = msg.sender_id;
            msg_json["sender_username"] = msg.sender_username;
            msg_json["content"] = msg.content;
            msg_json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                msg.created_at.time_since_epoch()).count();
            response.push_back(msg_json);
        }
        
        return crow::response(200, response.dump());
    });
    
    // 删除消息
    CROW_ROUTE(app, "/api/messages/<int>").methods("DELETE"_method)
    ([](const crow::request& req, int message_id) {
        // 手动验证认证
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            json error;
            error["error"] = "Authentication required";
            return crow::response(401, error.dump());
        }
        
        std::string token = auth_header.substr(7);
        if (!auth_manager->validate_session(token)) {
            json error;
            error["error"] = "Invalid or expired token";
            return crow::response(401, error.dump());
        }
        
        int user_id = auth_manager->get_user_id_from_session(token);
        std::string username = auth_manager->get_username_from_session(token);
        
        if (db->delete_message(message_id, user_id)) {
            // 通知所有用户消息被删除
            json notification;
            notification["type"] = "message_deleted";
            notification["message_id"] = message_id;
            notification["deleted_by"] = username;
            ws_manager->broadcast_message(notification.dump());
            
            json response;
            response["message"] = "Message deleted successfully";
            return crow::response(200, response.dump());
        } else {
            json error;
            error["error"] = "Failed to delete message or message not found";
            return crow::response(404, error.dump());
        }
    });
    
    // 获取在线用户
    CROW_ROUTE(app, "/api/users/online")
    ([](const crow::request& req) {
        // 手动验证认证
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            json error;
            error["error"] = "Authentication required";
            return crow::response(401, error.dump());
        }
        
        std::string token = auth_header.substr(7);
        if (!auth_manager->validate_session(token)) {
            json error;
            error["error"] = "Invalid or expired token";
            return crow::response(401, error.dump());
        }
        
        auto online_users = ws_manager->get_online_users();
        
        json response;
        response["count"] = ws_manager->get_online_count();
        response["users"] = online_users;
        return crow::response(200, response.dump());
    });
    
    // WebSocket连接
    CROW_ROUTE(app, "/ws")
    .websocket(&app)
    .onopen([&](crow::websocket::connection& conn) {
        std::cout << "WebSocket connection opened" << std::endl;
    })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
        std::cout << "WebSocket connection closed: " << reason << std::endl;
        ws_manager->remove_connection(&conn);
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        try {
            auto message = json::parse(data);
            std::string type = message["type"];
            
            if (type == "auth") {
                std::string token = message["token"];
                if (auth_manager->validate_session(token)) {
                    int user_id = auth_manager->get_user_id_from_session(token);
                    std::string username = auth_manager->get_username_from_session(token);
                    
                    ws_manager->add_connection(&conn, user_id, username);
                    
                    json auth_response;
                    auth_response["type"] = "auth_success";
                    auth_response["username"] = username;
                    conn.send_text(auth_response.dump());
                } else {
                    json auth_error;
                    auth_error["type"] = "auth_error";
                    auth_error["message"] = "Invalid token";
                    conn.send_text(auth_error.dump());
                    conn.close("Authentication failed");
                }
            } else if (type == "chat_message") {
                std::string token = message["token"];
                std::string content = message["content"];
                
                if (!auth_manager->validate_session(token)) {
                    json error;
                    error["type"] = "error";
                    error["message"] = "Authentication required";
                    conn.send_text(error.dump());
                    return;
                }
                
                int user_id = auth_manager->get_user_id_from_session(token);
                std::string username = auth_manager->get_username_from_session(token);
                
                // 检查用户是否允许发送消息
                auto user = db->get_user_by_id(user_id);
                if (!user || !user->is_active) {
                    json error;
                    error["type"] = "error";
                    error["message"] = "Account is not active";
                    conn.send_text(error.dump());
                    return;
                }
                
                // 过滤敏感内容
                std::string original_content = content;
                std::string filtered_content = content_filter->filter_content(content);
                
                // 保存消息到数据库
                if (db->save_message(user_id, filtered_content, original_content)) {
                    // 广播消息给所有用户
                    json broadcast_msg;
                    broadcast_msg["type"] = "chat_message";
                    broadcast_msg["sender_id"] = user_id;
                    broadcast_msg["sender_username"] = username;
                    broadcast_msg["content"] = filtered_content;
                    broadcast_msg["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    
                    if (content_filter->contains_sensitive_content(original_content)) {
                        broadcast_msg["filtered"] = true;
                    }
                    
                    ws_manager->broadcast_message(broadcast_msg.dump());
                }
            }
        } catch (const std::exception& e) {
            json error;
            error["type"] = "error";
            error["message"] = "Invalid message format";
            conn.send_text(error.dump());
        }
    });
    
    std::cout << "Starting Chat Room Server on port 8080..." << std::endl;
    app.port(8080).multithreaded().run();
    
    return 0;
}
