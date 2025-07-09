#include "server.hpp"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

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
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::cout << "New client connected!\n";

        // Receive HTTP request
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate
            std::cout << "Received request:\n" << buffer << std::endl;
        } else {
            std::cerr << "Failed to receive data or client disconnected.\n";
        }

        // Close client socket
        closesocket(clientSocket);
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();
}
