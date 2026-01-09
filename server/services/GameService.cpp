#include "GameService.h"
#include "../database/QuestionDAO.h"
#include "../database/UserDAO.h"
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
        resp.command = "IGNORE"; 
        return resp;
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
        int penalty = std::max(0, timeElapsed - 3);
        points = std::max(1, 10 - penalty);
        game.scores[userId] += points;
    }
    game.answered[userId] = true;

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
    if (nextRoundNeeded) {
        // Xếp hạng hiện tại (Thấp đến Cao để dễ lấy người chót)
        std::vector<std::pair<int, int>> rankList;
        for (int pid : game.players) {
            rankList.push_back({pid, game.scores[pid]});
        }
        // Sort tăng dần: Người thấp điểm nhất ở đầu [0]
        std::sort(rankList.begin(), rankList.end(), [](auto &a, auto &b){
            return a.second < b.second; 
        });

        int eliminatedId = -1;

        // --- HẾT VÒNG 1: LOẠI NGƯỜI THỨ 3 (Thấp nhất) ---
        if (game.currentRound == 1 && rankList.size() >= 3) {
            eliminatedId = rankList[0].first; // Người thấp điểm nhất
            
            // TRỪ 20 ĐIỂM RANKING
            UserDAO::addPoints(eliminatedId, -20);
            std::cout << "[GAME] Round 1 End. Eliminated User " << eliminatedId << " (-20 pts)\n";
        }
        // --- HẾT VÒNG 2: LOẠI NGƯỜI THỨ 2 (Thấp nhất trong 2 người còn lại) ---
        else if (game.currentRound == 2 && rankList.size() >= 2) {
            eliminatedId = rankList[0].first;
            
            // CỘNG 0 ĐIỂM RANKING
            UserDAO::addPoints(eliminatedId, 0); 
            std::cout << "[GAME] Round 2 End. Eliminated User " << eliminatedId << " (+0 pts)\n";
        }

        // THỰC HIỆN LOẠI BỎ KHỎI GAME
        if (eliminatedId != -1) {
            // Xóa khỏi danh sách players
            auto it = std::remove(game.players.begin(), game.players.end(), eliminatedId);
            game.players.erase(it, game.players.end());
            game.answered.erase(eliminatedId); // Xóa khỏi map trả lời

            

            // Gửi thông báo RIÊNG cho người bị loại (Gói tin đặc biệt)
            // Ta dùng cơ chế notify_id của Server.cpp để gửi riêng
            Message elimMsg;
            elimMsg.command = "ELIMINATED";
            elimMsg.params["notify_id"] = std::to_string(eliminatedId);
            elimMsg.params["notify_msg"] = "COMMAND: ELIMINATED\nLENGTH: 0\n\n"; // Gói tin gửi tới Client bị loại
            
            // Hack: Gắn kèm vào resp để Server xử lý gửi đi
            resp.params["eliminated_id"] = std::to_string(eliminatedId);
        }

        // Chuẩn bị cho vòng mới
        game.currentRound++;
        game.currentQuestionIndex = 0;
        game.questions = getQuestionsForRound(game.currentRound); // Lấy câu hỏi mới

        // Nếu chuẩn bị vào Vòng 3 -> Lưu điểm hiện tại của người sống sót để tính bonus
        if (game.currentRound == 3 && !game.players.empty()) {
            int survivorId = game.players[0];
            game.winnerScoreBeforeR3 = game.scores[survivorId];
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
            // +20 điểm Ranking + (Điểm kiếm được trong vòng 3)
            int scoreR3 = game.scores[winnerId] - game.winnerScoreBeforeR3;
            int totalReward = 20 + scoreR3;
            
            UserDAO::addPoints(winnerId, totalReward);
            std::cout << "[GAME] Winner User " << winnerId << " (+20 rank + " << scoreR3 << " bonus)\n";
        }

        resp.command = "ANSWER_RESULT";
        resp.params["game_ended"] = "true";
        resp.params["broadcast"] = "true";
        resp.params["match_id"] = std::to_string(matchId);
        resp.params["winner_id"] = std::to_string(winnerId);
        
        // Gửi Ranking giả (chỉ có người thắng) để hiển thị Result Screen
        if (winnerId != -1) {
            resp.params["rankings"] = std::to_string(winnerId) + ":" + std::to_string(game.scores[winnerId]);
        }

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
            // Chỉ gửi điểm của những người còn sống
            bool active = false;
            for(int p : game.players) if(p == uid) active = true;
            
            if (active) {
                if (c++ > 0) ssScore << ",";
                ssScore << uid << ":" << sc;
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