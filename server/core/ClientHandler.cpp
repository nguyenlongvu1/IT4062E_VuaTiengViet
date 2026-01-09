#include "ClientHandler.h"
#include "MessageParser.h"
#include "Dispatcher.h"
#include "Server.h"
#include "../services/RoomService.h"
#include "../database/UserDAO.h"
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>

ClientHandler::ClientHandler(int fd, Server* srv)
    : client_fd(fd), server(srv) {}

void ClientHandler::run() {
    while (running) {
        std::string msg = readMessage();
        if (msg.empty()) break;

        std::cout << "[CLIENT " << getUserId() << "] >> " << msg << std::endl;

        auto parsed = MessageParser::parse(msg);
        parsed.params["user_id"] = std::to_string(getUserId());

        Message response = Dispatcher::handleCommand(parsed, this);

        // 3. XỬ LÝ CÁC TRẠNG THÁI SERVER (Login/Logout)
        if (response.command == "LOGIN_OK" && response.params.count("user_id")) {
            int uid = std::stoi(response.params.at("user_id"));
            setUserId(uid);
            server->registerUser(uid, this);
            std::cout << "[SERVER] User " << uid << " logged in.\n";
        }
        else if (response.command == "LOGOUT_OK") {
            int uid = getUserId();
            server->unregisterUser(uid);
            setUserId(0);
        }

        // 4. XỬ LÝ BROADCAST (Gửi cho nhiều người)
        if (response.params.count("broadcast") && response.params.at("broadcast") == "true") {
            std::vector<int> targetIds;
            
            // Cách 1: Gửi theo Room ID (nếu có)
            if (response.params.count("room_id")) {
                int rId = std::stoi(response.params.at("room_id"));
                targetIds = RoomService::getPlayers(rId);
            }
            // Cách 2: Gửi theo danh sách ID cụ thể (dành cho Matchmaking)
            else if (response.params.count("players")) {
                std::string list = response.params.at("players");
                std::stringstream ss(list);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    if(!item.empty()) targetIds.push_back(std::stoi(item));
                }
            }
            // Đảm bảo người gửi cũng nhận được gói broadcast (tránh bị kẹt không thấy GAME_ENDED)
            int selfId = getUserId();
           bool selfInList = std::find(targetIds.begin(), targetIds.end(), selfId) != targetIds.end();
            if (selfId > 0 && !selfInList) targetIds.push_back(selfId);

            std::string packet = MessageParser::build(response);
            server->sendToUsers(targetIds, packet);
        }
        if (response.params.count("eliminated_id")) {
            int elimId = std::stoi(response.params.at("eliminated_id"));
            
            std::cout << "[SERVER] Sending ELIMINATED packet to User " << elimId << std::endl;
            
            // Tạo gói tin đơn giản báo bị loại
            std::string elimPacket = "COMMAND: ELIMINATED\nLENGTH: 0\n\n";
            
            // Gửi thẳng cho người đó
            server->sendToUser(elimId, elimPacket);
            
            // Xóa param này đi để không ảnh hưởng tới các logic khác
            response.params.erase("eliminated_id");
        }
        
        // 5. XỬ LÝ NOTIFICATION (Gửi thông báo riêng cho người khác - ví dụ: Kết bạn)
        else if (response.params.count("notify_id") && response.params.count("notify_msg")) {
            
            int targetId = std::stoi(response.params.at("notify_id"));
            std::string notifyPacket = response.params.at("notify_msg");
            server->sendToUser(targetId, notifyPacket);

            response.params.erase("notify_id");
            response.params.erase("notify_msg");

            std::string senderPacket = MessageParser::build(response);
            sendMessage(senderPacket);
        }

        else {
            if (response.command != "NO_RESPONSE") {
                std::string packet = MessageParser::build(response);
                sendMessage(packet);
            }
        }
    }

    performCleanup();
}

void ClientHandler::performCleanup() {
    int uid = getUserId();
    if (uid > 0) {
        std::cout << "[CLEANUP] User (ID: " << uid << ") disconnected.\n";

        int roomId = RoomService::leaveRoom(uid);
        if (roomId != -1) {
            std::vector<int> survivors = RoomService::getPlayers(roomId);
            for (int pid : survivors) {
                Message fakeReq; fakeReq.params["user_id"] = std::to_string(pid);
                Message infoMsg = RoomService::getRoomInfo(fakeReq);
                server->sendToUser(pid, MessageParser::build(infoMsg));
            }
        }

        UserDAO::removeSession(uid);
        server->unregisterUser(uid);
    }
    close(client_fd);
    server->removeClient(this);
}

std::string ClientHandler::readMessage() {
    char buffer[4096];
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return "";
    buffer[n] = '\0';
    return std::string(buffer);
}

void ClientHandler::sendMessage(const std::string& msg) {
    send(client_fd, msg.c_str(), msg.size(), 0);
}

void ClientHandler::stop() {
    running = false;
}