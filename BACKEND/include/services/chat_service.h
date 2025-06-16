#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include <mutex>
#include "../models/message.h"
#include "../models/user.h"

class DatabaseManager;
class MessageFilter;

class ChatService {
private:
    std::shared_ptr<DatabaseManager> db;
    std::shared_ptr<MessageFilter> filter;
    std::mutex users_mutex;
    std::unordered_set<int> online_users;
    
public:
    ChatService(std::shared_ptr<DatabaseManager> database);
    
    // 消息处理
    struct SendMessageResult {
        bool success;
        std::string message;
        std::unique_ptr<Message> processed_message;
    };
    SendMessageResult send_message(int sender_id, const std::string& content, 
                                  MessageType type = MessageType::PUBLIC, 
                                  int receiver_id = -1);
    
    // 消息撤回
    bool recall_message(int message_id, int user_id);
    
    // 获取消息历史
    std::vector<Message> get_chat_history(int user_id, int limit = 100);
    std::vector<Message> get_private_chat_history(int user1_id, int user2_id, int limit = 50);
    
    // 用户管理
    bool add_online_user(int user_id);
    bool remove_online_user(int user_id);
    std::vector<User> get_online_users_list();
    
    // 用户屏蔽
    bool block_user(int user_id, int blocked_user_id);
    bool unblock_user(int user_id, int blocked_user_id);
    std::vector<int> get_blocked_users(int user_id);
    
    // 消息过滤
    bool should_filter_message(int user_id, const Message& message);
    
    // 系统消息
    void broadcast_system_message(const std::string& content);
    void send_user_join_notification(const std::string& username);
    void send_user_leave_notification(const std::string& username);
    
private:
    bool is_user_blocked(int user_id, int potential_blocked_user_id);
};
