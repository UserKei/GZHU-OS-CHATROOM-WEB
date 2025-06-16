#include "../include/models/message.h"
#include <sstream>
#include <chrono>

std::string Message::to_json() const {
    std::ostringstream json;
    json << "{";
    json << "\"id\":" << id << ",";
    json << "\"sender_id\":" << sender_id << ",";
    json << "\"receiver_id\":" << receiver_id << ",";
    json << "\"content\":\"" << content << "\",";
    json << "\"type\":\"" << type_to_string(type) << "\",";
    json << "\"timestamp\":" << timestamp << ",";
    json << "\"is_deleted\":" << (is_deleted ? "true" : "false") << ",";
    json << "\"sender_username\":\"" << sender_username << "\"";
    json << "}";
    return json.str();
}

bool Message::is_valid() const {
    return !content.empty() && sender_id > 0;
}

bool Message::can_be_recalled() const {
    auto now = std::time(nullptr);
    return (now - timestamp) <= 120; // 2分钟内可以撤回
}

std::string Message::type_to_string(MessageType type) {
    switch (type) {
        case MessageType::PUBLIC: return "PUBLIC";
        case MessageType::PRIVATE: return "PRIVATE";
        case MessageType::SYSTEM: return "SYSTEM";
        default: return "PUBLIC";
    }
}

MessageType Message::string_to_type(const std::string& type) {
    if (type == "PRIVATE") return MessageType::PRIVATE;
    if (type == "SYSTEM") return MessageType::SYSTEM;
    return MessageType::PUBLIC;
}
