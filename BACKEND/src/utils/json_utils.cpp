#include "../include/utils/json_utils.h"
#include <iostream>

std::string JsonUtils::get_string(const nlohmann::json& j, const std::string& key, const std::string& default_value) {
    if (j.contains(key) && j[key].is_string()) {
        return j[key].get<std::string>();
    }
    return default_value;
}

int JsonUtils::get_int(const nlohmann::json& j, const std::string& key, int default_value) {
    if (j.contains(key) && j[key].is_number()) {
        return j[key].get<int>();
    }
    return default_value;
}

bool JsonUtils::get_bool(const nlohmann::json& j, const std::string& key, bool default_value) {
    if (j.contains(key) && j[key].is_boolean()) {
        return j[key].get<bool>();
    }
    return default_value;
}

bool JsonUtils::has_key(const nlohmann::json& j, const std::string& key) {
    return j.contains(key);
}

nlohmann::json JsonUtils::create_error_response(const std::string& message) {
    return nlohmann::json{
        {"success", false},
        {"message", message}
    };
}

nlohmann::json JsonUtils::create_success_response(const std::string& message, const nlohmann::json& data) {
    nlohmann::json response = {
        {"success", true},
        {"message", message}
    };
    
    if (!data.is_null() && !data.empty()) {
        response["data"] = data;
    }
    
    return response;
}

bool JsonUtils::is_valid_json(const std::string& json_str) {
    try {
        auto parsed = nlohmann::json::parse(json_str);
        (void)parsed; // 避免未使用变量警告
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
