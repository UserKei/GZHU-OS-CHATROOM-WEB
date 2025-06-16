#include "../include/services/chat_service.h"
#include "../include/database/database_manager.h"
#include "../include/services/message_filter.h"
#include <algorithm>

ChatService::ChatService(std::shared_ptr<DatabaseManager> database) 
    : db(database), filter(std::make_shared<MessageFilter>()) {}

ChatService::SendMessageResult ChatService::send_message(int sender_id, const std::string& content, 
                                                        MessageType type, int receiver_id) {
    SendMessageResult result;
    result.success = false;
    
    // 创建消息对象
    Message message(0, sender_id, content, type, receiver_id);
    
    // 验证消息
    if (!message.is_valid()) {
        result.message = "Invalid message";
        return result;
    }
    
    // 过滤敏感词
    std::string filtered_content = filter->filter_message(content);
    message.content = filtered_content;
    
    // 保存到数据库
    if (db->save_message(message)) {
        result.success = true;
        result.message = "Message sent successfully";
        
        // 从数据库重新获取消息（包含ID等信息）
        // 这里简化处理，实际应该返回完整的消息对象
        result.processed_message = std::make_unique<Message>(message);
    } else {
        result.message = "Failed to save message";
    }
    
    return result;
}

bool ChatService::recall_message(int message_id, int user_id) {
    // 这里需要检查消息是否属于该用户，以及是否在可撤回时间内
    return db->delete_message(message_id, user_id);
}

std::vector<Message> ChatService::get_chat_history(int user_id, int limit) {
    auto messages = db->get_recent_messages(limit);
    
    // 过滤被屏蔽用户的消息
    auto blocked_users = db->get_blocked_users(user_id);
    std::unordered_set<int> blocked_set(blocked_users.begin(), blocked_users.end());
    
    messages.erase(
        std::remove_if(messages.begin(), messages.end(),
            [&blocked_set](const Message& msg) {
                return blocked_set.find(msg.sender_id) != blocked_set.end();
            }),
        messages.end()
    );
    
    return messages;
}

std::vector<Message> ChatService::get_private_chat_history(int user1_id, int user2_id, int limit) {
    return db->get_private_messages(user1_id, user2_id, limit);
}

bool ChatService::add_online_user(int user_id) {
    std::lock_guard<std::mutex> lock(users_mutex);
    online_users.insert(user_id);
    db->update_user_status(user_id, UserStatus::ONLINE);
    return true;
}

bool ChatService::remove_online_user(int user_id) {
    std::lock_guard<std::mutex> lock(users_mutex);
    online_users.erase(user_id);
    db->update_user_status(user_id, UserStatus::OFFLINE);
    return true;
}

std::vector<User> ChatService::get_online_users_list() {
    std::lock_guard<std::mutex> lock(users_mutex);
    std::vector<User> users;
    
    for (int user_id : online_users) {
        auto user = db->get_user_by_id(user_id);
        if (user) {
            users.push_back(*user);
        }
    }
    
    return users;
}

bool ChatService::block_user(int user_id, int blocked_user_id) {
    return db->block_user(user_id, blocked_user_id);
}

bool ChatService::unblock_user(int user_id, int blocked_user_id) {
    return db->unblock_user(user_id, blocked_user_id);
}

std::vector<int> ChatService::get_blocked_users(int user_id) {
    return db->get_blocked_users(user_id);
}

bool ChatService::should_filter_message(int user_id, const Message& message) {
    return is_user_blocked(user_id, message.sender_id);
}

void ChatService::broadcast_system_message(const std::string& content) {
    Message system_msg(0, 0, content, MessageType::SYSTEM);
    db->save_message(system_msg);
}

void ChatService::send_user_join_notification(const std::string& username) {
    broadcast_system_message(username + " 加入了聊天室");
}

void ChatService::send_user_leave_notification(const std::string& username) {
    broadcast_system_message(username + " 离开了聊天室");
}

bool ChatService::is_user_blocked(int user_id, int potential_blocked_user_id) {
    auto blocked_users = db->get_blocked_users(user_id);
    return std::find(blocked_users.begin(), blocked_users.end(), potential_blocked_user_id) 
           != blocked_users.end();
}
