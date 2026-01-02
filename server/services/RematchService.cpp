#include "RematchService.h"
#include "RoomService.h"
#include <iostream>
#include "../core/MessageParser.h"
#include "../core/Server.h"
#include <algorithm>

static std::mutex rematch_mutex;
// map roomId -> set of user_ids who accepted rematch
static std::map<int, std::set<int>> rematchAccepts;
// track ask time
static std::map<int, std::chrono::steady_clock::time_point> rematchAskedAt;

Message RematchService::rematch(const Message& msg) {
    Message resp;

    // expected params: room_id, user_id, action=ask|accept|deny
    if (msg.params.count("room_id") == 0 || msg.params.count("user_id") == 0 || msg.params.count("action") == 0) {
        resp.command = "ERR";
        resp.params["msg"] = "Missing params";
        return resp;
    }

    int roomId = std::stoi(msg.params.at("room_id"));
    int userId = std::stoi(msg.params.at("user_id"));
    std::string action = msg.params.at("action");

    if (action == "ask") {
        // broadcast REMATCH to all players in room (this Service returns the token for dispatcher to send)
        resp.command = "REMATCH_ASK";
        resp.params["from"] = std::to_string(userId);
        resp.params["room_id"] = std::to_string(roomId);
        resp.params["broadcast"] = "true";
        
        // validate requester is in room
        auto players = RoomService::getPlayers(roomId);
        if (std::find(players.begin(), players.end(), userId) == players.end()) {
            resp.command = "ERR";
            resp.params["msg"] = "UserNotInRoom";
            return resp;
        }

        // reset previous accepts
        {
            std::lock_guard<std::mutex> lock(rematch_mutex);
            rematchAccepts.erase(roomId);
            rematchAskedAt[roomId] = std::chrono::steady_clock::now();
        }
        // start timer for rematch (auto-deny after 10 seconds by default)
        startRematchTimer(roomId, 10000);
        return resp;
    }

    if (action == "accept") {
        std::lock_guard<std::mutex> lock(rematch_mutex);
        rematchAccepts[roomId].insert(userId);

        // if all players accepted, produce REMATCH_OK
        auto players = RoomService::getPlayers(roomId);
        // validate user in room
        if (std::find(players.begin(), players.end(), userId) == players.end()) {
            resp.command = "ERR";
            resp.params["msg"] = "UserNotInRoom";
            return resp;
        }
        bool allAccepted = true;
        for (int pid : players) {
            if (rematchAccepts[roomId].count(pid) == 0) {
                allAccepted = false;
                break;
            }
        }

        if (allAccepted && players.size() > 0) {
            resp.command = "REMATCH_OK";
            resp.params["room_id"] = std::to_string(roomId);
            resp.params["broadcast"] = "true";
            resp.params["auto_start_match"] = "true";
            return resp;
        }

        resp.command = "REMATCH_ACCEPTED";
        resp.params["room_id"] = std::to_string(roomId);
        resp.params["user_id"] = std::to_string(userId);
        resp.params["broadcast"] = "true";
        return resp;
    }

    if (action == "deny") {
        std::lock_guard<std::mutex> lock(rematch_mutex);
        rematchAccepts.erase(roomId);
        rematchAskedAt.erase(roomId);
        resp.command = "REMATCH_DENIED";
        resp.params["room_id"] = std::to_string(roomId);
        resp.params["user_id"] = std::to_string(userId);
        resp.params["broadcast"] = "true";
        return resp;
    }

    resp.command = "ERR";
    resp.params["msg"] = "Unknown action";
    return resp;
}

bool RematchService::isRematchPending(int roomId) {
    std::lock_guard<std::mutex> lock(rematch_mutex);
    return rematchAccepts.count(roomId) && !rematchAccepts[roomId].empty();
}

void RematchService::startRematchTimer(int roomId, int timeoutMs) {
    // run detached thread to wait and then check
    std::thread([roomId, timeoutMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
        // check if rematch already passed
        std::lock_guard<std::mutex> lock(rematch_mutex);
        if (rematchAccepts.count(roomId) == 0) {
            // no accepts, nothing to do
            return;
        }
        // if not all accepted -> deny
        // We cannot broadcast here directly because we need Server to send messages; instead record deny and let next process
        // For simpler handling, we will erase accepts and leave it for ClientHandler to inform players when they next act.
        // prepare a REMATCH_DENIED message and broadcast to players in the room
        auto players = RoomService::getPlayers(roomId);
        rematchAccepts.erase(roomId);
        rematchAskedAt.erase(roomId);
        Message resp;
        resp.command = "REMATCH_DENIED";
        resp.params["room_id"] = std::to_string(roomId);
        resp.params["msg"] = "Timeout";
        resp.params["broadcast"] = "true";
        std::string s = MessageParser::build(resp);
        Server* srv = Server::getInstance();
        if (srv != nullptr) {
            srv->sendToUsers(players, s);
        }
    }).detach();
}
