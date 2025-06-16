#include "../include/services/auth_service.h"
#include "../include/database/database_manager.h"
#include <regex>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>

// 简单的token生成函数（替代JWT）
std::string generate_simple_token(const User& user) {
    std::stringstream ss;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    // 生成简单token: user_id:timestamp:random_string
    ss << user.id << ":" << std::time(nullptr) << ":";
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

// 简单的token解析函数
bool parse_simple_token(const std::string& token, int& user_id, std::time_t& timestamp) {
    size_t first_colon = token.find(':');
    size_t second_colon = token.find(':', first_colon + 1);
    
    if (first_colon == std::string::npos || second_colon == std::string::npos) {
        return false;
    }
    
    try {
        user_id = std::stoi(token.substr(0, first_colon));
        timestamp = std::stol(token.substr(first_colon + 1, second_colon - first_colon - 1));
        return true;
    } catch (...) {
        return false;
    }
}

AuthService::AuthService(std::shared_ptr<DatabaseManager> database) : db(database) {}

AuthService::RegisterResult AuthService::register_user(const std::string& username, 
                                                     const std::string& password, 
                                                     const std::string& email) {
    RegisterResult result;
    result.success = false;
    
    // 验证输入
    if (username.empty() || password.empty() || email.empty()) {
        result.message = "All fields are required";
        return result;
    }
    
    if (!validate_email(email)) {
        result.message = "Invalid email format";
        return result;
    }
    
    if (!validate_password(password)) {
        result.message = "Password must be at least 6 characters long";
        return result;
    }
    
    // 检查用户名是否可用
    if (!is_username_available(username)) {
        result.message = "Username already exists";
        return result;
    }
    
    // 检查邮箱是否可用
    if (!is_email_available(email)) {
        result.message = "Email already registered";
        return result;
    }
    
    // 创建用户
    User user;
    user.username = username;
    user.password_hash = User::hash_password(password);
    user.email = email;
    user.status = UserStatus::OFFLINE;
    
    if (db->create_user(user)) {
        result.success = true;
        result.message = "User registered successfully";
        result.user = db->get_user_by_username(username);
    } else {
        result.message = "Failed to create user";
    }
    
    return result;
}

AuthService::LoginResult AuthService::login_user(const std::string& username, 
                                                const std::string& password) {
    LoginResult result;
    result.success = false;
    
    // 获取用户
    auto user = db->get_user_by_username(username);
    if (!user) {
        result.message = "User not found";
        return result;
    }
    
    // 验证密码
    if (!user->verify_password(password)) {
        result.message = "Invalid password";
        return result;
    }
    
    // 更新用户状态为在线
    db->update_user_status(user->id, UserStatus::ONLINE);
    
    // 生成Token
    result.success = true;
    result.message = "Login successful";
    result.token = generate_token(*user);
    result.user = std::move(user);
    
    return result;
}

bool AuthService::logout_user(int user_id) {
    return db->update_user_status(user_id, UserStatus::OFFLINE);
}

AuthService::TokenValidationResult AuthService::validate_token(const std::string& token) {
    TokenValidationResult result;
    result.valid = false;
    
    int user_id;
    std::time_t timestamp;
    
    if (!parse_simple_token(token, user_id, timestamp)) {
        return result;
    }
    
    // 检查token是否过期（24小时）
    std::time_t now = std::time(nullptr);
    if (now - timestamp > 86400) { // 24小时 = 86400秒
        return result;
    }
    
    // 验证用户是否存在
    auto user = db->get_user_by_id(user_id);
    if (!user) {
        return result;
    }
    
    result.valid = true;
    result.user_id = user_id;
    result.username = user->username;
    
    return result;
}

std::string AuthService::generate_token(const User& user) {
    return generate_simple_token(user);
}

bool AuthService::update_user_status(int user_id, UserStatus status) {
    return db->update_user_status(user_id, status);
}

bool AuthService::is_username_available(const std::string& username) {
    auto user = db->get_user_by_username(username);
    return user == nullptr;
}

bool AuthService::is_email_available(const std::string&) {
    // 这里需要实现检查邮箱是否已注册的逻辑
    // 简化实现，假设邮箱唯一性由数据库约束保证
    return true;
}

bool AuthService::validate_email(const std::string& email) {
    const std::regex email_pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, email_pattern);
}

bool AuthService::validate_password(const std::string& password) {
    return password.length() >= 6;
}
