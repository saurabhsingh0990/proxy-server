#include "caching.hpp"

std::string SimpleCache::get(const std::string& key) {
    return cache_.at(key);
}

void SimpleCache::put(const std::string& key, const std::string& response) {
    cache_[key] = response;
}

bool SimpleCache::contains(const std::string& key) const {
    return cache_.find(key) != cache_.end();
}
