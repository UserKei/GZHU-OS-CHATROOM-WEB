#include "utils/content_filter.h"
#include <algorithm>
#include <cctype>

ContentFilter::ContentFilter() {
    load_default_sensitive_words();
    load_sensitive_patterns();
}

void ContentFilter::load_default_sensitive_words() {
    // 添加一些基本的敏感词（可以根据需要扩展）
    sensitive_words.insert("fuck");
    sensitive_words.insert("shit");
    sensitive_words.insert("damn");
    sensitive_words.insert("hell");
    sensitive_words.insert("垃圾");
    sensitive_words.insert("傻逼");
    sensitive_words.insert("混蛋");
    sensitive_words.insert("白痴");
    sensitive_words.insert("蠢货");
    sensitive_words.insert("死");
    sensitive_words.insert("杀");
    sensitive_words.insert("政治敏感词");
    sensitive_words.insert("暴力");
    sensitive_words.insert("恐怖");
}

void ContentFilter::load_sensitive_patterns() {
    // 添加一些敏感模式（正则表达式）
    try {
        sensitive_patterns.push_back(std::regex(R"(\d{11})", std::regex_constants::icase)); // 手机号
        sensitive_patterns.push_back(std::regex(R"(\d{15,19})", std::regex_constants::icase)); // 身份证号
        sensitive_patterns.push_back(std::regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})", std::regex_constants::icase)); // 邮箱
        sensitive_patterns.push_back(std::regex(R"(https?://[^\s]+)", std::regex_constants::icase)); // URL
    } catch (const std::regex_error& e) {
        // 处理正则表达式错误
    }
}

std::string ContentFilter::filter_content(const std::string& content) {
    std::string filtered = content;
    
    // 1. 过滤敏感词
    for (const auto& word : sensitive_words) {
        std::string replacement(word.length(), '*');
        
        // 大小写不敏感替换
        std::string content_lower = content;
        std::string word_lower = word;
        std::transform(content_lower.begin(), content_lower.end(), content_lower.begin(), ::tolower);
        std::transform(word_lower.begin(), word_lower.end(), word_lower.begin(), ::tolower);
        
        size_t pos = 0;
        while ((pos = content_lower.find(word_lower, pos)) != std::string::npos) {
            filtered.replace(pos, word.length(), replacement);
            content_lower.replace(pos, word.length(), replacement);
            pos += replacement.length();
        }
    }
    
    // 2. 过滤敏感模式
    for (const auto& pattern : sensitive_patterns) {
        try {
            filtered = std::regex_replace(filtered, pattern, "[已过滤]");
        } catch (const std::regex_error& e) {
            // 处理正则表达式错误
            continue;
        }
    }
    
    return filtered;
}

bool ContentFilter::contains_sensitive_content(const std::string& content) {
    std::string content_lower = content;
    std::transform(content_lower.begin(), content_lower.end(), content_lower.begin(), ::tolower);
    
    // 检查敏感词
    for (const auto& word : sensitive_words) {
        std::string word_lower = word;
        std::transform(word_lower.begin(), word_lower.end(), word_lower.begin(), ::tolower);
        
        if (content_lower.find(word_lower) != std::string::npos) {
            return true;
        }
    }
    
    // 检查敏感模式
    for (const auto& pattern : sensitive_patterns) {
        try {
            if (std::regex_search(content, pattern)) {
                return true;
            }
        } catch (const std::regex_error& e) {
            continue;
        }
    }
    
    return false;
}

void ContentFilter::add_sensitive_word(const std::string& word) {
    sensitive_words.insert(word);
}

void ContentFilter::remove_sensitive_word(const std::string& word) {
    sensitive_words.erase(word);
}

std::vector<std::string> ContentFilter::get_sensitive_words() const {
    std::vector<std::string> words;
    for (const auto& word : sensitive_words) {
        words.push_back(word);
    }
    return words;
}
