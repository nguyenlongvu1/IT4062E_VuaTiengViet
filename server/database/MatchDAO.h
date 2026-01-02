#pragma once
#include <vector>
#include "../models/Match.h"

class MatchDAO {
public:
    static int createMatch(int roomId, const std::vector<int>& players);
    static std::vector<int> getPlayersForMatch(int matchId);
    static std::vector<int> getQuestionsForMatch(int matchId, int round);
};
