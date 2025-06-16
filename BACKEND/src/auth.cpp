#include "auth.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>

AuthManager::AuthManager() {
    // 定期清理过期会话
}

std::string AuthManager::generate_token() {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int token_length = 32;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string token;
    for (int i = 0; i < token_length; ++i) {
        token += charset[dis(gen)];
    }
    
    return token;
}

std::string AuthManager::hash_password(const std::string& password) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password.c_str(), password.length());
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool AuthManager::verify_password(const std::string& password, const std::string& hash) {
    return hash_password(password) == hash;
}

std::string AuthManager::create_session(int user_id, const std::string& username) {
    cleanup_expired_sessions();
    
    std::string token = generate_token();
    
    Session session;
    session.user_id = user_id;
    session.username = username;
    session.created_at = std::chrono::system_clock::now();
    session.last_access = session.created_at;
    
    sessions[token] = session;
    return token;
}

bool AuthManager::validate_session(const std::string& token) {
    auto it = sessions.find(token);
    if (it == sessions.end()) {
        return false;
    }
    
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - it->second.last_access);
    
    if (duration.count() > SESSION_TIMEOUT_MINUTES) {
        sessions.erase(it);
        return false;
    }
    
    // 更新最后访问时间
    it->second.last_access = now;
    return true;
}

int AuthManager::get_user_id_from_session(const std::string& token) {
    if (!validate_session(token)) {
        return -1;
    }
    
    auto it = sessions.find(token);
    return it != sessions.end() ? it->second.user_id : -1;
}

std::string AuthManager::get_username_from_session(const std::string& token) {
    if (!validate_session(token)) {
        return "";
    }
    
    auto it = sessions.find(token);
    return it != sessions.end() ? it->second.username : "";
}

bool AuthManager::destroy_session(const std::string& token) {
    auto it = sessions.find(token);
    if (it != sessions.end()) {
        sessions.erase(it);
        return true;
    }
    return false;
}

void AuthManager::cleanup_expired_sessions() {
    auto now = std::chrono::system_clock::now();
    
    for (auto it = sessions.begin(); it != sessions.end();) {
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - it->second.last_access);
        if (duration.count() > SESSION_TIMEOUT_MINUTES) {
            it = sessions.erase(it);
        } else {
            ++it;
        }
    }
}

std::string AuthManager::hash_user_password(const std::string& password) {
    return hash_password(password);
}

bool AuthManager::verify_user_password(const std::string& password, const std::string& hash) {
    return verify_password(password, hash);
}

std::vector<std::string> AuthManager::get_active_sessions() {
    cleanup_expired_sessions();
    
    std::vector<std::string> active_tokens;
    for (const auto& pair : sessions) {
        active_tokens.push_back(pair.first);
    }
    return active_tokens;
}
