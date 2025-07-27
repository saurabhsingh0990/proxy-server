#pragma once
#include <string>
#include <vector>

class ContentFilter {
public:
    ContentFilter();
    void addBlockedWord(const std::string& word);
    void loadFromFile(const std::string& filepath);
    bool isBlocked(const std::string& text) const;

private:
    std::vector<std::string> blockedWords_;
};


