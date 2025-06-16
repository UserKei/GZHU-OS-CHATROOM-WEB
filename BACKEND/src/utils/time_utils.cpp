#include "../include/utils/time_utils.h"
#include <sstream>
#include <iomanip>

std::time_t TimeUtils::get_current_timestamp() {
    return std::time(nullptr);
}

std::string TimeUtils::timestamp_to_string(std::time_t timestamp) {
    std::tm* tm_info = std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::time_t TimeUtils::string_to_timestamp(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::mktime(&tm);
}

std::string TimeUtils::get_current_iso_string() {
    std::time_t now = std::time(nullptr);
    std::tm* tm_info = std::gmtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_info, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

long TimeUtils::get_time_diff_seconds(std::time_t start, std::time_t end) {
    return static_cast<long>(std::difftime(end, start));
}

bool TimeUtils::is_within_time_range(std::time_t timestamp, int seconds) {
    std::time_t now = get_current_timestamp();
    long diff = get_time_diff_seconds(timestamp, now);
    return diff <= seconds;
}
