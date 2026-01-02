#include "Server.h"
#include "ClientHandler.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <algorithm>

static Server* serverInstance = nullptr;

Server::Server(int port) : port(port) {
    serverInstance = this;
}

Server::~Server() {
    // Cleanup all handlers
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto handler : clients) {
            delete handler;
        }
        clients.clear();
    }
    close(server_fd);
}

void Server::start() {
    serverInstance = this;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return;
    }

    if (listen(server_fd, 20) < 0) {
        perror("listen");
        return;
    }

    std::cout << "[SERVER] Listening on port " << port << "...\n";
    acceptLoop();
}

void Server::acceptLoop() {
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "[SERVER] Client connected.\n";
        auto* handler = new ClientHandler(client_fd, this);

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(handler);
        }

        std::thread(&ClientHandler::run, handler).detach();
    }
}

void Server::removeClient(ClientHandler* handler) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    auto it = std::find(clients.begin(), clients.end(), handler);
    if (it != clients.end()) {
        clients.erase(it);
        delete handler;  // Safe: only Server deletes handlers
    }
    // NOTE: ClientHandler::run() already cleaned up session and user_map on disconnect
}

Server* Server::getInstance() {
    return serverInstance;
}

void Server::registerUser(int userId, ClientHandler* handler) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    user_map[userId] = handler;
}

void Server::unregisterUser(int userId) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    user_map.erase(userId);
}

void Server::sendToUser(int userId, const std::string& msg) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (user_map.count(userId)) {
        user_map[userId]->sendMessage(msg);
    }
}

void Server::sendToUsers(const std::vector<int>& userIds, const std::string& msg) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int uid : userIds) {
        if (user_map.count(uid)) {
            user_map[uid]->sendMessage(msg);
        }
    }
}

bool Server::isUserOnline(int userId) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    return user_map.count(userId) > 0;
}

