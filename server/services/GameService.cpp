#include "GameService.h"
#include "../database/QuestionDAO.h"
#include "../database/UserDAO.h"
#include "../core/Server.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>

// Game state for active matches
struct ActiveGame {
    int matchId;
    std::vector<int> players;
    std::map<int, int> scores;          // userId -> total score
    std::vector<Question> questions;     // 10 questions
    int currentQuestionIndex = 0;        // 0-9
    std::map<int, std::chrono::steady_clock::time_point> questionStartTime; // per player
    std::map<int, bool> answered;        // userId -> has answered current question
    std::mutex mtx;
};

static std::map<int, ActiveGame> activeGames;
static std::mutex activeGamesMutex;

// Helper: Parse "option1 | option2" into vector
static std::vector<std::string> parseOptions(const std::string &content) {
    std::vector<std::string> options;
    std::stringstream ss(content);
    std::string item;
    while (std::getline(ss, item, '|')) {
        // trim
        size_t start = item.find_first_not_of(" \t");
        size_t end = item.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
            options.push_back(item.substr(start, end - start + 1));
    }
    return options;
}

Message GameService::startMatch(const Message &msg) {
    Message resp;
    
    if (msg.params.count("match_id") == 0 || msg.params.count("room_id") == 0) {
        resp.command = "ERR";
        resp.params["msg"] = "Missing match_id or room_id";
        return resp;
    }
    
    int matchId = std::stoi(msg.params.at("match_id"));
    
    // Get players from room (should be 3 players)
    std::vector<int> players;
    if (msg.params.count("players")) {
        std::stringstream ss(msg.params.at("players"));
        std::string playerId;
        while (std::getline(ss, playerId, ',')) {
            if (!playerId.empty()) players.push_back(std::stoi(playerId));
        }
    }
    
    // Allow starting with 1-3 players (enable single-player testing)
    if (players.size() < 1 || players.size() > 3) {
        resp.command = "ERR";
        resp.params["msg"] = "Players count must be between 1 and 3";
        return resp;
    }
    
    // Get 10 random questions from category "Chính tả"
    std::vector<Question> questions = QuestionDAO::getRandomQuestions("Chính tả", 10);
    
    if (questions.size() < 10) {
        resp.command = "ERR";
        resp.params["msg"] = "Not enough questions in database";
        return resp;
    }
    
    // Create active game state in-place (mutex cannot be copied)
    {
        std::lock_guard<std::mutex> lock(activeGamesMutex);
        activeGames.emplace(std::piecewise_construct,
                           std::forward_as_tuple(matchId),
                           std::forward_as_tuple());
        
        ActiveGame& game = activeGames[matchId];
        game.matchId = matchId;
        game.players = players;
        game.questions = questions;
        game.currentQuestionIndex = 0;
        
        for (int pid : players) {
            game.scores[pid] = 0;
            game.answered[pid] = false;
        }
    }
    
    // Send first question to all players (free-text answer)
    Question &q = questions[0];
    
    resp.command = "GAME_QUESTION";
    resp.params["match_id"] = std::to_string(matchId);
    resp.params["question_num"] = "1";
    resp.params["total_questions"] = "10";
    resp.params["question_id"] = std::to_string(q.id);
    resp.params["question_text"] = q.text;
    resp.params["time_limit"] = "8";
    resp.params["broadcast"] = "true";
    resp.params["players"] = msg.params.at("players");
    resp.params["scores"] = ""; // all zero at start
    
    return resp;
}

