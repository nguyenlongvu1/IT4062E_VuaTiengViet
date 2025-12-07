#include "GameService.h"
#include "../database/QuestionDAO.h"
#include "../database/MatchDAO.h"
#include "../services/MatchService.h"
#include <iostream>
#include <algorithm>
#include <mutex>
#include <set>
#include <sstream>

// Forward declaration of GameState
struct GameState {
    int matchId;
    std::vector<int> players;
    std::map<int,int> scores;
    int round = 1;
    int qIndex = 0; // 0-based in round
    std::vector<std::vector<int>> questions; // questions[round-1][index]
    std::map<int, std::set<int>> answersForQ; // qIndex -> set of user ids who answered current q
    std::mutex mtx;
};

static std::map<int, GameState> gamesMap;
static std::mutex gamesMapMutex;

static inline std::string normalize(const std::string &s) {
    std::string out = s;
    // Unicode normalization and Vietnamese diacritics stripping should be done via a library (ICU).
    // We'll implement a simple accent removal map for common Vietnamese characters for now.
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    // naive diacritic removal: map common UTF-8 Vietnamese characters to base forms
    static const std::map<std::string, std::string> diacriticMap = {
        // a variants
        {"á","a"}, {"à","a"}, {"ả","a"}, {"ã","a"}, {"ạ","a"},
        {"ă","a"}, {"ắ","a"}, {"ằ","a"}, {"ẳ","a"}, {"ẵ","a"}, {"ặ","a"},
        {"â","a"}, {"ấ","a"}, {"ầ","a"}, {"ẩ","a"}, {"ẫ","a"}, {"ậ","a"},
        // d variants
        {"đ","d"},
        // e variants
        {"é","e"}, {"è","e"}, {"ẻ","e"}, {"ẽ","e"}, {"ẹ","e"}, {"ê","e"},
        {"ế","e"}, {"ề","e"}, {"ể","e"}, {"ễ","e"}, {"ệ","e"},
        // i variants
        {"í","i"}, {"ì","i"}, {"ỉ","i"}, {"ĩ","i"}, {"ị","i"},
        // o variants
        {"ó","o"}, {"ò","o"}, {"ỏ","o"}, {"õ","o"}, {"ọ","o"}, {"ô","o"},
        {"ố","o"}, {"ồ","o"}, {"ổ","o"}, {"ỗ","o"}, {"ộ","o"}, {"ơ","o"},
        {"ớ","o"}, {"ờ","o"}, {"ở","o"}, {"ỡ","o"}, {"ợ","o"},
        // u variants
        {"ú","u"}, {"ù","u"}, {"ủ","u"}, {"ũ","u"}, {"ụ","u"}, {"ư","u"},
        {"ứ","u"}, {"ừ","u"}, {"ử","u"}, {"ữ","u"}, {"ự","u"},
        // y variants
        {"ý","y"}, {"ỳ","y"}, {"ỷ","y"}, {"ỹ","y"}, {"ỵ","y"}
    };
    // Replace diacritics - iterate through all positions for multi-byte UTF-8 characters
    for (auto &p : diacriticMap) {
        size_t pos = 0;
        while ((pos = out.find(p.first, pos)) != std::string::npos) {
            out.replace(pos, p.first.length(), p.second);
            pos += p.second.length();
        }
    }
    // trim
    while (!out.empty() && isspace((unsigned char)out.front())) out.erase(out.begin());
    while (!out.empty() && isspace((unsigned char)out.back())) out.pop_back();
    return out;
}

