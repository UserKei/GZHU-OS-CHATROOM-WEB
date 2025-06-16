#include "../include/services/message_filter.h"
#include <algorithm>
#include <fstream>
#include <iostream>

MessageFilter::MessageFilter() {
    initialize_filter();
}

void MessageFilter::initialize_filter() {
    auto default_words = get_default_sensitive_words();
    for (const auto& word : default_words) {
        sensitive_words.insert(word);
    }
}

void MessageFilter::load_sensitive_words_from_file(const std::string& filename) {
    std::ifstream file(filename);
    std::string word;
    while (std::getline(file, word)) {
        if (!word.empty()) {
            sensitive_words.insert(word);
        }
    }
}

void MessageFilter::add_sensitive_word(const std::string& word) {
    sensitive_words.insert(word);
}

std::string MessageFilter::filter_message(const std::string& message) {
    return replace_sensitive_words(message);
}

bool MessageFilter::contains_sensitive_content(const std::string& message) {
    std::string lower_message = message;
    std::transform(lower_message.begin(), lower_message.end(), lower_message.begin(), ::tolower);
    
    for (const auto& word : sensitive_words) {
        if (lower_message.find(word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

int MessageFilter::get_filtered_words_count(const std::string& message) {
    int count = 0;
    std::string lower_message = message;
    std::transform(lower_message.begin(), lower_message.end(), lower_message.begin(), ::tolower);
    
    for (const auto& word : sensitive_words) {
        size_t pos = 0;
        while ((pos = lower_message.find(word, pos)) != std::string::npos) {
            count++;
            pos += word.length();
        }
    }
    return count;
}

std::string MessageFilter::replace_sensitive_words(const std::string& message) {
    std::string result = message;
    
    for (const auto& word : sensitive_words) {
        size_t pos = 0;
        std::string replacement(word.length(), '*');
        
        // 不区分大小写的替换
        while ((pos = result.find(word, pos)) != std::string::npos) {
            result.replace(pos, word.length(), replacement);
            pos += replacement.length();
        }
    }
    
    return result;
}

std::vector<std::string> MessageFilter::get_default_sensitive_words() {
    return {
        "垃圾", "傻逼", "白痴", "混蛋", "废物",
        "shit", "fuck", "damn", "hell", "stupid",
        "政治", "暴力", "血腥", "色情"
        // 可以根据需要添加更多敏感词
    };
}
