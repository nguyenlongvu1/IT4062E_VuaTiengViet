#include "MatchmakingService.h"
#include "../database/MatchDAO.h" 
#include <queue>
#include <mutex>
#include <sstream>
#include <iostream>
#include "../services/GameService.h"
#include <algorithm> 

static std::queue<int> queuePlayers;
static std::mutex queue_mutex;

Message MatchmakingService::findMatch(const Message& msg) {
    if (msg.params.count("user_id") == 0) {
        Message err; err.command = "ERR"; err.params["msg"] = "Missing user_id"; return err;
    }

    int userId = std::stoi(msg.params.at("user_id"));
    int rankId = msg.params.count("rank_id") ? std::stoi(msg.params.at("rank_id")) : 1; 
    Message resp;

    std::lock_guard<std::mutex> lock(queue_mutex);

    // --- SỬA 1: CHỐNG DUPLICATE ID TRONG QUEUE ---
    std::queue<int> checkQueue = queuePlayers;
    bool alreadyInQueue = false;
    while(!checkQueue.empty()){
        if(checkQueue.front() == userId) { alreadyInQueue = true; break; }
        checkQueue.pop();
    }

    if (!alreadyInQueue) {
        queuePlayers.push(userId);
        std::cout << "[MATCHMAKING] User " << userId << " joined queue. Queue size: " << queuePlayers.size() << std::endl;
    }

    // --- SỬA 2: CHỈ TRẢ VỀ START_MATCH KHI ĐỦ 3 NGƯỜI ---
    if (queuePlayers.size() >= 3) {
        std::vector<int> players;
        for (int i = 0; i < 3; ++i) {
            players.push_back(queuePlayers.front());
            queuePlayers.pop();
        }

        int dbMatchId = MatchDAO::createMatch(rankId, players); 

        resp.command = "START_MATCH"; 
        resp.params["match_id"] = std::to_string(dbMatchId);
        resp.params["room_id"] = std::to_string(rankId);
        resp.params["broadcast"] = "true"; // Để ClientHandler biết đường gửi cho cả 3
        
        std::stringstream ss;
        for (size_t i = 0; i < players.size(); ++i) {
            if (i) ss << ",";
            ss << players[i];
        }
        resp.params["players"] = ss.str(); 
        
        return resp;
    }

    // Nếu chưa đủ người, CHỈ trả về cho đúng người đang gửi yêu cầu
    resp.command = "FIND_MATCH_WAIT";
    resp.params["status"] = "waiting";
    resp.params["broadcast"] = "false"; // QUAN TRỌNG: Không broadcast cái này
    return resp;
}

//HÀM HỦY TÌM TRẬN (Để khớp với Dispatcher)
Message MatchmakingService::cancelMatch(const Message& msg) {
    Message resp;
    resp.command = "CANCEL_MATCH_RES";

    if (msg.params.count("user_id") == 0) {
        return resp;
    }
    int userId = std::stoi(msg.params.at("user_id"));

    std::lock_guard<std::mutex> lock(queue_mutex);

    std::queue<int> tempQueue;
    bool found = false;

    while (!queuePlayers.empty()) {
        int id = queuePlayers.front();
        queuePlayers.pop();

        if (id == userId) {
            found = true; 
            std::cout << "[MATCHMAKING] User " << userId << " removed from queue.\n";
        } else {
            tempQueue.push(id);
        }
    }
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