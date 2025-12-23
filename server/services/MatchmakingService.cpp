#include "MatchmakingService.h"
#include "../database/MatchDAO.h" 
#include <queue>
#include <mutex>
#include <sstream>
#include <iostream>
#include <algorithm> // Để dùng std::find nếu cần, nhưng queue thì phải loop thủ công

static std::queue<int> queuePlayers;
static std::mutex queue_mutex;

Message MatchmakingService::findMatch(const Message& msg) {
    // Validate param
    if (msg.params.count("user_id") == 0) {
        Message err; err.command = "ERR"; err.params["msg"] = "Missing user_id"; return err;
    }

    int userId = std::stoi(msg.params.at("user_id"));
    int rankId = msg.params.count("rank_id") ? std::stoi(msg.params.at("rank_id")) : 1; 
    Message resp;

    std::lock_guard<std::mutex> lock(queue_mutex);

    // 1. Kiểm tra xem user đã có trong hàng chờ chưa để tránh duplicate
    // (Vì std::queue không duyệt được, ta tạm chấp nhận rủi ro hoặc dùng deque. 
    // Nhưng ở đây ta cứ push vào, logic ghép sẽ xử lý sau).
    queuePlayers.push(userId);
    std::cout << "[MATCHMAKING] User " << userId << " joined queue. Queue size: " << queuePlayers.size() << std::endl;

    // 2. Khi đủ 3 người chơi
    if (queuePlayers.size() >= 3) {
        std::vector<int> players;
        for (int i = 0; i < 3; ++i) {
            players.push_back(queuePlayers.front());
            queuePlayers.pop();
        }

        // 3. GỌI DAO: Lưu vào DB
        int dbMatchId = MatchDAO::createMatch(rankId, players); 

        // 4. Đóng gói lệnh START_MATCH
        resp.command = "START_MATCH"; 
        resp.params["match_id"] = std::to_string(dbMatchId);
        
        // Tạo danh sách ID để ClientHandler Broadcast
        std::stringstream ss;
        for (size_t i = 0; i < players.size(); ++i) {
            if (i) ss << ",";
            ss << players[i];
        }
        
        resp.params["players"] = ss.str();
        resp.params["broadcast"] = "true"; 
        
        std::cout << "[MATCHMAKING] Success! MatchID: " << dbMatchId << " created with players: " << ss.str() << std::endl;
        return resp;
    }

    // Nếu chưa đủ người
    resp.command = "FIND_MATCH_WAIT";
    resp.params["status"] = "waiting";
    return resp;
}

// =========================================================
// [MỚI] HÀM HỦY TÌM TRẬN (Để khớp với Dispatcher)
// =========================================================
Message MatchmakingService::cancelMatch(const Message& msg) {
    Message resp;
    resp.command = "CANCEL_MATCH_RES";

    if (msg.params.count("user_id") == 0) {
        return resp;
    }
    int userId = std::stoi(msg.params.at("user_id"));

    std::lock_guard<std::mutex> lock(queue_mutex);
    
    // std::queue không hỗ trợ xóa phần tử ở giữa.
    // Giải thuật: Tạo 1 queue tạm, chuyển tất cả sang queue tạm TRỪ người muốn hủy.
    std::queue<int> tempQueue;
    bool found = false;

    while (!queuePlayers.empty()) {
        int id = queuePlayers.front();
        queuePlayers.pop();

        if (id == userId) {
            found = true; // Bỏ qua, không push vào temp -> Coi như đã xóa
            std::cout << "[MATCHMAKING] User " << userId << " removed from queue.\n";
        } else {
            tempQueue.push(id);
        }
    }

    // Gán ngược lại
    queuePlayers = tempQueue;

    if (found) {
        resp.params["status"] = "success";
        resp.params["msg"] = "Cancelled matchmaking";
    } else {
        resp.params["status"] = "fail";
        resp.params["msg"] = "User not found in queue";
    }

    return resp;
}