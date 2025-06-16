#include "../include/models/user.h"
#include <sstream>
#include <iomanip>
#include <functional>
#include <random>

std::string User::to_json() const {
    std::ostringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"username\":\"" << username << "\",";
    json << "\"email\":\"" << email << "\",";
    json << "\"status\":\"" << status_to_string(status) << "\",";
    json << "\"created_at\":" << created_at << ",";
    json << "\"last_seen\":" << last_seen;
    json << "}";
    return json.str();
}

bool User::verify_password(const std::string& password) const {
    return hash_password(password) == password_hash;
}

std::string User::hash_password(const std::string& password) {
    // 使用简单的哈希函数替代SHA-256（仅用于演示）
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + "salt_key_for_security");
    
    std::stringstream ss;
    ss << std::hex << hash_value;
    return ss.str();
}

std::string User::status_to_string(UserStatus status) {
    switch (status) {
        case UserStatus::ONLINE: return "ONLINE";
        case UserStatus::BUSY: return "BUSY";
        case UserStatus::OFFLINE: return "OFFLINE";
        default: return "OFFLINE";
    }
}

UserStatus User::string_to_status(const std::string& status) {
    if (status == "ONLINE") return UserStatus::ONLINE;
    if (status == "BUSY") return UserStatus::BUSY;
    return UserStatus::OFFLINE;
}
