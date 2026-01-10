#include "GameService.h"
#include "../database/QuestionDAO.h"
#include "../database/UserDAO.h"
#include "../database/MatchDAO.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>

// --- CẤU TRÚC LƯU TRẠNG THÁI GAME ---
struct ActiveGame {
    int matchId;
    std::vector<int> players;           
    std::map<int, int> scores;          
    std::vector<Question> questions;    
    int currentQuestionIndex = 0;       
    int currentRound = 1;               
    std::map<int, bool> answered;       
    std::mutex gameMutex; 

    int winnerScoreBeforeR3 = 0;
    int rankId = 1; // 1 = Matchmaking (tính điểm), khác = Phòng bạn (không tính)
};

static std::map<int, ActiveGame> activeGames;
static std::mutex globalGamesMutex; 

// --- HÀM HELPER: Lấy danh sách câu hỏi ---
static std::vector<Question> getQuestionsForRound(int round) {
    std::string category;
    int count = 10; // Cấu hình 10 câu
    
    switch(round) {
        case 1: category = "Chính tả"; break;
        case 2: category = "Sắp xếp";  break;
        case 3: category = "Điền từ";  break;
        default: category = "Chính tả"; break;
    }
    return QuestionDAO::getRandomQuestions(category, count);
}

// --- HÀM HELPER: Chuẩn hóa chuỗi ---
static std::string normalizeString(const std::string &s) {
    std::string temp = s;
    size_t first = temp.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = temp.find_last_not_of(" \t\r\n");
    temp = temp.substr(first, (last - first + 1));
    std::transform(temp.begin(), temp.end(), temp.begin(), 
                   [](unsigned char c){ return std::tolower(c); });
    return temp;
}

// =================================================================================
// 1. START MATCH
// =================================================================================
Message GameService::startMatch(const Message &msg) {
    Message resp;

    if (msg.params.find("match_id") == msg.params.end()) {
        resp.command = "ERR"; resp.params["msg"] = "Missing match_id"; return resp;
    }
    int matchId = std::stoi(msg.params.at("match_id"));

    std::lock_guard<std::mutex> lock(globalGamesMutex);

    // [CHỐNG SPAM]: Nếu game đã có -> IGNORE
    if (activeGames.count(matchId) > 0) {
        std::cout << "[WARNING] Match ID " << matchId << " exists. Overwriting/Resetting...\n";
        activeGames.erase(matchId);
    }

    // TẠO GAME MỚI
    std::vector<int> players;
    if (msg.params.count("players")) {
        std::stringstream ss(msg.params.at("players"));
        std::string pid;
        while (std::getline(ss, pid, ',')) {
            if (!pid.empty()) players.push_back(std::stoi(pid));
        }
    }

    if (players.empty()) {
        resp.command = "ERR"; resp.params["msg"] = "No players"; return resp;
    }

    std::vector<Question> questions = getQuestionsForRound(1);
    if (questions.empty()) {
        resp.command = "ERR"; resp.params["msg"] = "Database empty"; return resp;
    }

    activeGames.emplace(std::piecewise_construct,
                        std::forward_as_tuple(matchId),
                        std::forward_as_tuple());
    
    ActiveGame& newGame = activeGames[matchId];
    newGame.matchId = matchId;
    newGame.players = players;
    newGame.questions = questions;
    newGame.currentQuestionIndex = 0;
    newGame.currentRound = 1;
    newGame.rankId = MatchDAO::getRankId(matchId); // Lấy rank_id từ DB

    for (int p : players) {
        newGame.scores[p] = 0;
        newGame.answered[p] = false;
    }

    // Gửi câu hỏi đầu tiên
    Question &firstQ = questions[0];
    resp.command = "GAME_QUESTION";
    resp.params["match_id"] = std::to_string(matchId);
    resp.params["round_id"] = "1";
    resp.params["question_num"] = "1";
    resp.params["question_id"] = std::to_string(firstQ.id);
    resp.params["time_limit"] = "10";
    resp.params["broadcast"] = "true";
    resp.params["players"] = msg.params.at("players");
    
    resp.params["question_text"] = "Chọn từ viết đúng chính tả:";
    resp.params["options"] = firstQ.text;

    return resp;
}

