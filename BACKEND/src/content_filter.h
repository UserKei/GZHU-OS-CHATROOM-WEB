#ifndef CONTENT_FILTER_H
#define CONTENT_FILTER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <regex>

class ContentFilter {
private:
    std::unordered_set<std::string> sensitive_words;
    std::vector<std::regex> sensitive_patterns;
    
    void load_default_sensitive_words();
    void load_sensitive_patterns();
    
public:
    ContentFilter();
    
    // 过滤敏感词
    std::string filter_content(const std::string& content);
    
    // 检查是否包含敏感词
    bool contains_sensitive_content(const std::string& content);
    
    // 添加敏感词
    void add_sensitive_word(const std::string& word);
    
    // 移除敏感词
    void remove_sensitive_word(const std::string& word);
    
    // 获取所有敏感词
    std::vector<std::string> get_sensitive_words() const;
};

#endif
