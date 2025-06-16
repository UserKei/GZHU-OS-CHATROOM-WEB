#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>

class AuthManager {
private:
    struct Session {
        int user_id;
        std::string username;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point last_access;
    };
    
    std::unordered_map<std::string, Session> sessions;
    static const int SESSION_TIMEOUT_MINUTES = 60;
    
    std::string generate_token();
    std::string hash_password(const std::string& password);
    bool verify_password(const std::string& password, const std::string& hash);
    
public:
    AuthManager();
    
    // 创建会话
    std::string create_session(int user_id, const std::string& username);
    
    // 验证会话
    bool validate_session(const std::string& token);
    
    // 获取会话用户ID
    int get_user_id_from_session(const std::string& token);
    
    // 获取会话用户名
    std::string get_username_from_session(const std::string& token);
    
    // 删除会话
    bool destroy_session(const std::string& token);
    
    // 清理过期会话
    void cleanup_expired_sessions();
    
    // 密码工具函数
    std::string hash_user_password(const std::string& password);
    bool verify_user_password(const std::string& password, const std::string& hash);
    
    // 获取所有活跃会话
    std::vector<std::string> get_active_sessions();
};

#endif
