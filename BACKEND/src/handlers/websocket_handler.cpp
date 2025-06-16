#include "../include/handlers/websocket_handler.h"
#include "../include/services/chat_service.h"
#include "../include/services/auth_service.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

WebSocketHandler::WebSocketHandler(std::shared_ptr<ChatService> chat_service, 
                                 std::shared_ptr<AuthService> auth_service)
    : chat_service(chat_service), auth_service(auth_service) {}

void WebSocketHandler::on_open(crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients[&conn] = std::make_unique<ClientConnection>();
    clients[&conn]->conn = &conn;
    clients[&conn]->user_id = 0; // 未认证
    clients[&conn]->connected_at = std::time(nullptr);
    
    std::cout << "WebSocket connection opened" << std::endl;
}

void WebSocketHandler::on_close(crow::websocket::connection& conn, const std::string& reason) {
    cleanup_connection(conn);
    std::cout << "WebSocket connection closed: " << reason << std::endl;
}

void WebSocketHandler::on_message(crow::websocket::connection& conn, const std::string& data, bool is_binary) {
    if (is_binary) return;
    
    std::cout << "Received WebSocket message: " << data << std::endl;
    
    try {
        json msg = json::parse(data);
        std::string type = msg["type"];
        
        std::cout << "Message type: " << type << std::endl;
        
        if (type == "auth") {
            // 认证消息
            std::string token = msg["token"];
            authenticate_connection(conn, token);
        } else if (type == "chat") {
            // 聊天消息
            std::string message_content = msg.contains("content") ? msg["content"] : msg["message"];
            handle_chat_message(conn, message_content);
        } else if (type == "private") {
            // 私聊消息
            handle_private_message(conn, data);
        } else if (type == "status") {
            // 状态更改
            handle_status_change(conn, msg["status"]);
        } else if (type == "recall") {
            // 撤回消息
            handle_recall_message(conn, msg["message_id"]);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing WebSocket message: " << e.what() << std::endl;
    }
}

bool WebSocketHandler::authenticate_connection(crow::websocket::connection& conn, const std::string& token) {
    auto validation_result = auth_service->validate_token(token);
    
    if (!validation_result.valid) {
        // 认证失败，关闭连接
        json error_msg = {
            {"type", "error"},
            {"message", "Authentication failed"}
        };
        send_to_connection(&conn, error_msg.dump());
        conn.close("Authentication failed");
        return false;
    }
    
    // 认证成功
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (clients.find(&conn) != clients.end()) {
        clients[&conn]->user_id = validation_result.user_id;
        clients[&conn]->username = validation_result.username;
        user_connections[validation_result.user_id] = &conn;
        
        // 添加到在线用户列表
        chat_service->add_online_user(validation_result.user_id);
        
        // 发送认证成功消息
        json success_msg = {
            {"type", "auth_success"},
            {"message", "Authentication successful"}
        };
        send_to_connection(&conn, success_msg.dump());
        
        // 广播用户加入消息
        chat_service->send_user_join_notification(validation_result.username);
        
        // 发送在线用户列表
        auto online_users = chat_service->get_online_users_list();
        json user_list_msg = {
            {"type", "user_list"},
            {"users", json::array()}
        };
        
        for (const auto& user : online_users) {
            user_list_msg["users"].push_back({
                {"id", user.id},
                {"username", user.username},
                {"status", User::status_to_string(user.status)}
            });
        }
        
        broadcast_message(user_list_msg.dump());
        
        return true;
    }
    
    return false;
}

void WebSocketHandler::handle_chat_message(crow::websocket::connection& conn, const std::string& content) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(&conn);
    
    if (it == clients.end() || it->second->user_id == 0) {
        // 未认证用户
        return;
    }
    
    auto client = it->second.get();
    auto result = chat_service->send_message(client->user_id, content);
    
    if (result.success && result.processed_message) {
        // 广播消息给所有连接的客户端
        json broadcast_msg = {
            {"type", "message"},
            {"message", {
                {"id", result.processed_message->id},
                {"sender_id", result.processed_message->sender_id},
                {"sender_username", client->username},
                {"content", result.processed_message->content},
                {"timestamp", result.processed_message->timestamp},
                {"type", Message::type_to_string(result.processed_message->type)}
            }}
        };
        
        broadcast_message(broadcast_msg.dump(), client->user_id);
    }
}

