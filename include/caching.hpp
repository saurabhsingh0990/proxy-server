#ifndef CACHING_HPP
#define CACHING_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <list>

class SimpleCache {
public:
    SimpleCache();                         // Default constructor (TTL=60, MaxSize=100)
    SimpleCache(int ttl, size_t maxSize);  // Custom TTL and cache size

    bool contains(const std::string& key);
    std::string get(const std::string& key);
    void put(const std::string& key, const std::string& value);

private:
    int ttlSeconds = 60;
    size_t maxSize = 100;

    std::unordered_map<std::string, std::pair<std::string, std::chrono::steady_clock::time_point>> cache_;
    std::list<std::string> order_; // Insertion order to evict oldest
};

#endif // CACHING_HPP
