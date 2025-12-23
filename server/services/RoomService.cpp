#include "RoomService.h"
#include <iostream>
#include <mutex>
#include "../services/UserService.h"
#include <algorithm>
#include "../database/MatchDAO.h"
#include "../core/Server.h"

// static int NEXT_ROOM_ID = 1;
static std::map<int, Room> roomTable;
static std::mutex room_mutex;

std::deque<int> RoomService::matchmakingQueue;

Message RoomService::createRoom(const Message&  msg) {

    int hostId = 0;
    if (msg.params.count("user_id")) {
        hostId = std::stoi(msg.params.at("user_id"));
    }

    // ================================================================
    // [FIX QUAN TRỌNG] THÊM ĐOẠN NÀY ĐỂ XÓA SẠCH DẤU VẾT CŨ
    // ================================================================
    if (hostId > 0) {
        std::lock_guard<std::mutex> lock(room_mutex);
        
        // Duyệt tất cả các phòng, thấy mặt thằng này ở đâu là xóa ngay
        for (auto& pair : roomTable) {
            Room& r = pair.second;
            auto it = std::find(r.players.begin(), r.players.end(), hostId);
            
            if (it != r.players.end()) {
                r.players.erase(it); // ĐÁ NÓ RA
                std::cout << "[FIX] Da xoa User " << hostId << " khoi phong cu " << pair.first << "\n";
            }
        }
    }
    Room room;
    // Tạo ID ngẫu nhiên từ 100000 đến 999999
    room.roomId = 100000 + (std::rand() % 900000); 


    
    // 1. Cố gắng lấy ID từ params (do Client gửi lên)
    if (msg.params.count("user_id")) {
        hostId = std::stoi(msg.params.at("user_id"));
    } 
    // 2. Nếu không có params, dùng senderId (nếu hệ thống bạn đã map socket->id)
    else {
        // hostId = msg.senderId; // Bỏ comment nếu senderId là UserID chuẩn
    }

    // THÊM NGAY LẬP TỨC
    if (hostId > 0) {
        room.players.push_back(hostId);
        std::cout << "[INFO] Created Room " << room.roomId << " with HostID: " << hostId << std::endl;
    } else {
        std::cout << "[ERROR] Created Room but HostID is 0! Client sent missing params?" << std::endl;
    }
    {
        std::lock_guard<std::mutex> lock(room_mutex);
        roomTable[room.roomId] = room;
    }

    Message resp;
    resp.command = "ROOM_CREATED";
    resp.params["room_id"] = std::to_string(room.roomId);
    return resp;
}


Message RoomService::joinRoom(const Message& msg) {
    Message resp;
    // Mặc định set lệnh trả về chuẩn
    resp.command = "JOIN_ROOM_RES";

    // 1. Kiểm tra dữ liệu đầu vào
    if (msg.params.find("room_id") == msg.params.end()) {
        resp.params["status"] = "fail";
        resp.params["reason"] = "MissingRoomID";
        return resp;
    }
    if (msg.params.find("user_id") == msg.params.end()) {
        resp.params["status"] = "fail";
        resp.params["reason"] = "MissingUserID";
        return resp;
    }

    int roomId = std::stoi(msg.params.at("room_id"));
    int userId = std::stoi(msg.params.at("user_id"));

    {
        std::lock_guard<std::mutex> lock(room_mutex);

        // 2. AUTO-LEAVE: Dọn dẹp phòng cũ (Giữ nguyên logic của bạn)
        for (auto& pair : roomTable) {
            Room& r = pair.second;
            auto it = std::find(r.players.begin(), r.players.end(), userId);
            if (it != r.players.end()) {
                r.players.erase(it);
                std::cout << "[INFO] User " << userId << " auto-left Room " << pair.first << "\n";
                // Lưu ý: Nếu muốn báo cho phòng cũ biết user này đã thoát, cũng cần code thêm ở đây
            }
        }

        // 3. Kiểm tra phòng tồn tại
        if (roomTable.find(roomId) == roomTable.end()) {
            resp.params["status"] = "fail";
            resp.params["reason"] = "PhongKhongTonTai";
            return resp;
        }

        Room& room = roomTable[roomId];

        // 4. Kiểm tra phòng đầy
        if (room.players.size() >= 3) {
            resp.params["status"] = "fail";
            resp.params["reason"] = "PhongDaDay";
            return resp;
        }

        // 5. THỰC HIỆN THÊM NGƯỜI
        room.players.push_back(userId);
        std::cout << "[INFO] User " << userId << " joined Room " << roomId << " (Count: " << room.players.size() << ")\n";

        // =================================================================
        // [PHẦN MỚI THÊM] THÔNG BÁO CHO NGƯỜI KHÁC TRONG PHÒNG
        // =================================================================
        Server* srv = Server::getInstance();
        if (srv) {
            // Tạo gói tin thông báo
            std::string notifyMsg = "COMMAND: PLAYER_JOINED_NOTIFY\n\nroom_id=" + std::to_string(roomId);
            
            // Duyệt danh sách người chơi trong phòng
            for (int pid : room.players) {
                // Chỉ gửi cho người khác (Không gửi cho chính người vừa vào)
                if (pid != userId) {
                    srv->sendToUser(pid, notifyMsg);
                }
            }
        }
        // =================================================================

        // 6. CẬP NHẬT KẾT QUẢ TRẢ VỀ (Cho người gọi)
        resp.params["status"] = "success";
        resp.params["room_id"] = std::to_string(roomId);
        resp.params["count"] = std::to_string(room.players.size());
    }
    
    return resp;
}
std::vector<int> RoomService::getPlayers(int roomId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    if (roomTable.count(roomId)) {
        return roomTable[roomId].players;
    }
    return {};
}

