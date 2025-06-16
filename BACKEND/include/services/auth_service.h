#pragma once
#include <string>
#include <memory>
#include "../models/user.h"

class DatabaseManager;

class AuthService {
private:
    std::shared_ptr<DatabaseManager> db;
    
public:
    AuthService(std::shared_ptr<DatabaseManager> database);
    
    // 用户注册
    struct RegisterResult {
        bool success;
        std::string message;
        std::unique_ptr<User> user;
    };
    RegisterResult register_user(const std::string& username, 
                                const std::string& password, 
                                const std::string& email);
    
    // 用户登录
    struct LoginResult {
        bool success;
        std::string message;
        std::unique_ptr<User> user;
        std::string token;
    };
    LoginResult login_user(const std::string& username, const std::string& password);
    
    // 用户注销
    bool logout_user(int user_id);
    
    // Token验证
    struct TokenValidationResult {
        bool valid;
        int user_id;
        std::string username;
    };
    TokenValidationResult validate_token(const std::string& token);
    
    // 生成JWT Token
    std::string generate_token(const User& user);
    
    // 更新用户状态
    bool update_user_status(int user_id, UserStatus status);
    
private:
    bool is_username_available(const std::string& username);
    bool is_email_available(const std::string& email);
    bool validate_email(const std::string& email);
    bool validate_password(const std::string& password);
};
