#include "websocket_manager.h"
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

WebSocketManager::WebSocketManager() {}

WebSocketManager::~WebSocketManager() {
    std::lock_guard<std::mutex> lock(connections_mutex);
    for (auto& pair : connections) {
        if (pair.second && pair.second->is_active) {
            pair.first->close("Server shutting down");
        }
    }
}

void WebSocketManager::add_connection(crow::websocket::connection* conn, int user_id, const std::string& username) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    auto ws_conn = std::make_unique<WebSocketConnection>();
    ws_conn->conn = conn;
    ws_conn->user_id = user_id;
    ws_conn->username = username;
    ws_conn->connected_at = std::chrono::system_clock::now();
    ws_conn->is_active = true;
    
    connections[conn] = std::move(ws_conn);
    user_connections[user_id].insert(conn);
    
    // 通知其他用户有新用户加入
    notify_user_joined(username);
}

void WebSocketManager::remove_connection(crow::websocket::connection* conn) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    auto it = connections.find(conn);
    if (it != connections.end()) {
        int user_id = it->second->user_id;
        std::string username = it->second->username;
        
        // 从用户连接映射中移除
        auto user_it = user_connections.find(user_id);
        if (user_it != user_connections.end()) {
            user_it->second.erase(conn);
            if (user_it->second.empty()) {
                user_connections.erase(user_it);
                // 通知其他用户该用户离开
                notify_user_left(username);
            }
        }
        
        connections.erase(it);
    }
}

bool WebSocketManager::is_user_online(int user_id) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    auto it = user_connections.find(user_id);
    return it != user_connections.end() && !it->second.empty();
}

void WebSocketManager::cleanup_dead_connections() {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    std::vector<crow::websocket::connection*> to_remove;
    
    for (auto& pair : connections) {
        if (!pair.second->is_active) {
            to_remove.push_back(pair.first);
        }
    }
    
    for (auto conn : to_remove) {
        remove_connection(conn);
    }
}

void WebSocketManager::broadcast_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    for (auto& pair : connections) {
        if (pair.second && pair.second->is_active) {
            try {
                pair.first->send_text(message);
            } catch (...) {
                pair.second->is_active = false;
            }
        }
    }
    
    cleanup_dead_connections();
}

void WebSocketManager::broadcast_message_except(const std::string& message, crow::websocket::connection* except_conn) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    for (auto& pair : connections) {
        if (pair.second && pair.second->is_active && pair.first != except_conn) {
            try {
                pair.first->send_text(message);
            } catch (...) {
                pair.second->is_active = false;
            }
        }
    }
    
    cleanup_dead_connections();
}

void WebSocketManager::send_message_to_user(int user_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    auto it = user_connections.find(user_id);
    if (it != user_connections.end()) {
        for (auto conn : it->second) {
            auto conn_it = connections.find(conn);
            if (conn_it != connections.end() && conn_it->second->is_active) {
                try {
                    conn->send_text(message);
                } catch (...) {
                    conn_it->second->is_active = false;
                }
            }
        }
    }
    
    cleanup_dead_connections();
}

void WebSocketManager::send_message_to_connection(crow::websocket::connection* conn, const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    auto it = connections.find(conn);
    if (it != connections.end() && it->second->is_active) {
        try {
            conn->send_text(message);
        } catch (...) {
            it->second->is_active = false;
            cleanup_dead_connections();
        }
    }
}

int WebSocketManager::get_online_count() {
    std::lock_guard<std::mutex> lock(connections_mutex);
    return user_connections.size();
}

std::vector<std::string> WebSocketManager::get_online_users() {
    std::lock_guard<std::mutex> lock(connections_mutex);
    
    std::vector<std::string> online_users;
    for (const auto& pair : user_connections) {
        if (!pair.second.empty()) {
            // 获取第一个连接的用户名
            auto conn = *pair.second.begin();
            auto conn_it = connections.find(conn);
            if (conn_it != connections.end()) {
                online_users.push_back(conn_it->second->username);
            }
        }
    }
    
    return online_users;
}

void WebSocketManager::broadcast_system_message(const std::string& message) {
    json system_msg;
    system_msg["type"] = "system";
    system_msg["message"] = message;
    system_msg["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    broadcast_message(system_msg.dump());
}

void WebSocketManager::notify_user_joined(const std::string& username) {
    json notification;
    notification["type"] = "user_joined";
    notification["username"] = username;
    notification["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    notification["online_count"] = get_online_count();
    
    broadcast_message(notification.dump());
}

void WebSocketManager::notify_user_left(const std::string& username) {
    json notification;
    notification["type"] = "user_left";
    notification["username"] = username;
    notification["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    notification["online_count"] = get_online_count();
    
    broadcast_message(notification.dump());
}
