#include <iostream>
#include "caching.hpp"
#include <chrono>
using namespace std;

SimpleCache::SimpleCache() : ttlSeconds(60), maxSize(100) {}

SimpleCache::SimpleCache(int ttl, size_t maxSize)
    : ttlSeconds(ttl), maxSize(maxSize) {}

bool SimpleCache::contains(const std::string& key) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        auto now = chrono::steady_clock::now();
        auto age = chrono::duration_cast<chrono::seconds>(now - it->second.second);

        if (age.count() < ttlSeconds) {
            cout << "[CACHE] HIT: " << key << "\n";
            return true;
        } else {
            cout << "[CACHE] EXPIRED: " << key << "\n";
            cache_.erase(it);
            order_.remove(key);
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
    // If already exists, update value and timestamp, and move to back of list
    if (cache_.find(key) != cache_.end()) {
        order_.remove(key);
    }

    // Enforce max size before inserting
    if (cache_.size() >= maxSize) {
        std::string oldestKey = order_.front();
        order_.pop_front();
        cache_.erase(oldestKey);
        cout << "[CACHE] EVICTED: " << oldestKey << "\n";
    }

    cache_[key] = {value, chrono::steady_clock::now()};
    order_.push_back(key);
}
