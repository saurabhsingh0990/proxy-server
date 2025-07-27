#include "server.hpp"
#include "logger.hpp"
#include "caching.hpp"
#include "content_filter.hpp"

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock

SimpleCache cache;
ContentFilter filter;

ProxyServer::ProxyServer(int port) : port_(port) {}

void ProxyServer::start() {
    WSADATA wsaData;
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    // Bind address and port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Proxy Server listening on port " << port_ << "...\n";

    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        char* clientIP = inet_ntoa(clientAddr.sin_addr);
        std::cout << "New client connected!\n";

        // Receive HTTP request
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string request(buffer);

            // Extract Host header
            std::string hostHeader = "Host: ";
            size_t pos = request.find(hostHeader);
            if (pos != std::string::npos) {
                size_t end = request.find("\r\n", pos);
                std::string hostLine = request.substr(pos + hostHeader.length(), end - pos - hostHeader.length());
                std::cout << "[+] Extracted Host: " << hostLine << "\n";

                std::string method = request.substr(0, request.find(' '));
                std::string urlLine = request.substr(request.find(' ') + 1);
                std::string url = urlLine.substr(0, urlLine.find(' '));
                std::string cacheKey = hostLine + url;

                logRequest(method, hostLine, request, clientIP);

                // Step 1: Check Cache
                if (cache.contains(cacheKey)) {
                    std::string cachedResponse = cache.get(cacheKey);
                    send(clientSocket, cachedResponse.c_str(), cachedResponse.length(), 0);
                    closesocket(clientSocket);
                    continue;
                }

                // Step 2: Resolve Host
                addrinfo hints{}, *res;
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_STREAM;

                int result = getaddrinfo(hostLine.c_str(), "80", &hints, &res);
                if (result != 0) {
                    std::cerr << "getaddrinfo failed: " << result << "\n";
                    closesocket(clientSocket);
                    continue;
                }

                SOCKET targetSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (targetSocket == INVALID_SOCKET) {
                    std::cerr << "Failed to create target socket.\n";
                    freeaddrinfo(res);
                    closesocket(clientSocket);
                    continue;
                }

                if (connect(targetSocket, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
                    std::cerr << "Connection to target failed.\n";
                    closesocket(targetSocket);
                    freeaddrinfo(res);
                    closesocket(clientSocket);
                    continue;
                }

                freeaddrinfo(res);

                // Step 3: Forward request to target
                send(targetSocket, request.c_str(), request.length(), 0);

                // Step 4: Receive and filter response
                char responseBuffer[8192];
                std::string responseData;

                int bytes;
                while ((bytes = recv(targetSocket, responseBuffer, sizeof(responseBuffer), 0)) > 0) {
                    responseData.append(responseBuffer, bytes);
                }

                closesocket(targetSocket);

                // Filter the content
                if (filter.isBlocked(responseData)) {
                    std::string blockMessage =
                        "HTTP/1.1 403 Forbidden\r\n"
                        "Content-Type: text/html\r\n\r\n"
                        "<html><body><h1>403 Forbidden</h1><p>Content blocked by proxy filter.</p></body></html>";
                    send(clientSocket, blockMessage.c_str(), blockMessage.size(), 0);
                    std::cout << "[FILTER] Blocked content for: " << cacheKey << "\n";
                } else {
                    send(clientSocket, responseData.c_str(), responseData.size(), 0);
                    cache.put(cacheKey, responseData);
                    std::cout << "[Cache STORE] Cached response for: " << cacheKey << "\n";
                    std::cout << "[✓] Response forwarded to client.\n";
                }

            } else {
                std::cerr << "[-] Host header not found.\n";
            }
        } else {
            std::cerr << "Failed to receive data or client disconnected.\n";
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
}
