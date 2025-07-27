#include "content_filter.hpp"
#include <fstream>
#include <iostream>

// ContentFilter::ContentFilter() {
//     // Add default blocked words (you can customize this)
//     blockedWords_ = {"blocked", "forbidden", "malware", "ads"};
// }


ContentFilter::ContentFilter() {
    // Optional: default load
    loadFromFile("utils/blocked.txt");  // relative path from project root
}

void ContentFilter::addBlockedWord(const std::string& word) {
    blockedWords_.push_back(word);
}

void ContentFilter::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "[!] Failed to open blocklist: " << filepath << "\n";
        return;
    }

    std::string word;
    while (file >> word) {
        addBlockedWord(word);
    }

    std::cout << "[✓] Loaded blocklist from " << filepath << "\n";
}

bool ContentFilter::isBlocked(const std::string& text) const {
    for (const auto& word : blockedWords_) {
        if (text.find(word) != std::string::npos) {
            std::cout << "[FILTER] Blocked word found: " << word << "\n";
            return true;
        }
    }
    return false;
}
