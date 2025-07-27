#include <iostream>
#include "caching.hpp"
#include <chrono>
using namespace std;

SimpleCache::SimpleCache() : ttlSeconds(60) {}  // default TTL to 60s

SimpleCache::SimpleCache(int ttl) : ttlSeconds(ttl) {}

bool SimpleCache::contains(const std::string& key) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        auto now = std::chrono::steady_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.second);

        if (age.count() < ttlSeconds) {
            cout << "[CACHE] HIT: " << key << "\n";
            return true;
        } else {
            cout << "[CACHE] EXPIRED: " << key << "\n";
            cache_.erase(it); // remove expired
        }
    }
    cout << "[CACHE] MISS: " << key << "\n";
    return false;
}


std::string SimpleCache::get(const std::string& key) {
    if (contains(key)) {
        return cache_[key].first;
    }
    return "";
}

void SimpleCache::put(const std::string& key, const std::string& value) {
    cache_[key] = {value, std::chrono::steady_clock::now()};
}
