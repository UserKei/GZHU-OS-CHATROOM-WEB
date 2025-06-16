#pragma once
#include <string>
#include <nlohmann/json.hpp>

class JsonUtils {
public:
    // 安全地从JSON获取字符串值
    static std::string get_string(const nlohmann::json& j, const std::string& key, const std::string& default_value = "");
    
    // 安全地从JSON获取整数值
    static int get_int(const nlohmann::json& j, const std::string& key, int default_value = 0);
    
    // 安全地从JSON获取布尔值
    static bool get_bool(const nlohmann::json& j, const std::string& key, bool default_value = false);
    
    // 检查JSON中是否存在键
    static bool has_key(const nlohmann::json& j, const std::string& key);
    
    // 创建错误响应JSON
    static nlohmann::json create_error_response(const std::string& message);
    
    // 创建成功响应JSON
    static nlohmann::json create_success_response(const std::string& message, const nlohmann::json& data = nlohmann::json::object());
    
    // 验证JSON格式
    static bool is_valid_json(const std::string& json_str);
};
