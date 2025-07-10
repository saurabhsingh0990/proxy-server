#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

void logRequest(const std::string& method, const std::string& host, const std::string& fullRequest, const std::string& clientIP);

#endif
