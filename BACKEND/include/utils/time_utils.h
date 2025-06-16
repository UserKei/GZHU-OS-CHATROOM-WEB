#pragma once
#include <string>
#include <ctime>

class TimeUtils {
public:
    // 获取当前时间戳
    static std::time_t get_current_timestamp();
    
    // 时间戳转字符串
    static std::string timestamp_to_string(std::time_t timestamp);
    
    // 字符串转时间戳
    static std::time_t string_to_timestamp(const std::string& time_str);
    
    // 获取当前时间的ISO字符串
    static std::string get_current_iso_string();
    
    // 检查时间差（秒）
    static long get_time_diff_seconds(std::time_t start, std::time_t end);
    
    // 检查是否在指定时间范围内
    static bool is_within_time_range(std::time_t timestamp, int seconds);
};
