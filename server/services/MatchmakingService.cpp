#include "MatchmakingService.h"
#include <queue>
#include <mutex>
#include <sstream>

static std::queue<int> queuePlayers;
static std::mutex queue_mutex;
static int NEXT_MATCH_ID = 1;

Message MatchmakingService::findMatch(const Message& msg) {
    int userId = std::stoi(msg.params.at("user_id"));
    Message resp;

    std::lock_guard<std::mutex> lock(queue_mutex);
    queuePlayers.push(userId);

    if (queuePlayers.size() >= 3) {
        std::vector<int> players;
        for (int i = 0; i < 3; ++i) {
            players.push_back(queuePlayers.front());
            queuePlayers.pop();
        }

        int matchId = NEXT_MATCH_ID++;
        resp.command = "MATCH_FOUND";
        resp.params["match_id"] = std::to_string(matchId);
        std::stringstream ss;
        for (size_t i = 0; i < players.size(); ++i) {
            if (i) ss << ",";
            ss << players[i];
        }
        resp.params["players"] = ss.str();
        resp.params["broadcast"] = "true";
        return resp;
    }

    resp.command = "FIND_MATCH_WAIT";
    return resp;
}
