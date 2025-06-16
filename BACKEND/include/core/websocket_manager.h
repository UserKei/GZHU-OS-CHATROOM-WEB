#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <crow.h>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct WebSocketConnection {
    crow::websocket::connection* conn;
    int user_id;
    std::string username;
    std::chrono::system_clock::time_point connected_at;
    bool is_active;
};

class WebSocketManager {
private:
    std::unordered_map<crow::websocket::connection*, std::unique_ptr<WebSocketConnection>> connections;
    std::unordered_map<int, std::unordered_set<crow::websocket::connection*>> user_connections;
    std::mutex connections_mutex;
    
    void cleanup_dead_connections();
    
public:
    WebSocketManager();
    ~WebSocketManager();
    
    // 连接管理
    void add_connection(crow::websocket::connection* conn, int user_id, const std::string& username);
    void remove_connection(crow::websocket::connection* conn);
    bool is_user_online(int user_id);
    
    // 消息广播
    void broadcast_message(const std::string& message);
    void broadcast_message_except(const std::string& message, crow::websocket::connection* except_conn);
    void send_message_to_user(int user_id, const std::string& message);
    void send_message_to_connection(crow::websocket::connection* conn, const std::string& message);
    
    // 统计信息
    int get_online_count();
    std::vector<std::string> get_online_users();
    
    // 系统消息
    void broadcast_system_message(const std::string& message);
    void notify_user_joined(const std::string& username);
    void notify_user_left(const std::string& username);
};

#endif
