#include "database.h"
#include <iostream>
#include <iomanip>
#include <sstream>

Database::Database(const std::string& database_path) : db(nullptr), db_path(database_path) {}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::initialize() {
    int result = sqlite3_open(db_path.c_str(), &db);
    if (result != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    create_tables();
    return true;
}

bool Database::execute_sql(const std::string& sql) {
    char* error_msg = nullptr;
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_msg);
    
    if (result != SQLITE_OK) {
        std::cerr << "SQL error: " << error_msg << std::endl;
        sqlite3_free(error_msg);
        return false;
    }
    return true;
}

void Database::create_tables() {
    // 创建用户表
    std::string create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            email TEXT,
            is_active BOOLEAN DEFAULT 1,
            accept_messages BOOLEAN DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_login DATETIME
        );
    )";
    
    // 创建消息表
    std::string create_messages_table = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id INTEGER NOT NULL,
            content TEXT NOT NULL,
            original_content TEXT,
            is_deleted BOOLEAN DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            deleted_at DATETIME,
            FOREIGN KEY (sender_id) REFERENCES users(id)
        );
    )";
    
    // 创建在线用户表
    std::string create_online_users_table = R"(
        CREATE TABLE IF NOT EXISTS online_users (
            user_id INTEGER PRIMARY KEY,
            last_heartbeat DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id)
        );
    )";
    
    execute_sql(create_users_table);
    execute_sql(create_messages_table);
    execute_sql(create_online_users_table);
}

bool Database::create_user(const std::string& username, const std::string& password_hash, const std::string& email) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO users (username, password_hash, email) VALUES (?, ?, ?)";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

std::unique_ptr<User> Database::get_user_by_username(const std::string& username) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, username, password_hash, email, is_active, accept_messages, created_at, last_login FROM users WHERE username = ?";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->is_active = sqlite3_column_int(stmt, 4) != 0;
        user->accept_messages = sqlite3_column_int(stmt, 5) != 0;
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return nullptr;
}

std::unique_ptr<User> Database::get_user_by_id(int user_id) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, username, password_hash, email, is_active, accept_messages, created_at, last_login FROM users WHERE id = ?";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return nullptr;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto user = std::make_unique<User>();
        user->id = sqlite3_column_int(stmt, 0);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->is_active = sqlite3_column_int(stmt, 4) != 0;
        user->accept_messages = sqlite3_column_int(stmt, 5) != 0;
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return nullptr;
}

bool Database::update_user_settings(int user_id, bool accept_messages) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE users SET accept_messages = ? WHERE id = ?";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, accept_messages ? 1 : 0);
    sqlite3_bind_int(stmt, 2, user_id);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

bool Database::update_last_login(int user_id) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = ?";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

std::vector<User> Database::get_all_users() {
    std::vector<User> users;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, username, password_hash, email, is_active, accept_messages FROM users WHERE is_active = 1";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return users;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.is_active = sqlite3_column_int(stmt, 4) != 0;
        user.accept_messages = sqlite3_column_int(stmt, 5) != 0;
        users.push_back(user);
    }
    
    sqlite3_finalize(stmt);
    return users;
}

bool Database::save_message(int sender_id, const std::string& content, const std::string& original_content) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO messages (sender_id, content, original_content) VALUES (?, ?, ?)";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, sender_id);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, original_content.empty() ? content.c_str() : original_content.c_str(), -1, SQLITE_STATIC);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

std::vector<Message> Database::get_recent_messages(int limit) {
    std::vector<Message> messages;
    sqlite3_stmt* stmt;
    const char* sql = R"(
        SELECT m.id, m.sender_id, u.username, m.content, m.original_content, 
               m.is_deleted, m.created_at, m.deleted_at
        FROM messages m
        JOIN users u ON m.sender_id = u.id
        WHERE m.is_deleted = 0
        ORDER BY m.created_at DESC
        LIMIT ?
    )";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return messages;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sender_id = sqlite3_column_int(stmt, 1);
        msg.sender_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        msg.original_content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        msg.is_deleted = sqlite3_column_int(stmt, 5) != 0;
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    
    // 反转顺序，让最新的消息在最后
    std::reverse(messages.begin(), messages.end());
    return messages;
}

bool Database::delete_message(int message_id, int user_id) {
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE messages SET is_deleted = 1, deleted_at = CURRENT_TIMESTAMP WHERE id = ? AND sender_id = ?";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, message_id);
    sqlite3_bind_int(stmt, 2, user_id);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE && sqlite3_changes(db) > 0;
}

bool Database::cleanup_old_messages(int days) {
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM messages WHERE created_at < datetime('now', '-' || ? || ' days')";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, days);
    
    result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

int Database::get_online_user_count() {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM online_users WHERE last_heartbeat > datetime('now', '-5 minutes')";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

int Database::get_total_message_count() {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM messages WHERE is_deleted = 0";
    
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}
