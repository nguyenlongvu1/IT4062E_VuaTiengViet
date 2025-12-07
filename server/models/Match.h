#pragma once
#include <vector>
#include <map>

struct Match {
    int matchId = 0;
    std::vector<int> players;
    std::map<int, int> scores; // userId -> score
};
