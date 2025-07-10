#ifndef CACHE_HPP
#define CACHE_HPP

#include <string>
#include <unordered_map>

class SimpleCache {
public:
    std::string get(const std::string& key);
    void put(const std::string& key, const std::string& response);
    bool contains(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> cache_;
};

#endif
