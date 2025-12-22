#include "ClientHandler.h"
#include "MessageParser.h"
#include "Dispatcher.h"
#include "Server.h"
#include "../services/RoomService.h"
#include "../database/UserDAO.h"
#include "../database/FriendDAO.h"
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <vector>

ClientHandler::ClientHandler(int fd, Server* srv)
    : client_fd(fd), server(srv) {}

void ClientHandler::run() {
    while (running) {
        std::string msg = readMessage();
        if (msg.empty()) break;

        std::cout << "[CLIENT] >> " << msg << std::endl;

        auto parsed = MessageParser::parse(msg);

        // =========================================================
        // 1. XỬ LÝ TÌM KIẾM (SEARCH_REQ)
        // =========================================================
        if (parsed.command == "SEARCH_REQ") {
            std::string keyword = "";
            if (parsed.params.count("keyword")) {
                keyword = parsed.params.at("keyword");
            }

            // Gọi DAO tìm kiếm
            std::vector<UserDAO::UserSearchInfo> results = UserDAO::searchUsers(keyword);

            // Đóng gói kết quả: User1,Online;User2,Offline;
            std::string payloadStr = "";
            for (const auto& u : results) {
                payloadStr += u.username + "," + u.status + ";";
            }

            // Gửi trả về Client
            std::string responseStr = "COMMAND: SEARCH_RES\nLENGTH: " + std::to_string(payloadStr.length()) + "\n\n" + payloadStr;
            sendMessage(responseStr);
            continue; 
        }

        // =========================================================
        // 2. XỬ LÝ GỬI LỜI MỜI KẾT BẠN (ADD_FRIEND_REQ) - LOGIC THẬT
        // =========================================================
        if (parsed.command == "ADD_FRIEND_REQ") {
            std::string targetUser = "";
            if (parsed.params.count("target_username")) targetUser = parsed.params.at("target_username");
            
            std::cout << "[SERVER] Dang xu ly ket ban tu ID " << getUserId() << " den: " << targetUser << std::endl;

            // 1. Tìm ID người nhận
            auto targetUserObj = UserDAO::findByUsername(targetUser);
            if (!targetUserObj) {
                sendMessage("COMMAND: ADD_FRIEND_RES\n\nstatus=fail;msg=User not found");
                continue;
            }
            
            int targetId = targetUserObj->id;
            int myId = getUserId();
            
            if (targetId == myId) {
                 sendMessage("COMMAND: ADD_FRIEND_RES\n\nstatus=fail;msg=Cannot add yourself");
                 continue;
            }

            // 2. Gửi yêu cầu vào DB
            bool success = FriendDAO::sendRequest(myId, targetId);
            
            if (success) {
                // 3. Báo thành công cho người gửi
                sendMessage("COMMAND: ADD_FRIEND_RES\n\nstatus=success;msg=Request sent");
                
                // 4. Báo ngay cho người nhận (nếu đang Online)
                if (server->isUserOnline(targetId)) {
                    auto myUserObj = UserDAO::findById(myId);
                    std::string myName = myUserObj ? myUserObj->username : "Unknown";
                    
                    std::cout << "[SERVER] Nguoi nhan Online -> Gui thong bao ngay!" << std::endl;

                    std::string notifyMsg = "COMMAND: NOTIFY_FRIEND_REQ\n\nsender_username=" + myName;
                    server->sendToUser(targetId, notifyMsg);
                }
            } else {
                sendMessage("COMMAND: ADD_FRIEND_RES\n\nstatus=fail;msg=Request already exists or are friends");
            }
            continue;
        }

        // =========================================================
        // 3. LẤY DANH SÁCH LỜI MỜI (GET_PENDING_REQ)
        // =========================================================
        if (parsed.command == "GET_PENDING_REQ") {
            int myId = getUserId();
            std::vector<int> pendingIds = FriendDAO::getPendingRequests(myId);
            
            std::string listStr = "";
            for (int uid : pendingIds) {
                auto u = UserDAO::findById(uid);
                if (u) {
                    listStr += u->username + ",";
                }
            }
            
            sendMessage("COMMAND: GET_PENDING_RES\n\nrequest_list=" + listStr);
            continue;
        }

        // =========================================================
        // 4. CHẤP NHẬN KẾT BẠN (ACCEPT_FRIEND_REQ)
        // =========================================================
        if (parsed.command == "ACCEPT_FRIEND_REQ") {
            std::string targetUserStr = "";
            if (parsed.params.count("target_username")) targetUserStr = parsed.params.at("target_username");

            auto targetUserObj = UserDAO::findByUsername(targetUserStr);
            if (!targetUserObj) continue;

            int targetId = targetUserObj->id; // Người gửi lời mời
            int myId = getUserId();           // Mình (người chấp nhận)

            bool ok = FriendDAO::acceptRequest(targetId, myId); 
            
            if (ok) {
                // 1. Báo cho mình
                sendMessage("COMMAND: ACCEPT_FRIEND_RES\n\nstatus=success;target=" + targetUserStr);
                
                // 2. Báo cho người kia (nếu Online)
                if (server->isUserOnline(targetId)) {
                    auto myUserObj = UserDAO::findById(myId);
                    std::string myName = myUserObj ? myUserObj->username : "Unknown";
                    
                    std::string notifyMsg = "COMMAND: NOTIFY_FRIEND_ACCEPTED\n\nfriend_username=" + myName;
                    server->sendToUser(targetId, notifyMsg);
                }
            }
            continue;
        }
                // Trong ClientHandler.cpp
        if (parsed.command == "GET_FRIEND_LIST") {
            int myId = getUserId();
            // Gọi FriendDAO lấy danh sách bạn bè (status='accepted')
            std::vector<UserDAO::UserSearchInfo> friends = FriendDAO::getFriends(myId);
            
            std::string payload = "";
            for (const auto& f : friends) {
                payload += f.username + "," + f.status + ";";
            }
            
            sendMessage("COMMAND: FRIEND_LIST_RES\nLENGTH: " + std::to_string(payload.length()) + "\n\n" + payload);
            continue;
        }
        // Thêm vào ClientHandler::run(), trước phần "LOGIC CŨ"
        if (parsed.command == "JOIN_ROOM_REQ") {
            int roomId = std::stoi(parsed.params.at("room_id"));
            int myId = getUserId();

            Message joinMsg;
            joinMsg.command = "JOIN_ROOM";
            joinMsg.params["room_id"] = std::to_string(roomId);
            joinMsg.params["user_id"] = std::to_string(myId);

            Message resp = RoomService::joinRoom(joinMsg);
            
            // Gửi phản hồi về cho người vừa xin vào
                std::string finalStatus = resp.params.count("status") ? resp.params["status"] : "fail";
            std::string failReason = resp.params.count("reason") ? resp.params["reason"] : "";

            std::string responseMsg = "COMMAND: JOIN_ROOM_RES\n\nstatus=" + finalStatus + 
                                    ";room_id=" + std::to_string(roomId) + 
                                    ";reason=" + failReason;

            sendMessage(responseMsg);

            // Nếu thành công, Notify cho chủ phòng và những người khác
            if (finalStatus == "success") {
                auto players = RoomService::getPlayers(roomId);
                std::string notify = "COMMAND: PLAYER_JOINED_NOTIFY\n\nroom_id=" + std::to_string(roomId) + ";count=" + resp.params["count"];
                server->sendToUsers(players, notify);
            }
            continue;
        }
        // =========================================================
        // [MỚI] XỬ LÝ TÌM TRẬN (RANK)
        // =========================================================
        if (parsed.command == "FIND_MATCH_REQ") {
            int myUid = getUserId();
            RoomService::addToQueue(myUid);
            
            auto result = RoomService::processMatchmaking();
            if (result.success) {
                std::string msgFound = "COMMAND: MATCH_FOUND_NOTIFY\n\nroom_id=" + std::to_string(result.roomId);
                
                // Gửi tin nhắn cho 3 người
                server->sendToUser(result.p1, msgFound);
                server->sendToUser(result.p2, msgFound);
                server->sendToUser(result.p3, msgFound);
                
                // Cực kỳ quan trọng: Dùng return hoặc continue để kết thúc xử lý tại đây
                return; 
            } else {
                sendMessage("COMMAND: FIND_MATCH_RES\n\nstatus=waiting");
            }
            return; // Đảm bảo không chạy xuống Dispatcher::handleCommand bên dưới
        }
        // Xử lý Hủy tìm trận
        if (parsed.command == "CANCEL_MATCH_REQ") {
            int myUid = getUserId();
            RoomService::removeFromQueue(myUid);
            sendMessage("COMMAND: CANCEL_MATCH_RES\n\nstatus=cancelled");
            continue;
        }

        // =========================================================
        // LOGIC CŨ (LOGIN, ROOM, GAME...) - GIỮ NGUYÊN
        // =========================================================
        Message response = Dispatcher::handleCommand(parsed, this);
        std::string responseStr = MessageParser::build(response);

        if (response.command == "LOGIN_OK" && response.params.count("user_id")) {
            int uid = std::stoi(response.params.at("user_id"));
            setUserId(uid);
            server->registerUser(uid, this);
            std::cout << "[SERVER] User " << uid << " is now ONLINE.\n";
        }

        if (response.command == "LOGOUT_OK" && response.params.count("cleanup_user_map")) {
            if (response.params.count("user_id")) {
                int uid = std::stoi(response.params.at("user_id"));
                server->unregisterUser(uid);
                setUserId(0);
            }
        }
        if (parsed.command == "LEAVE_ROOM_REQ") {
            int uid = getUserId();
            
            // 1. Xử lý rời phòng
            int roomId = RoomService::leaveRoom(uid);
            
            // 2. Báo về cho chính người rời (để họ đóng cửa sổ)
            sendMessage("COMMAND: LEAVE_ROOM_RES\n\nstatus=success");

            // 3. Báo tin cập nhật cho những người còn lại (Logic y hệt CLEANUP)
            if (roomId != -1) {
            // Lấy danh sách những người còn lại
            std::vector<int> survivors = RoomService::getPlayers(roomId);
            
            // Duyệt qua từng người để báo tin
            for (int pid : survivors) {
                Message fakeReq;
                fakeReq.params["user_id"] = std::to_string(pid);
                
                // Lấy thông tin phòng MỚI NHẤT (Lúc này đã thiếu 1 người)
                Message infoMsg = RoomService::getRoomInfo(fakeReq);
                
                // Gửi về Client
                server->sendToUser(pid, MessageParser::build(infoMsg));
            }
            continue;}
        }

        if (response.params.count("broadcast") && response.params.at("broadcast") == "true") {
            std::vector<int> players;
            if (response.params.count("room_id")) {
                int roomId = std::stoi(response.params.at("room_id"));
                players = RoomService::getPlayers(roomId);
            } else if (response.params.count("players")) {
                // Xử lý parse list players thủ công
                std::string list = response.params.at("players");
                size_t start = 0;
                while (start < list.size()) {
                    size_t pos = list.find(',', start);
                    std::string token = (pos==std::string::npos) ? list.substr(start) : list.substr(start, pos-start);
                    if (!token.empty()) players.push_back(std::stoi(token));
                    if (pos==std::string::npos) break;
                    start = pos + 1;
                }
            }
            
            if (response.command == "REMATCH_ASK") {
                std::vector<int> targets;
                for (int p : players) if (p != getUserId()) targets.push_back(p);
                server->sendToUsers(targets, responseStr);
            } else {
                server->sendToUsers(players, responseStr);
            }

            if (response.params.count("auto_start_match") && response.params.at("auto_start_match") == "true") {
                Message startMsg;
                startMsg.command = "START_MATCH";
                startMsg.params["room_id"] = response.params.at("room_id");
                Message startResp = Dispatcher::handleCommand(startMsg, this);
                std::string startRespStr = MessageParser::build(startResp);
                if (startRespStr.size() > 0) {
                     if (startResp.params.count("room_id")) {
                         auto playersList = RoomService::getPlayers(std::stoi(startResp.params.at("room_id")));
                         server->sendToUsers(playersList, startRespStr);
                     }
                }
            }
        } else {
            if (!responseStr.empty()) {
                sendMessage(responseStr);
            }
        }
        
    }

    // --- CLEANUP ---
   // Trong ClientHandler.cpp -> Hàm run() -> Phần CLEANUP (Cuối hàm)

    // --- CLEANUP ---
    int uid = getUserId();
    if (uid > 0) {
        std::cout << "[CLEANUP] User (ID: " << uid << ") disconnected.\n";

        // 1. Rời phòng và LẤY ID PHÒNG (Để biết cần báo tin cho ai)
        // Lưu ý: Hàm leaveRoom phải trả về int (roomId) như đã sửa ở bước trước
        int roomId = RoomService::leaveRoom(uid); 
        
        // 2. NẾU VỪA RỜI KHỎI MỘT PHÒNG -> BÁO TIN CHO NGƯỜI Ở LẠI
        if (roomId != -1) {
            std::vector<int> survivors = RoomService::getPlayers(roomId);
            
            // Duyệt qua những người còn sống sót trong phòng
            for (int pid : survivors) {
                // Tạo một yêu cầu giả để lấy thông tin phòng mới nhất
                Message fakeReq;
                fakeReq.params["user_id"] = std::to_string(pid);
                
                // Gọi hàm lấy info (Lúc này Server đã tự đẩy người 2 lên làm Host rồi)
                Message infoMsg = RoomService::getRoomInfo(fakeReq);
                
                // Đóng gói thành chuỗi gửi đi
                std::string packet = MessageParser::build(infoMsg);
                
                // Gửi cho người ở lại
                server->sendToUser(pid, packet);
            }
        }

        UserDAO::removeSession(uid);  
        server->unregisterUser(uid); 
    } else {
        std::cout << "[CLEANUP] Client disconnected without logging in.\n";
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