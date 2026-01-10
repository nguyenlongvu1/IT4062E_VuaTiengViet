#pragma once
#include <vector>
#include <utility>
#include "../models/Match.h"
#include <string>

struct MatchLogItem {
    int matchId;
    int roundId;
    std::string questionText;
    std::string userAnswer;
    int isCorrect; // THÊM DÒNG NÀY (0 là sai, 1 là đúng)
    int points;
    std::string timestamp;
};

class MatchDAO {
public:
    static int createMatch(int roomId, const std::vector<int>& players);
    static std::vector<int> getPlayersForMatch(int matchId);
    static std::vector<int> getQuestionsForMatch(int matchId, int round);
    static bool saveMove(int matchId, int roundId, int questionId, int userId, 
                         const std::string& answer, bool isCorrect, int points);
        static bool deleteHistory(int matchId, int userId);
    static std::vector<MatchLogItem> getHistoryByUser(int userId);
    static void updateMatchScore(int matchId, int userId, int scoreChange, int rankPos);
    static std::vector<MatchLogItem> getMatchDetails(int matchId, int userId); // THÊM userId Ở ĐÂY
    static int getRankId(int matchId); // Lấy rank_id để phân biệt loại trận
};