int RoomService::getPlayerCount(int roomId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    if (roomTable.count(roomId)) return roomTable[roomId].players.size();
    return 0;
}


Message RoomService::getRoomInfo(const Message& msg) {
    Message resp;
    
    // 1. Kiểm tra đầu vào
    if (msg.params.find("user_id") == msg.params.end()) {
        resp.command = "ERR"; resp.params["msg"] = "MissingUserID"; return resp;
    }
    int currentUserId = std::stoi(msg.params.at("user_id"));

    // 2. Tìm phòng chứa User này
    int foundRoomId = -1;
    Room foundRoom;

    {
        std::lock_guard<std::mutex> lock(room_mutex);
        for (const auto& pair : roomTable) {
            const Room& r = pair.second;
            // Tìm trong danh sách players
            for (int playerID : r.players) {
                if (playerID == currentUserId) {
                    foundRoomId = pair.first;
                    foundRoom = r; // Copy dữ liệu phòng ra để xử lý
                    break;
                }
            }
            if (foundRoomId != -1) break;
        }
    }

    if (foundRoomId == -1) {
        resp.command = "ERR"; resp.params["msg"] = "NotInAnyRoom"; return resp;
    }

    // 3. Đóng gói dữ liệu trả về (FIX LỖI TÊN RỖNG Ở ĐÂY)
    resp.command = "ROOM_INFO_RES";
    
    // --- SLOT 1: HOST (Người chơi tại index 0) ---
    if (foundRoom.players.size() >= 1) {
        resp.params["p1"] = UserService::getUsername(foundRoom.players[0]);
    } else {
        resp.params["p1"] = "";
    }

    // --- SLOT 2: GUEST 1 (Người chơi tại index 1) ---
    if (foundRoom.players.size() >= 2) {
        resp.params["p2"] = UserService::getUsername(foundRoom.players[1]);
    } else {
        resp.params["p2"] = "";
    }

    // --- SLOT 3: GUEST 2 (Người chơi tại index 2) ---
    if (foundRoom.players.size() >= 3) {
        resp.params["p3"] = UserService::getUsername(foundRoom.players[2]);
    } else {
        resp.params["p3"] = "";
    }
    return resp;
}
int RoomService::leaveRoom(int userId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    
    // Duyệt qua tất cả các phòng
    for (auto it = roomTable.begin(); it != roomTable.end(); ) {
        Room& room = it->second;
        auto& players = room.players;
        
        // Tìm user trong phòng
        auto playerIt = std::find(players.begin(), players.end(), userId);
        
        if (playerIt != players.end()) {
            // -- TÌM THẤY --
            int leftRoomId = it->first; // Lưu lại ID phòng trước khi làm gì khác

            players.erase(playerIt); // Xóa user khỏi danh sách
            std::cout << "[RoomService] User " << userId << " removed from Room " << leftRoomId << "\n";
            
            // Nếu phòng trống thì xóa luôn phòng
            if (players.empty()) {
                it = roomTable.erase(it); 
            } else {
                ++it;
            }
            
            // QUAN TRỌNG: Trả về ID phòng để bên ngoài biết mà báo tin
            return leftRoomId; 
        } else {
            ++it;
        }
    }

    return -1; // Không tìm thấy user này ở phòng nào
}
Message RoomService::leaveRoom(const Message& msg) {
    Message resp;
    resp.command = "LEAVE_ROOM_RES";

    if (msg.params.count("user_id") == 0) {
        resp.params["status"] = "fail";
        resp.params["msg"] = "Missing user_id";
        return resp;
    }

    int userId = std::stoi(msg.params.at("user_id"));

    // 1. Gọi hàm logic để xóa user khỏi danh sách & check xóa phòng
    int roomId = leaveRoom(userId);

    if (roomId != -1) {
        resp.params["status"] = "success";
        resp.params["room_id"] = std::to_string(roomId); 

        // 2. [FIX MỚI] THÔNG BÁO CHO NGƯỜI CÒN LẠI
        Server* srv = Server::getInstance();
        if (srv) {
            std::lock_guard<std::mutex> lock(room_mutex); // Khóa để đọc roomTable an toàn
            
            // Kiểm tra xem phòng còn tồn tại không (nếu người cuối cùng rời thì phòng đã bị xóa ở step 1)
            if (roomTable.find(roomId) != roomTable.end()) {
                Room& room = roomTable[roomId];
                std::string notifyMsg = "COMMAND: PLAYER_LEFT_NOTIFY\n\nroom_id=" + std::to_string(roomId);
                
                // Gửi cho tất cả những người còn lại
                for (int pid : room.players) {
                    srv->sendToUser(pid, notifyMsg);
                }
            }
        }
    } else {
        resp.params["status"] = "fail"; 
        resp.params["reason"] = "User not in any room";
    }

    return resp;
}
// --- [MỚI] TRIỂN KHAI CÁC HÀM MATCHMAKING ---

