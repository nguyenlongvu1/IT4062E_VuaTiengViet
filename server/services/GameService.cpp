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

    if (msg.params.find("match_id") == msg.params.end() || 
        msg.params.find("user_id") == msg.params.end() || 
        msg.params.find("answer") == msg.params.end()) {
        resp.command = "ERR"; resp.params["msg"] = "Missing params"; return resp;
    }

    int matchId = std::stoi(msg.params.at("match_id"));
    int userId = std::stoi(msg.params.at("user_id"));
    std::string answer = msg.params.at("answer");
    int timeElapsed = msg.params.count("time_elapsed") ? std::stoi(msg.params.at("time_elapsed")) : 10;

    std::lock_guard<std::mutex> globalLock(globalGamesMutex);
    if (activeGames.count(matchId) == 0) {
        resp.command = "ERR"; resp.params["msg"] = "Match ended"; return resp;
    }

    ActiveGame &game = activeGames[matchId];
    std::lock_guard<std::mutex> gameLock(game.gameMutex);

    // --- 1. CHẤM ĐIỂM ---
    Question &currQ = game.questions[game.currentQuestionIndex];
    bool isCorrect = (normalizeString(answer) == normalizeString(currQ.correctAnswer));
    int points = 0;

    if (isCorrect) {
        int penalty = std::max(0, timeElapsed - 3);
        points = std::max(1, 10 - penalty);
        game.scores[userId] += points;
    }
    game.answered[userId] = true;

    // --- 2. KIỂM TRA SỐ LƯỢNG NGƯỜI ĐÃ TRẢ LỜI ---
    int answeredCount = 0;
    for (int pid : game.players) {
        if (game.answered[pid]) answeredCount++;
    }

    // [TRƯỜNG HỢP A]: CHƯA ĐỦ NGƯỜI -> GỬI KẾT QUẢ ĐÚNG/SAI
    if (answeredCount < (int)game.players.size()) {
        resp.command = "ANSWER_RESULT";
        resp.params["match_id"] = std::to_string(matchId);
        resp.params["user_id"] = std::to_string(userId);
        resp.params["correct"] = isCorrect ? "true" : "false";
        resp.params["points_earned"] = std::to_string(points);
        resp.params["total_score"] = std::to_string(game.scores[userId]);
        resp.params["broadcast"] = "true";
        {
           std::stringstream ss;
            for (size_t i = 0; i < game.players.size(); ++i) {
                if (i) ss << ",";
                ss << game.players[i];
            }
            resp.params["players"] = ss.str();
        }
        return resp; 
    }

    // [TRƯỜNG HỢP B]: ĐỦ NGƯỜI RỒI -> CHUYỂN NGAY SANG CÂU HỎI MỚI (GAME_QUESTION)
    // Cách này đảm bảo tất cả Client đều nhận được lệnh chuyển màn hình
    
    game.currentQuestionIndex++;
    for (int pid : game.players) game.answered[pid] = false;

    // Logic chuyển vòng/kết thúc
    bool nextRoundNeeded = false;
    bool gameEnded = false;

    if (game.currentQuestionIndex >= (int)game.questions.size()) {
        if (game.currentRound < 3) nextRoundNeeded = true;
        else gameEnded = true;
    }

    // --- XỬ LÝ HẾT GAME ---
    if (gameEnded) {
        resp.command = "ANSWER_RESULT"; // Quay về lệnh này để báo hết game
        resp.params["game_ended"] = "true";
        resp.params["broadcast"] = "true";
        resp.params["match_id"] = std::to_string(matchId);

        {
            std::stringstream ss;
            for (size_t i = 0; i < game.players.size(); ++i) {
                if (i) ss << ",";
                ss << game.players[i];
            }
            resp.params["players"] = ss.str();
        }

        // Xếp hạng
        std::vector<std::pair<int, int>> rankList;
        for (auto const& [uid, score] : game.scores) rankList.push_back({uid, score});
        std::sort(rankList.begin(), rankList.end(), [](auto &a, auto &b){ return a.second > b.second; });

        for (auto &r : rankList) UserDAO::addPoints(r.first, r.second);

        std::stringstream ss;
        for (size_t i = 0; i < rankList.size(); ++i) {
            if (i > 0) ss << ",";
            ss << rankList[i].first << ":" << rankList[i].second;
        }
        resp.params["rankings"] = ss.str();
        if(!rankList.empty()) resp.params["winner_id"] = std::to_string(rankList[0].first);
        
        activeGames.erase(matchId); 
    } 
    // --- XỬ LÝ CÂU HỎI TIẾP THEO ---
    else {
        if (nextRoundNeeded) {
            game.currentRound++;
            game.currentQuestionIndex = 0;
            game.questions = getQuestionsForRound(game.currentRound);
            if (game.questions.empty()) {
                resp.command = "ANSWER_RESULT";
                resp.params["game_ended"] = "true"; 
                activeGames.erase(matchId);
                return resp;
            }
        }

        Question &nextQ = game.questions[game.currentQuestionIndex];
        
        // [FIX]: GỬI THẲNG LỆNH GAME_QUESTION
        resp.command = "GAME_QUESTION";
        resp.params["match_id"] = std::to_string(matchId);
        resp.params["broadcast"] = "true";

       {
         std::stringstream ss;
            for (size_t i = 0; i < game.players.size(); ++i) {
                if (i) ss << ",";
                ss << game.players[i];
            }
            resp.params["players"] = ss.str();
        }
        resp.params["round_id"] = std::to_string(game.currentRound);
        resp.params["question_num"] = std::to_string(game.currentQuestionIndex + 1);
        resp.params["question_id"] = std::to_string(nextQ.id);
        resp.params["time_limit"] = "10";

        if (game.currentRound == 1) {
            resp.params["question_text"] = "Chọn từ viết đúng chính tả:";
            resp.params["options"] = nextQ.text;
        } else {
            resp.params["question_text"] = nextQ.text;
            resp.params["options"] = ""; 
        }
        
        // Gửi kèm điểm số mới nhất để Client cập nhật
        std::stringstream ssScore;
        int c = 0;
        for (auto const& [uid, sc] : game.scores) {
            if (c++ > 0) ssScore << ",";
            ssScore << uid << ":" << sc;
        }
        resp.params["scores"] = ssScore.str();
    }

    return resp;
}

void GameService::forfeitMatch(int matchId) {
    std::lock_guard<std::mutex> lock(globalGamesMutex);
    activeGames.erase(matchId);
}