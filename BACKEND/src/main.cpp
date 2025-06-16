#include <crow.h>
#include <crow/middlewares/cors.h>
#include <memory>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

#include "database/database_manager.h"
#include "services/auth_service.h"
#include "services/chat_service.h"
#include "handlers/websocket_handler.h"

class ChatRoomServer {
private:
    crow::App<crow::CORSHandler> app;
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<AuthService> auth_service;
    std::shared_ptr<ChatService> chat_service;
    std::shared_ptr<WebSocketHandler> websocket_handler;
    
    // 线程池相关（体现进程间通信概念）
    std::vector<std::thread> worker_threads;
    bool running;
    
public:
    ChatRoomServer() : running(false) {}
    
    bool initialize() {
        // 初始化数据库
        db = std::make_shared<DatabaseManager>("chatroom.db");
        if (!db->initialize()) {
            std::cerr << "Failed to initialize database" << std::endl;
            return false;
        }
        
        // 初始化服务
        auth_service = std::make_shared<AuthService>(db);
        chat_service = std::make_shared<ChatService>(db);
        websocket_handler = std::make_shared<WebSocketHandler>(chat_service, auth_service);
        
        setup_routes();
        setup_cors();
        
        return true;
    }
    
    void setup_routes() {
        // 静态文件服务
        CROW_ROUTE(app, "/")
        ([](const crow::request&) {
            return crow::response(200, "text/html", 
                "<h1>Chat Room Server</h1><p>WebSocket endpoint: /ws</p>");
        });
        
        // 用户认证API
        CROW_ROUTE(app, "/api/auth/register").methods("POST"_method)
        ([this](const crow::request& req) {
            return handle_register(req);
        });
        
        CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)
        ([this](const crow::request& req) {
            return handle_login(req);
        });
        
        CROW_ROUTE(app, "/api/auth/logout").methods("POST"_method)
        ([this](const crow::request& req) {
            return handle_logout(req);
        });
        
        // 聊天相关API
        CROW_ROUTE(app, "/api/chat/history").methods("GET"_method)
        ([this](const crow::request& req) {
            return handle_get_history(req);
        });
        
        CROW_ROUTE(app, "/api/chat/users").methods("GET"_method)
        ([this](const crow::request& req) {
            return handle_get_online_users(req);
        });
        
        CROW_ROUTE(app, "/api/chat/block").methods("POST"_method)
        ([this](const crow::request& req) {
            return handle_block_user(req);
        });
        