// =================================================================================
// 2. SUBMIT ANSWER (ĐÃ SỬA LOGIC CHUYỂN CÂU)
// =================================================================================
Message GameService::submitAnswer(const Message &msg) {
    Message resp;

    // 1. Validate Input
    if (msg.params.find("match_id") == msg.params.end() || 
        msg.params.find("user_id") == msg.params.end() || 
        msg.params.find("answer") == msg.params.end()) {
        return resp; // Silent fail
    }

    int matchId = std::stoi(msg.params.at("match_id"));
    int userId = std::stoi(msg.params.at("user_id"));
    std::string answer = msg.params.at("answer");
    int timeElapsed = msg.params.count("time_elapsed") ? std::stoi(msg.params.at("time_elapsed")) : 10;

    std::lock_guard<std::mutex> globalLock(globalGamesMutex);
    if (activeGames.count(matchId) == 0) return resp;

    ActiveGame &game = activeGames[matchId];
    std::lock_guard<std::mutex> gameLock(game.gameMutex);

    // 2. Chấm điểm (Chỉ tính nếu người chơi còn trong danh sách)
    bool isPlayerActive = false;
    for(int pid : game.players) if(pid == userId) isPlayerActive = true;
    
    if (!isPlayerActive) return resp; // Người đã bị loại nộp bài -> Bỏ qua

    Question &currQ = game.questions[game.currentQuestionIndex];
    bool isCorrect = (normalizeString(answer) == normalizeString(currQ.correctAnswer));
    int points = 0;

    if (isCorrect) {
        // Vòng 3: mỗi câu đúng chỉ +1 điểm
        if (game.currentRound == 3) {
            points = 1;
        } else {
            int penalty = std::max(0, timeElapsed - 3);
            points = std::max(1, 10 - penalty);
        }
        game.scores[userId] += points;
    }
    game.answered[userId] = true;
    MatchDAO::saveMove(
        matchId, 
        game.currentRound, 
        currQ.id, 
        userId, 
        answer, 
        isCorrect, 
        points
    );

    // 3. Kiểm tra đủ người trả lời chưa
    int answeredCount = 0;
    for (int pid : game.players) {
        if (game.answered[pid]) answeredCount++;
    }

    // --- CHƯA ĐỦ NGƯỜI -> GỬI KẾT QUẢ CÁ NHÂN ---
    if (answeredCount < (int)game.players.size()) {
        resp.command = "ANSWER_RESULT";
        resp.params["match_id"] = std::to_string(matchId);
        resp.params["user_id"] = std::to_string(userId);
        resp.params["correct"] = isCorrect ? "true" : "false";
        resp.params["points_earned"] = std::to_string(points);
        resp.params["total_score"] = std::to_string(game.scores[userId]);
        resp.params["broadcast"] = "false";
        return resp; 
    }

    // --- ĐỦ NGƯỜI -> CHUẨN BỊ QUA CÂU MỚI HOẶC VÒNG MỚI ---
    game.currentQuestionIndex++;
    for (int pid : game.players) game.answered[pid] = false; // Reset cờ

    bool nextRoundNeeded = false;
    bool gameEnded = false;

    // Kiểm tra hết câu hỏi trong vòng (Index >= 10)
    if (game.currentQuestionIndex >= (int)game.questions.size()) {
        if (game.currentRound < 3) nextRoundNeeded = true;
        else gameEnded = true;
    }

    // ========================================================================
    // LOGIC LOẠI NGƯỜI (BATTLE ROYALE) KHI CHUYỂN VÒNG
    // ========================================================================
    // --- LOGIC LOẠI NGƯỜI (BATTLE ROYALE) ---
    if (nextRoundNeeded) {
        int scoreChange = 0;
        std::vector<std::pair<int, int>> rankList;
        // Chỉ lấy những người ĐANG CHƠI (chưa bị loại) để xét duyệt
        for (int pid : game.players) {
            rankList.push_back({pid, game.scores[pid]});
        }

        // [QUAN TRỌNG]: Sắp xếp từ THẤP ĐẾN CAO (a.second < b.second)
        // Để rankList[0] sẽ là người thấp điểm nhất
        std::sort(rankList.begin(), rankList.end(), [](auto &a, auto &b){ 
            return a.second < b.second; 
        });

        int eliminatedId = -1;
        // CHỈ TÍNH ĐIỂM NẾU LÀ MATCHMAKING (rank_id = 1)
        bool shouldCountPoints = (game.rankId == 1);
        
        // Nếu vòng 1 và còn >= 3 người -> Loại người bét (rankList[0])
        if (game.currentRound == 1 && rankList.size() >= 3) {
            eliminatedId = rankList[0].first;
            scoreChange = -20;
            
            // Chỉ cộng điểm nếu là matchmaking
            if (shouldCountPoints) {
                UserDAO::addPoints(eliminatedId, scoreChange);
            }
            MatchDAO::updateMatchScore(matchId, eliminatedId, scoreChange, (int)rankList.size());
        }
        // Nếu vòng 2 và còn >= 2 người -> Loại người bét (rankList[0])
        else if (game.currentRound == 2 && rankList.size() >= 2) {
            eliminatedId = rankList[0].first;
            scoreChange = 0; // Điểm biến động
            
            // Chỉ cộng điểm nếu là matchmaking
            if (shouldCountPoints) {
                UserDAO::addPoints(eliminatedId, scoreChange);
            }
            MatchDAO::updateMatchScore(matchId, eliminatedId, scoreChange, (int)rankList.size());
        }

        if (eliminatedId != -1) {
           UserDAO::addPoints(eliminatedId, scoreChange);

            // Xóa người chơi khỏi danh sách
            auto it = std::remove(game.players.begin(), game.players.end(), eliminatedId);
            game.players.erase(it, game.players.end());
            game.answered.erase(eliminatedId);
            
            // Gắn cờ báo cho Server biết để gửi tin ELIMINATED
            resp.params["eliminated_id"] = std::to_string(eliminatedId);
        }

        game.currentRound++;
        game.currentQuestionIndex = 0;
        game.questions = getQuestionsForRound(game.currentRound);
        
        // Lưu điểm người sống sót vào vòng 3 để tính bonus sau này
        if (game.currentRound == 3 && !game.players.empty()) {
            game.winnerScoreBeforeR3 = game.scores[game.players[0]];
        }
    }

    // ========================================================================
    // LOGIC KẾT THÚC GAME (HẾT VÒNG 3)
    // ========================================================================
    if (gameEnded) {
        int winnerId = -1;
        if (!game.players.empty()) winnerId = game.players[0];

        if (winnerId != -1) {
            // TÍNH ĐIỂM NGƯỜI THẮNG CUỘC
            // +20 điểm Ranking + (Điểm kiếm được trong vòng 3 - mỗi câu +1)
            int scoreR3 = game.scores[winnerId] - game.winnerScoreBeforeR3;
            int totalReward = 20 + scoreR3;
            
            // CHỈ CỘNG ĐIỂM NẾU LÀ MATCHMAKING (rank_id = 1)
            if (game.rankId == 1) {
                UserDAO::addPoints(winnerId, totalReward);
                std::cout << "[GAME] Winner User " << winnerId << " (+20 rank + " << scoreR3 << " bonus)\n";
            } else {
                std::cout << "[GAME] Winner User " << winnerId << " (Phòng bạn - không tính điểm)\n";
            }
            MatchDAO::updateMatchScore(matchId, winnerId, totalReward, 1);
        }

        resp.command = "ANSWER_RESULT";
        resp.params["game_ended"] = "true";
        resp.params["broadcast"] = "true";
        resp.params["match_id"] = std::to_string(matchId);
        resp.params["winner_id"] = std::to_string(winnerId);
        
        // Gửi Ranking giả (chỉ có người thắng) để hiển thị Result Screen
        if (winnerId != -1) {
            resp.params["winner_id"] = UserDAO::getUsername(winnerId); // Gửi Tên
        }
        std::vector<std::pair<int, int>> fullRankList;
        for (auto const& [uid, score] : game.scores) {
            fullRankList.push_back({uid, score});
        }
        std::sort(fullRankList.begin(), fullRankList.end(), [](auto &a, auto &b){ 
            return a.second > b.second; // Cao -> Thấp
        });

        std::stringstream ss;
        for (size_t i = 0; i < fullRankList.size(); ++i) {
            if (i > 0) ss << ",";
            // [HIỆN TÊN]: Convert ID sang Tên
            std::string uName = UserDAO::getUsername(fullRankList[i].first);
            ss << uName << ":" << fullRankList[i].second;
        }
        resp.params["rankings"] = ss.str();
        // Danh sách người nhận tin (chỉ còn người thắng)
        std::stringstream ssPlayers;
        for (size_t i = 0; i < game.players.size(); ++i) {
            if (i > 0) ssPlayers << ",";
            ssPlayers << game.players[i];
        }
        resp.params["players"] = ssPlayers.str();

        activeGames.erase(matchId); 
    }
    // ========================================================================
    // LOGIC GỬI CÂU HỎI TIẾP THEO (CHO NHỮNG NGƯỜI CÒN SỐNG)
    // ========================================================================
    else {
        Question &nextQ = game.questions[game.currentQuestionIndex];
        
        resp.command = "GAME_QUESTION";
        resp.params["match_id"] = std::to_string(matchId);
        resp.params["broadcast"] = "true"; 
        resp.params["round_id"] = std::to_string(game.currentRound);
        resp.params["question_num"] = std::to_string(game.currentQuestionIndex + 1);
        resp.params["question_id"] = std::to_string(nextQ.id);
        resp.params["time_limit"] = "10";

        // Gửi danh sách players CÒN SỐNG để Server broadcast đúng người
        std::stringstream ssPlayers;
        for (size_t i = 0; i < game.players.size(); ++i) {
            if (i > 0) ssPlayers << ",";
            ssPlayers << game.players[i];
        }
        resp.params["players"] = ssPlayers.str();

        if (game.currentRound == 1) {
            resp.params["question_text"] = "Chọn từ viết đúng chính tả:";
            resp.params["options"] = nextQ.text;
        } else {
            resp.params["question_text"] = nextQ.text;
            resp.params["options"] = ""; 
        }
        
        // Gửi điểm số
        std::stringstream ssScore;
        int c = 0;
        for (auto const& [uid, sc] : game.scores) {
            if (c++ > 0) ssScore << ",";
            
            std::string uName = UserDAO::getUsername(uid);
            bool isActive = false;
            for(int p : game.players) if(p == uid) isActive = true;

            if (!isActive) {
                ssScore << uName << " (Loại):" << sc;
            } else {
                // NẾU LÀ VÒNG 3: HIỆN DẠNG "BASE + (BONUS)"
                if (game.currentRound == 3) {
                    int base = game.winnerScoreBeforeR3;
                    int bonus = sc - base;
                    ssScore << uName << ":" << base << " + (" << bonus << ")";
                } else {
                    ssScore << uName << ":" << sc;
                }
            }
        }
        resp.params["scores"] = ssScore.str();
    }

    return resp;
}

