#include "logger.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>

void logRequest(const std::string& method, const std::string& host, const std::string& fullRequest, const std::string& clientIP) {
    std::ofstream logFile("proxy.log", std::ios::app);  // append mode

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    char timeBuffer[100];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&nowTime));

    logFile << "=============================\n";
    logFile << "Timestamp: " << timeBuffer << "\n";
    logFile << "Method: " << method << "\n";
    logFile << "Host: " << host << "\n";
    logFile << "Client IP: "<< clientIP<<"\n";
    logFile << "Full Request:\n" << fullRequest << "\n\n";
}
