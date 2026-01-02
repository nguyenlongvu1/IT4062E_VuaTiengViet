#pragma once
#include <vector>

struct Room {
    int roomId = 0;
    std::vector<int> players; // user IDs
    bool isFull() const { return players.size() >= 3; }
};
