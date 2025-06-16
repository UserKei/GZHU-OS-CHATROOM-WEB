#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

struct User {
    int id;
    std::string username;
    std::string password_hash;
    std::string email;
    bool is_active;
    bool accept_messages;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_login;
};

struct Message {
    int id;
    int sender_id;
    std::string sender_username;
    std::string content;
    std::string original_content; // 保存原始内容（敏感词过滤前）
    bool is_deleted;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point deleted_at;
};

class Database {
private:
    sqlite3* db;
    std::string db_path;
    
    bool execute_sql(const std::string& sql);
    void create_tables();
    
public:
    Database(const std::string& database_path);
    ~Database();
    
    bool initialize();
    
    // 用户管理
    bool create_user(const std::string& username, const std::string& password_hash, const std::string& email);
    std::unique_ptr<User> get_user_by_username(const std::string& username);
    std::unique_ptr<User> get_user_by_id(int user_id);
    bool update_user_settings(int user_id, bool accept_messages);
    bool update_last_login(int user_id);
    std::vector<User> get_all_users();
    
    // 消息管理
    bool save_message(int sender_id, const std::string& content, const std::string& original_content = "");
    std::vector<Message> get_recent_messages(int limit = 100);
    std::vector<Message> get_messages_since(const std::chrono::system_clock::time_point& since);
    bool delete_message(int message_id, int user_id); // 只有发送者可以删除
    bool cleanup_old_messages(int days = 3); // 清理N天前的消息
    
    // 统计功能
    int get_online_user_count();
    int get_total_message_count();
};

#endif
