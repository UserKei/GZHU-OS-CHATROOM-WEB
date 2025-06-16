#pragma once
#include <crow.h>
#include <unordered_map>
#include <memory>
#include <mutex>

class ChatService;
class AuthService;

class WebSocketHandler {
private:
    struct ClientConnection {
        crow::websocket::connection* conn;
        int user_id;
        std::string username;
        std::time_t connected_at;
    };
    
    std::unordered_map<crow::websocket::connection*, std::unique_ptr<ClientConnection>> clients;
    std::unordered_map<int, crow::websocket::connection*> user_connections;
    std::mutex clients_mutex;
    
    std::shared_ptr<ChatService> chat_service;
    std::shared_ptr<AuthService> auth_service;
    
public:
    WebSocketHandler(std::shared_ptr<ChatService> chat_service, 
                    std::shared_ptr<AuthService> auth_service);
    
    // WebSocket事件处理
    void on_open(crow::websocket::connection& conn);
    void on_close(crow::websocket::connection& conn, const std::string& reason);
    void on_message(crow::websocket::connection& conn, const std::string& data, bool is_binary);
    
    // 消息广播
    void broadcast_message(const std::string& message, int exclude_user_id = -1);
    void send_to_user(int user_id, const std::string& message);
    void send_to_connection(crow::websocket::connection* conn, const std::string& message);
    
    // 连接管理
    bool authenticate_connection(crow::websocket::connection& conn, const std::string& token);
    void disconnect_user(int user_id);
    std::vector<std::string> get_connected_users();
    
private:
    void handle_chat_message(crow::websocket::connection& conn, const std::string& message);
    void handle_private_message(crow::websocket::connection& conn, const std::string& message);
    void handle_status_change(crow::websocket::connection& conn, const std::string& status);
    void handle_recall_message(crow::websocket::connection& conn, int message_id);
    
    std::string create_message_json(const std::string& type, const std::string& content, 
                                   const std::string& sender = "", int message_id = 0);
    void cleanup_connection(crow::websocket::connection& conn);
};