void WebSocketHandler::handle_private_message(crow::websocket::connection& conn, const std::string& message) {
    try {
        json msg = json::parse(message);
        
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = clients.find(&conn);
        
        if (it == clients.end() || it->second->user_id == 0) return;
        
        auto client = it->second.get();
        int receiver_id = msg["receiver_id"];
        std::string content = msg["content"];
        
        auto result = chat_service->send_message(client->user_id, content, MessageType::PRIVATE, receiver_id);
        
        if (result.success && result.processed_message) {
            json private_msg = {
                {"type", "private_message"},
                {"message", {
                    {"id", result.processed_message->id},
                    {"sender_id", result.processed_message->sender_id},
                    {"receiver_id", result.processed_message->receiver_id},
                    {"sender_username", client->username},
                    {"content", result.processed_message->content},
                    {"timestamp", result.processed_message->timestamp}
                }}
            };
            
            // 发送给接收者
            send_to_user(receiver_id, private_msg.dump());
            // 也发送给发送者（确认消息）
            send_to_connection(&conn, private_msg.dump());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling private message: " << e.what() << std::endl;
    }
}

void WebSocketHandler::handle_status_change(crow::websocket::connection& conn, const std::string& status) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(&conn);
    
    if (it == clients.end() || it->second->user_id == 0) return;
    
    auto client = it->second.get();
    UserStatus user_status = User::string_to_status(status);
    
    if (auth_service->update_user_status(client->user_id, user_status)) {
        // 广播状态更新
        json status_msg = {
            {"type", "status_update"},
            {"user_id", client->user_id},
            {"username", client->username},
            {"status", status}
        };
        
        broadcast_message(status_msg.dump());
    }
}

void WebSocketHandler::handle_recall_message(crow::websocket::connection& conn, int message_id) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = clients.find(&conn);
    
    if (it == clients.end() || it->second->user_id == 0) return;
    
    auto client = it->second.get();
    
    if (chat_service->recall_message(message_id, client->user_id)) {
        // 广播消息撤回
        json recall_msg = {
            {"type", "message_recalled"},
            {"message_id", message_id}
        };
        
        broadcast_message(recall_msg.dump());
    }
}

void WebSocketHandler::broadcast_message(const std::string& message, int exclude_user_id) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    for (const auto& pair : clients) {
        auto client = pair.second.get();
        if (client->user_id != 0 && client->user_id != exclude_user_id) {
            client->conn->send_text(message);
        }
    }
}

void WebSocketHandler::send_to_user(int user_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    auto it = user_connections.find(user_id);
    if (it != user_connections.end()) {
        it->second->send_text(message);
    }
}

void WebSocketHandler::send_to_connection(crow::websocket::connection* conn, const std::string& message) {
    if (conn) {
        conn->send_text(message);
    }
}

void WebSocketHandler::disconnect_user(int user_id) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    auto it = user_connections.find(user_id);
    if (it != user_connections.end()) {
        it->second->close("User disconnected");
    }
}

std::vector<std::string> WebSocketHandler::get_connected_users() {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::vector<std::string> usernames;
    
    for (const auto& pair : clients) {
        if (pair.second->user_id != 0) {
            usernames.push_back(pair.second->username);
        }
    }
    
    return usernames;
}

void WebSocketHandler::cleanup_connection(crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    auto it = clients.find(&conn);
    if (it != clients.end()) {
        auto client = it->second.get();
        
        if (client->user_id != 0) {
            // 从在线用户列表移除
            chat_service->remove_online_user(client->user_id);
            
            // 广播用户离开消息
            chat_service->send_user_leave_notification(client->username);
            
            // 从用户连接映射中移除
            user_connections.erase(client->user_id);
        }
        
        clients.erase(it);
    }
}
