#include "MatchService.h"
#include "../database/MatchDAO.h"
#include "../services/RoomService.h"
#include <sstream>
#include <iostream>

Message MatchService::createMatchFromRoom(const Message &msg) {
    Message resp;
    if (msg.params.count("room_id") == 0) {
        resp.command = "ERR";
        resp.params["msg"] = "Missing room_id";
        return resp;
    }
    int roomId = std::stoi(msg.params.at("room_id"));
    auto players = RoomService::getPlayers(roomId);
    if (players.empty()) {
        resp.command = "ERR";
        resp.params["msg"] = "NoPlayersInRoom";
        return resp;
    }
    int matchId = MatchDAO::createMatch(roomId, players);
    if (!matchId) {
        resp.command = "ERR";
        resp.params["msg"] = "CreateMatchFailed";
        return resp;
    }
    resp.command = "MATCH_CREATED";
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
