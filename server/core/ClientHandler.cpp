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

        // 1. Parse tin nhắn
        auto parsed = MessageParser::parse(msg);
        parsed.params["user_id"] = std::to_string(getUserId());

        // 2. Gửi sang Dispatcher xử lý logic
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
        // Service trả về params["broadcast"] = "true" và params["players"] = "id1,id2,..."
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

            // Thực hiện gửi
            std::string packet = MessageParser::build(response);
            server->sendToUsers(targetIds, packet);
        }
        
        // 5. XỬ LÝ NOTIFICATION (Gửi thông báo riêng cho người khác - ví dụ: Kết bạn)
        // Service trả về params["notify_id"] và params["notify_msg"]
        else if (response.params.count("notify_id") && response.params.count("notify_msg")) {
            
            // BƯỚC 1: Lấy thông tin cần gửi cho người nhận (Target)
            int targetId = std::stoi(response.params.at("notify_id"));
            std::string notifyPacket = response.params.at("notify_msg");
            
            // BƯỚC 2: Gửi thông báo cho người kia
            server->sendToUser(targetId, notifyPacket);

            // BƯỚC 3: [FIX QUAN TRỌNG] Xóa dữ liệu notify khỏi response trước khi gửi lại cho người gửi (Sender)
            // Lý do: Nếu để nguyên, Client người gửi sẽ thấy chuỗi "COMMAND:..." trong payload 
            // và lầm tưởng đó là một lệnh mới gửi cho mình -> Gây ra lỗi hiện thông báo 2 bên.
            response.params.erase("notify_id");
            response.params.erase("notify_msg");

            // BƯỚC 4: Gửi phản hồi kết quả (ADD_FRIEND_RES) về cho người gửi
            std::string senderPacket = MessageParser::build(response);
            sendMessage(senderPacket);
        }

        // 6. PHẢN HỒI THÔNG THƯỜNG (Gửi lại cho chính người gọi)
        else {
            // Chỉ gửi nếu command không phải là NO_RESPONSE (một số logic không cần trả lời)
            if (response.command != "NO_RESPONSE") {
                std::string packet = MessageParser::build(response);
                sendMessage(packet);
            }
        }
    }

    // --- CLEANUP KHI NGẮT KẾT NỐI ---
    performCleanup();
}

void ClientHandler::performCleanup() {
    int uid = getUserId();
    if (uid > 0) {
        std::cout << "[CLEANUP] User (ID: " << uid << ") disconnected.\n";

        // Logic rời phòng khi rớt mạng
        int roomId = RoomService::leaveRoom(uid);
        if (roomId != -1) {
            std::vector<int> survivors = RoomService::getPlayers(roomId);
            for (int pid : survivors) {
                // Giả lập lệnh lấy Info để cập nhật lại UI cho người ở lại
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