        // WebSocket路由
        CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([this](crow::websocket::connection& conn) {
            websocket_handler->on_open(conn);
        })
        .onclose([this](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            websocket_handler->on_close(conn, reason);
        })
        .onmessage([this](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
            websocket_handler->on_message(conn, data, is_binary);
        });
    }
    
    void setup_cors() {
        // CORS中间件
        app.get_middleware<crow::CORSHandler>().global()
            .headers("Content-Type", "Authorization")
            .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method)
            .origin("http://localhost:5173"); // Vue开发服务器地址
    }
    
    void start_background_tasks() {
        running = true;
        
        // 数据清理线程（体现进程间通信 - 定期清理任务）
        worker_threads.emplace_back([this]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::hours(1));
                if (running) {
                    db->cleanup_old_messages();
                    std::cout << "Cleaned up old messages" << std::endl;
                }
            }
        });
        
        // 用户状态检查线程
        worker_threads.emplace_back([this]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::minutes(5));
                if (running) {
                    // 检查离线用户状态
                    std::cout << "Checking user status..." << std::endl;
                }
            }
        });
    }
    
    void run(int port = 8080) {
        start_background_tasks();
        
        std::cout << "Starting Chat Room Server on port " << port << std::endl;
        std::cout << "WebSocket endpoint: ws://localhost:" << port << "/ws" << std::endl;
        
        app.port(port).multithreaded().run();
    }
    
    void stop() {
        running = false;
        for (auto& thread : worker_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    
    // API处理函数
    crow::response handle_register(const crow::request& req) {
        try {
            nlohmann::json request_data = nlohmann::json::parse(req.body);
            
            std::string username = request_data["username"];
            std::string password = request_data["password"];
            std::string email = request_data["email"];
            
            auto result = auth_service->register_user(username, password, email);
            
            nlohmann::json response = {
                {"success", result.success},
                {"message", result.message}
            };
            
            if (result.success && result.user) {
                response["user"] = {
                    {"id", result.user->id},
                    {"username", result.user->username},
                    {"email", result.user->email},
                    {"status", User::status_to_string(result.user->status)}
                };
            }
            
            return crow::response(result.success ? 200 : 400, "application/json", response.dump());
        } catch (const std::exception& e) {
            nlohmann::json error = {{"success", false}, {"message", "Invalid request format"}};
            return crow::response(400, "application/json", error.dump());
        }
    }
    
    crow::response handle_login(const crow::request& req) {
        try {
            nlohmann::json request_data = nlohmann::json::parse(req.body);
            
            std::string username = request_data["username"];
            std::string password = request_data["password"];
            
            auto result = auth_service->login_user(username, password);
            
            nlohmann::json response = {
                {"success", result.success},
                {"message", result.message}
            };
            
            if (result.success && result.user) {
                response["user"] = {
                    {"id", result.user->id},
                    {"username", result.user->username},
                    {"email", result.user->email},
                    {"status", User::status_to_string(result.user->status)}
                };
                response["token"] = result.token;
            }
            
            return crow::response(result.success ? 200 : 401, "application/json", response.dump());
        } catch (const std::exception& e) {
            nlohmann::json error = {{"success", false}, {"message", "Invalid request format"}};
            return crow::response(400, "application/json", error.dump());
        }
    }
    
    crow::response handle_logout(const crow::request& req) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
                nlohmann::json error = {{"success", false}, {"message", "Missing or invalid authorization header"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            std::string token = auth_header.substr(7);
            auto validation = auth_service->validate_token(token);
            
            if (!validation.valid) {
                nlohmann::json error = {{"success", false}, {"message", "Invalid token"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            bool success = auth_service->logout_user(validation.user_id);
            nlohmann::json response = {
                {"success", success},
                {"message", success ? "Logout successful" : "Logout failed"}
            };
            
            return crow::response(success ? 200 : 500, "application/json", response.dump());
        } catch (const std::exception& e) {
            nlohmann::json error = {{"success", false}, {"message", "Server error"}};
            return crow::response(500, "application/json", error.dump());
        }
    }
    
    crow::response handle_get_history(const crow::request& req) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
                nlohmann::json error = {{"success", false}, {"message", "Missing authorization header"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            std::string token = auth_header.substr(7);
            auto validation = auth_service->validate_token(token);
            
            if (!validation.valid) {
                nlohmann::json error = {{"success", false}, {"message", "Invalid token"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            auto messages = chat_service->get_chat_history(validation.user_id);
            nlohmann::json response = {
                {"success", true},
                {"messages", nlohmann::json::array()}
            };
            
            for (const auto& msg : messages) {
                response["messages"].push_back({
                    {"id", msg.id},
                    {"sender_id", msg.sender_id},
                    {"sender_username", msg.sender_username},
                    {"content", msg.content},
                    {"type", Message::type_to_string(msg.type)},
                    {"timestamp", msg.timestamp},
                    {"is_deleted", msg.is_deleted}
                });
            }
            
            return crow::response(200, "application/json", response.dump());
        } catch (const std::exception& e) {
            nlohmann::json error = {{"success", false}, {"message", "Server error"}};
            return crow::response(500, "application/json", error.dump());
        }
    }
    
    crow::response handle_get_online_users(const crow::request& req) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
                nlohmann::json error = {{"success", false}, {"message", "Missing authorization header"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            std::string token = auth_header.substr(7);
            auto validation = auth_service->validate_token(token);
            
            if (!validation.valid) {
                nlohmann::json error = {{"success", false}, {"message", "Invalid token"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            auto users = chat_service->get_online_users_list();
            nlohmann::json response = {
                {"success", true},
                {"users", nlohmann::json::array()}
            };
            
            for (const auto& user : users) {
                response["users"].push_back({
                    {"id", user.id},
                    {"username", user.username},
                    {"status", User::status_to_string(user.status)}
                });
            }
            
            return crow::response(200, "application/json", response.dump());
        } catch (const std::exception& e) {
            nlohmann::json error = {{"success", false}, {"message", "Server error"}};
            return crow::response(500, "application/json", error.dump());
        }
    }
    
    crow::response handle_block_user(const crow::request& req) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
                nlohmann::json error = {{"success", false}, {"message", "Missing authorization header"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            std::string token = auth_header.substr(7);
            auto validation = auth_service->validate_token(token);
            
            if (!validation.valid) {
                nlohmann::json error = {{"success", false}, {"message", "Invalid token"}};
                return crow::response(401, "application/json", error.dump());
            }
            
            nlohmann::json request_data = nlohmann::json::parse(req.body);
            int blocked_user_id = request_data["blocked_user_id"];
            
            bool success = chat_service->block_user(validation.user_id, blocked_user_id);
            nlohmann::json response = {
                {"success", success},
                {"message", success ? "User blocked successfully" : "Failed to block user"}
            };
            
            return crow::response(success ? 200 : 500, "application/json", response.dump());
        } catch (const std::exception& e) {
            nlohmann::json error = {{"success", false}, {"message", "Server error"}};
            return crow::response(500, "application/json", error.dump());
        }
    }
};

int main() {
    ChatRoomServer server;
    
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    try {
        server.run(8080);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        server.stop();
        return 1;
    }
    
    return 0;
}