Message GameService::submitAnswer(const Message &msg) {
    Message resp;
    
    if (msg.params.count("match_id") == 0 || msg.params.count("user_id") == 0 || msg.params.count("answer") == 0) {
        resp.command = "ERR";
        resp.params["msg"] = "Missing parameters";
        return resp;
    }
    
    int matchId = std::stoi(msg.params.at("match_id"));
    int userId = std::stoi(msg.params.at("user_id"));
    std::string answer = msg.params.at("answer"); // free text
    
    auto timeElapsed = 0;
    if (msg.params.count("time_elapsed")) {
        timeElapsed = std::stoi(msg.params.at("time_elapsed"));
    }
    
    std::lock_guard<std::mutex> lock(activeGamesMutex);
    
    auto it = activeGames.find(matchId);
    if (it == activeGames.end()) {
        resp.command = "ERR";
        resp.params["msg"] = "Match not found";
        return resp;
    }
    
    ActiveGame &game = it->second;
    
    if (game.currentQuestionIndex >= 10) {
        resp.command = "ERR";
        resp.params["msg"] = "Game already ended";
        return resp;
    }
    
    Question &currentQ = game.questions[game.currentQuestionIndex];
    // Case-insensitive compare to correctAnswer
    auto toLowerTrim = [](const std::string &s) {
        size_t start = s.find_first_not_of(" \t\n\r");
        size_t end = s.find_last_not_of(" \t\n\r");
        std::string sub = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
        std::string lower;
        lower.reserve(sub.size());
        for (char c : sub) lower.push_back(std::tolower(static_cast<unsigned char>(c)));
        return lower;
    };

    bool isCorrect = (toLowerTrim(answer) == toLowerTrim(currentQ.correctAnswer));
    int points = 0;
    
    if (isCorrect) {
        // 8s per question, no penalty first 3s, then -1 point each second after
        int penalty = std::max(0, timeElapsed - 3);
        points = std::max(0, 10 - penalty);
        game.scores[userId] += points;
    }
    
    game.answered[userId] = true;
    
    // Check if all players have answered
    int answeredCount = 0;
    for (auto &p : game.answered) {
        if (p.second) answeredCount++;
    }
    
    resp.command = "ANSWER_RESULT";
    resp.params["match_id"] = std::to_string(matchId);
    resp.params["user_id"] = std::to_string(userId);
    resp.params["correct"] = isCorrect ? "true" : "false";
    resp.params["points_earned"] = std::to_string(points);
    resp.params["total_score"] = std::to_string(game.scores[userId]);
    
    // If all players answered, move to next question or end game
    if (answeredCount >= (int)game.players.size()) {
        game.currentQuestionIndex++;
        
        // Reset answered flags
        for (int pid : game.players) {
            game.answered[pid] = false;
        }
        
        if (game.currentQuestionIndex < 10) {
            // Send next question
            Question &nextQ = game.questions[game.currentQuestionIndex];
            
            resp.params["next_question"] = "true";
            resp.params["question_num"] = std::to_string(game.currentQuestionIndex + 1);
            resp.params["question_id"] = std::to_string(nextQ.id);
            resp.params["question_text"] = nextQ.text;
            resp.params["time_limit"] = "8";
        } else {
            // Game ended, send final results
            resp.params["game_ended"] = "true";
            
            // Sort players by score
            std::vector<std::pair<int, int>> rankings; // userId, score
            for (auto &p : game.scores) {
                rankings.push_back({p.first, p.second});
            }
            std::sort(rankings.begin(), rankings.end(), [](auto &a, auto &b) {
                return a.second > b.second; // descending
            });
            
            // Update total_points in DB
            for (auto &r : rankings) {
                UserDAO::addPoints(r.first, r.second);
            }
            
            // Build ranking string: "userId1:score1,userId2:score2,userId3:score3"
            std::stringstream ss;
            for (size_t i = 0; i < rankings.size(); i++) {
                if (i > 0) ss << ",";
                ss << rankings[i].first << ":" << rankings[i].second;
            }
            resp.params["rankings"] = ss.str();
            
            if (rankings.size() > 0) {
                resp.params["winner_id"] = std::to_string(rankings[0].first);
            }
            
            // Remove game from active games
            activeGames.erase(matchId);
        }
        
        resp.params["broadcast"] = "true";
        resp.params["players"] = "";
        for (size_t i = 0; i < game.players.size(); i++) {
            if (i > 0) resp.params["players"] += ",";
            resp.params["players"] += std::to_string(game.players[i]);
        }
        // Include live scores for UI display
        std::stringstream scoreSs;
        size_t idx = 0;
        for (auto &kv : game.scores) {
            if (idx++ > 0) scoreSs << ",";
            scoreSs << kv.first << ":" << kv.second;
        }
        resp.params["scores"] = scoreSs.str();
    }
    
    return resp;
}

void GameService::forfeitMatch(int matchId) {
    std::lock_guard<std::mutex> lock(activeGamesMutex);
    activeGames.erase(matchId);
}
