#pragma once
#include <string>
#include <vector>
#include <unordered_set>

class MessageFilter {
private:
    std::unordered_set<std::string> sensitive_words;
    
public:
    MessageFilter();
    
    // 初始化敏感词库
    void initialize_filter();
    void load_sensitive_words_from_file(const std::string& filename);
    void add_sensitive_word(const std::string& word);
    
    // 消息过滤
    std::string filter_message(const std::string& message);
    bool contains_sensitive_content(const std::string& message);
    
    // 获取过滤统计
    int get_filtered_words_count(const std::string& message);
    
private:
    std::string replace_sensitive_words(const std::string& message);
    std::vector<std::string> get_default_sensitive_words();
};
