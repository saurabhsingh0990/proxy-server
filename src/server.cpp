#include "server.hpp"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "logger.hpp"


#pragma comment(lib, "ws2_32.lib") // Link Winsock


ProxyServer :: ProxyServer(int port): port_(port){};
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

    // Listen
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Proxy Server listening on port " << port_ << "...\n";

    // Main loop
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
            

            buffer[bytesReceived] = '\0'; // Null-terminate
            std::string request(buffer);
            
            // Extract Host header
            std::string hostHeader = "Host: ";
            size_t pos = request.find(hostHeader);
            if (pos != std::string::npos) {
                size_t end = request.find("\r\n", pos);
                std::string hostLine = request.substr(pos + hostHeader.length(), end - pos - hostHeader.length());
                std::cout << "[+] Extracted Host: " << hostLine << "\n";

                std::string method = request.substr(0, request.find(' '));  // "GET", "POST", etc.

                // log the request
                logRequest(method, hostLine, request, clientIP);

                // ==========================================
                // 🔁 Step 1: Resolve host and connect to it
                // ==========================================
                addrinfo hints{}, *res;
                hints.ai_family = AF_INET;
                hints.ai_socktype = SOCK_STREAM;

                int result = getaddrinfo(hostLine.c_str(), "80", &hints, &res);
                if (result != 0) {
                    std::cerr << "getaddrinfo failed: " << result << "\n";
                    closesocket(clientSocket);
                    return;
                }

                SOCKET targetSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (targetSocket == INVALID_SOCKET) {
                    std::cerr << "Failed to create target socket.\n";
                    freeaddrinfo(res);
                    closesocket(clientSocket);
                    return;
                }

                if (connect(targetSocket, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
                    std::cerr << "Connection to target failed.\n";
                    closesocket(targetSocket);
                    freeaddrinfo(res);
                    closesocket(clientSocket);
                    return;
                }

                freeaddrinfo(res);

                // ==========================================
                // 🔁 Step 2: Forward client request to target
                // ==========================================
                send(targetSocket, request.c_str(), request.length(), 0);

                // ==========================================
                // 🔁 Step 3: Receive response from target
                // ==========================================
                char responseBuffer[8192];
                int bytes;
                while ((bytes = recv(targetSocket, responseBuffer, sizeof(responseBuffer), 0)) > 0) {
                    send(clientSocket, responseBuffer, bytes, 0); // relay back to client
                }

                std::cout << "[✓] Response forwarded to client.\n";

                closesocket(targetSocket); // Close target server connection
            } else {
                std::cerr << "[-] Host header not found.\n";
            }
        }
        else {
            std::cerr << "Failed to receive data or client disconnected.\n";
        }

        // Close client socket
        closesocket(clientSocket);
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();
}
