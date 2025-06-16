#pragma once
#include <string>
#include <memory>
#include <vector>
#include <sqlite3.h>
#include "../models/user.h"
#include "../models/message.h"

class DatabaseManager {
private:
    sqlite3* db;
    std::string db_path;
    
public:
    DatabaseManager(const std::string& db_path);
    ~DatabaseManager();
    
    bool initialize();
    bool create_tables();
    
    // 用户相关操作
    bool create_user(const User& user);
    std::unique_ptr<User> get_user_by_username(const std::string& username);
    std::unique_ptr<User> get_user_by_id(int user_id);
    bool update_user_status(int user_id, UserStatus status);
    bool update_user_last_seen(int user_id);
    std::vector<User> get_online_users();
    
    // 消息相关操作
    bool save_message(const Message& message);
    std::vector<Message> get_recent_messages(int limit = 100);
    std::vector<Message> get_messages_after_timestamp(const std::string& timestamp);
    std::vector<Message> get_private_messages(int user1_id, int user2_id, int limit = 50);
    bool delete_message(int message_id, int user_id);
    bool mark_message_as_read(int message_id, int user_id);
    
    // 用户关系操作
    bool block_user(int user_id, int blocked_user_id);
    bool unblock_user(int user_id, int blocked_user_id);
    std::vector<int> get_blocked_users(int user_id);
    
    // 清理过期数据
    bool cleanup_old_messages();
    
private:
    bool execute_query(const std::string& query);
    bool check_table_exists(const std::string& table_name);
};