void RoomService::addToQueue(int userId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    
    // Kiểm tra xem user đã có trong hàng chưa (tránh spam)
    for (int id : matchmakingQueue) {
        if (id == userId) return;
    }
    matchmakingQueue.push_back(userId);
    std::cout << "[Matchmaking] User " << userId << " added to queue. Size: " << matchmakingQueue.size() << "\n";
}

void RoomService::removeFromQueue(int userId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    auto it = std::remove(matchmakingQueue.begin(), matchmakingQueue.end(), userId);
    if (it != matchmakingQueue.end()) {
        matchmakingQueue.erase(it, matchmakingQueue.end());
        std::cout << "[Matchmaking] User " << userId << " removed from queue.\n";
    }
}

RoomService::MatchResult RoomService::processMatchmaking() {
    std::lock_guard<std::mutex> lock(room_mutex);
    
    // 1. Kiểm tra đủ 3 người
    if (matchmakingQueue.size() < 3) {
        return { -1, -1, -1, -1, false }; 
    }

    // 2. Lấy 3 người chơi ra khỏi hàng đợi
    int p1 = matchmakingQueue.front(); matchmakingQueue.pop_front();
    int p2 = matchmakingQueue.front(); matchmakingQueue.pop_front();
    int p3 = matchmakingQueue.front(); matchmakingQueue.pop_front();

    // 3. Tạo RoomID ngẫu nhiên (6 chữ số)
    int randomRoomId = 100000 + (std::rand() % 900000);

    // 4. SỬ DỤNG MatchDAO ĐỂ KHỞI TẠO TRẬN ĐẤU TRONG DB
    // Hàm này sẽ tự động INSERT vào bảng Match, MatchPlayers (3 người) và MatchQuestions (30 câu)
    std::vector<int> players = {p1, p2, p3};
    int matchId = MatchDAO::createMatch(randomRoomId, players);

    if (matchId > 0) {
        // 5. Lưu vào bộ nhớ tạm để xử lý nhanh các yêu cầu sau đó
        Room room;
        room.roomId = randomRoomId;
        room.players = players;
        roomTable[randomRoomId] = room;

        std::cout << "[Matchmaking] Created Match " << matchId << " for Room " << randomRoomId << "\n";
        return { randomRoomId, p1, p2, p3, true };
    }

    return { -1, -1, -1, -1, false };
}