#pragma once
#include <vector>
#include <map>

struct Match {
    int matchId = 0;
    std::vector<int> players;
    std::map<int, int> scores; // userId -> score
    int roomId = 0;             // Room ID (nếu 0 = Matchmaking, khác 0 = Chơi với bạn)
    bool isRoomMatch = false;   // true = Đấu với bạn (không tính điểm), false = Matchmaking (tính điểm)
};
