#ifndef CACHING_HPP
#define CACHING_HPP

#include <string>
#include <unordered_map>
#include <chrono>

class SimpleCache {
public:
    SimpleCache();                   // Default constructor
    SimpleCache(int ttl);           // Constructor with TTL in seconds

    bool contains(const std::string& key) ;
    std::string get(const std::string& key);
    void put(const std::string& key, const std::string& value);

private:
    int ttlSeconds = 60; // Default TTL (60 seconds)
    std::unordered_map<std::string, std::pair<std::string, std::chrono::steady_clock::time_point>> cache_;
};

#endif // CACHING_HPP
