#pragma once
#include <string>
#include <ctime>

enum class UserStatus {
    ONLINE,
    BUSY,
    OFFLINE
};

class User {
public:
    int id;
    std::string username;
    std::string password_hash;
    std::string email;
    UserStatus status;
    std::time_t created_at;
    std::time_t last_seen;
    
    User() : id(0), status(UserStatus::OFFLINE), created_at(0), last_seen(0) {}
    
    User(int id, const std::string& username, const std::string& password_hash, 
         const std::string& email, UserStatus status = UserStatus::OFFLINE)
        : id(id), username(username), password_hash(password_hash), 
          email(email), status(status), created_at(std::time(nullptr)), 
          last_seen(std::time(nullptr)) {}
    
    // JSON序列化
    std::string to_json() const;
    static User from_json(const std::string& json);
    
    // 密码验证
    bool verify_password(const std::string& password) const;
    static std::string hash_password(const std::string& password);
    
    // 状态转换
    static std::string status_to_string(UserStatus status);
    static UserStatus string_to_status(const std::string& status);
};
