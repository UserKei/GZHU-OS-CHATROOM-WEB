#pragma once
#include <string>
#include <ctime>

enum class MessageType {
    PUBLIC,
    PRIVATE,
    SYSTEM
};

class Message {
public:
    int id;
    int sender_id;
    int receiver_id; // -1 for public messages
    std::string content;
    MessageType type;
    std::time_t timestamp;
    bool is_deleted;
    std::string sender_username;
    
    Message() : id(0), sender_id(0), receiver_id(-1), type(MessageType::PUBLIC), 
                timestamp(0), is_deleted(false) {}
    
    Message(int id, int sender_id, const std::string& content, 
            MessageType type = MessageType::PUBLIC, int receiver_id = -1)
        : id(id), sender_id(sender_id), receiver_id(receiver_id), 
          content(content), type(type), timestamp(std::time(nullptr)), 
          is_deleted(false) {}
    
    // JSON序列化
    std::string to_json() const;
    static Message from_json(const std::string& json);
    
    // 消息验证
    bool is_valid() const;
    bool can_be_recalled() const; // 检查是否在2分钟内可撤回
    
    // 类型转换
    static std::string type_to_string(MessageType type);
    static MessageType string_to_type(const std::string& type);
};