void GameService::forfeitMatch(int matchId) {
    std::lock_guard<std::mutex> lock(globalGamesMutex);
    activeGames.erase(matchId);
}
Message GameService::surrenderMatch(const Message &msg) {
    Message resp;
    // Validate
    if (msg.params.find("match_id") == msg.params.end() || msg.params.find("user_id") == msg.params.end()) {
        return resp; 
    }

    int matchId = std::stoi(msg.params.at("match_id"));
    int userId = std::stoi(msg.params.at("user_id"));

    std::lock_guard<std::mutex> globalLock(globalGamesMutex);
    if (activeGames.count(matchId) == 0) return resp;

    ActiveGame &game = activeGames[matchId];
    std::lock_guard<std::mutex> gameLock(game.gameMutex);

    // 1. Set điểm về 0 (Theo yêu cầu)
    game.scores[userId] = 0;
    
    // 2. Cập nhật DB (Lưu trận thua/điểm 0)
    UserDAO::addPoints(userId, 0); // Hoặc trừ điểm nếu muốn

    // 3. [QUAN TRỌNG] Xóa người chơi khỏi danh sách để những người khác không phải đợi
    auto it = std::remove(game.players.begin(), game.players.end(), userId);
    if (it != game.players.end()) {
        game.players.erase(it, game.players.end());
    }
    
    // Xóa trạng thái đã trả lời
    game.answered.erase(userId);

    // 4. Tạo thông báo cho những người còn lại
    resp.command = "PLAYER_SURRENDERED";
    resp.params["broadcast"] = "true";
    resp.params["user_id"] = std::to_string(userId);
    resp.params["match_id"] = std::to_string(matchId);
    
    // Gửi danh sách người còn lại để Server biết đường broadcast
    std::stringstream ssPlayers;
    for (size_t i = 0; i < game.players.size(); ++i) {
        if (i > 0) ssPlayers << ",";
        ssPlayers << game.players[i];
    }
    resp.params["players"] = ssPlayers.str();

    // 5. [CHECK LOGIC]: Nếu sau khi ông này thoát mà những người còn lại ĐÃ TRẢ LỜI XONG HẾT
    // Thì phải kích hoạt chuyển câu hỏi ngay lập tức (Logic này hơi phức tạp, 
    // để đơn giản thì người còn lại nộp bài sẽ tự kích hoạt next question).
    
    // Nếu phòng trống rỗng -> Xóa game
    if (game.players.empty()) {
        activeGames.erase(matchId);
    }

    return resp;
}
Message GameService::getMatchLog(const Message& msg) {
    Message resp;
    resp.command = "MATCH_LOG_DATA";

    try {
        int matchId = std::stoi(msg.params.at("match_id"));
        int userId = std::stoi(msg.params.at("user_id")); 

        std::vector<MatchLogItem> details = MatchDAO::getMatchDetails(matchId, userId);
        
        std::stringstream ss;
        for (const auto& item : details) {
            // Thay dấu ; cuối dòng bằng \n
            ss << item.roundId << "|" 
               << item.userAnswer << "|" 
               << (item.isCorrect ? "ĐÚNG" : "SAI") << "|" 
               << item.points << "\n"; // Dùng \n ở đây
        }
        resp.params["data"] = ss.str();
    } catch (...) {
        resp.params["data"] = "EMPTY";
    }
    return resp;
}