Message GameService::submitAnswer(const Message &msg) {
    Message resp;
    if (msg.params.count("answer") == 0) {
        resp.command = "ERR";
        resp.params["msg"] = "Missing answer param";
        return resp;
    }
    if (msg.params.count("user_id") == 0) {
        resp.command = "ERR";
        resp.params["msg"] = "Missing user_id param";
        return resp;
    }
    int userId = std::stoi(msg.params.at("user_id"));
    std::string answer = msg.params.at("answer");
    std::string normAnswer = normalize(answer);

    bool correct = false;
    if (msg.params.count("question_id")) {
        int qid = std::stoi(msg.params.at("question_id"));
        auto oq = QuestionDAO::findById(qid);
        if (oq) {
            std::string expected = normalize(oq->correctAnswer);
            correct = (normAnswer == expected);
        }
    }

    resp.command = "ROUND1_END";
    resp.params["answer"] = answer;
    resp.params["correct"] = correct ? "true" : "false";
    resp.params["score"] = correct ? "100" : "0";
    // Update in-memory game state if match exists
    if (msg.params.count("match_id")) {
        int matchId = std::stoi(msg.params.at("match_id"));
        // find game state
        auto it = gamesMap.find(matchId);
        if (it != gamesMap.end()) {
            GameState &gs = it->second;
            std::lock_guard<std::mutex> lock(gs.mtx);
            int currentQIndex = gs.qIndex;
            std::set<int> &answered = gs.answersForQ[currentQIndex];
            answered.insert(userId);
            if (correct) gs.scores[userId] += 10;
            // check if all active players answered
            int activeCount = (int)gs.players.size();
            if ((int)answered.size() >= activeCount) {
                // advance to next question or end round
                gs.qIndex++;
                // reset answers
                gs.answersForQ[gs.qIndex];
                // attach next question to broadcast if exists
                if (gs.qIndex < (int)gs.questions[gs.round-1].size()) {
                    int nextQid = gs.questions[gs.round-1][gs.qIndex];
                    auto oq = QuestionDAO::findById(nextQid);
                    if (oq) {
                        resp.params["next_question_id"] = std::to_string(nextQid);
                        resp.params["next_question_text"] = oq->text;
                    }
                } else {
                    // end round: compute top players
                    std::vector<std::pair<int,int>> v;
                    for (auto &p : gs.scores) v.emplace_back(p.first, p.second);
                    std::sort(v.begin(), v.end(), [](auto &a, auto &b){ return a.second > b.second; });
                    // select top 2 for next round (or 1 if round 2 -> winner)
                    std::stringstream ssPlayers;
                    if (gs.round == 1) {
                        // pick top 2
                        std::vector<int> nextPlayers;
                        for (size_t i = 0; i < std::min<size_t>(2, v.size()); ++i) nextPlayers.push_back(v[i].first);
                        gs.players = nextPlayers;
                        gs.round = 2;
                        gs.qIndex = 0;
                        for (size_t i = 0; i < gs.players.size(); ++i) { if (i) ssPlayers << ","; ssPlayers << gs.players[i]; }
                        resp.params["round_end"] = "true";
                        resp.params["next_round_players"] = ssPlayers.str();
                    } else if (gs.round == 2) {
                        // top 1 -> player wins the match (stop)
                        int winner = v.size()>0 ? v[0].first : 0;
                        resp.params["match_winner"] = std::to_string(winner);
                        // finish game
                        gamesMap.erase(matchId);
                    }
                }
            }
        }
    }
    return resp;
}

// Start a match and return MATCH_CREATED with players list
Message GameService::startMatch(const Message &msg) {
    // delegate to MatchService::createMatchFromRoom to create match and players
    Message resp = MatchService::createMatchFromRoom(msg);
    if (resp.command != "MATCH_CREATED") return resp;
    int matchId = std::stoi(resp.params.at("match_id"));
    // get players
    auto players = MatchDAO::getPlayersForMatch(matchId);
    // build in-memory game state - use try_emplace to construct in-place
    auto [it, inserted] = gamesMap.try_emplace(matchId);
    GameState& gs = it->second;
    if (inserted) {
        gs.matchId = matchId;
        gs.players = players;
        for (int p : players) gs.scores[p] = 0;
        // load questions for rounds
        for (int r = 1; r <= 3; ++r) {
            gs.questions.push_back(MatchDAO::getQuestionsForMatch(matchId, r));
        }
    }

    // build GAME_START message + include first question details
    Message start;
    start.command = "GAME_START";
    start.params["match_id"] = std::to_string(matchId);
    std::stringstream ss;
    for (size_t i = 0; i < players.size(); ++i) { if (i) ss << ","; ss << players[i]; }
    start.params["players"] = ss.str();
    start.params["round"] = "1";
    if (!gs.questions.empty() && !gs.questions[0].empty()) {
        int qid = gs.questions[0][0];
        start.params["question_id"] = std::to_string(qid);
        auto oq = QuestionDAO::findById(qid);
        if (oq) start.params["question_text"] = oq->text;
    }
    start.params["broadcast"] = "true";
    return start;
}

void GameService::forfeitMatch(int matchId) {
    // placeholder: handle a player forfeit by marking match result and adjusting scores
}
