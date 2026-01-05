#include "Dispatcher.h"
#include "../services/UserService.h"
#include "../services/FriendService.h"
#include "../services/RoomService.h"
#include "../services/MatchmakingService.h"
#include "../services/GameService.h"
#include "../services/RematchService.h"
#include <iostream>
#include "../services/MatchService.h"

Message Dispatcher::handleCommand(const Message& msg, ClientHandler*) {
    Message resp;

    // --- NHÓM USER & AUTH ---
    if (msg.command == "LOGIN") {
        resp = UserService::login(msg);
    }
    else if (msg.command == "REGISTER") {
        resp = UserService::registerUser(msg);
    }
    else if (msg.command == "LOGOUT") {
        resp = UserService::logout(msg);
    }
    else if (msg.command == "SEARCH_REQ") { 
        resp = UserService::searchUsers(msg);
    }

    // --- NHÓM BẠN BÈ (Mapping lệnh Client -> Service) ---
    else if (msg.command == "ADD_FRIEND_REQ") {
        resp = FriendService::sendFriendRequest(msg);
    }
    else if (msg.command == "ACCEPT_FRIEND_REQ") {
        resp = FriendService::acceptFriendRequest(msg);
    }
    else if (msg.command == "GET_PENDING_REQ") { 
        resp = FriendService::listPendingRequests(msg);
    }
    else if (msg.command == "GET_FRIEND_LIST") { 
        resp = FriendService::listFriends(msg);
    }

    // --- NHÓM PHÒNG (ROOM) ---
    else if (msg.command == "CREATE_ROOM") {
        resp = RoomService::createRoom(msg);
    }
    else if (msg.command == "JOIN_ROOM_REQ") {
        resp = RoomService::joinRoom(msg);
    }
    else if (msg.command == "LEAVE_ROOM_REQ") {
        resp = RoomService::leaveRoom(msg);
    }
    else if (msg.command == "GET_ROOM_INFO") {
        resp = RoomService::getRoomInfo(msg);
    }

    // --- NHÓM MATCHMAKING & GAME ---
    else if (msg.command == "FIND_MATCH" || msg.command == "FIND_MATCH_REQ") {
        resp = MatchmakingService::findMatch(msg);
    }
    else if (msg.command == "CANCEL_MATCH_REQ") { // [MỚI]
        resp = MatchmakingService::cancelMatch(msg);
    }
    else if (msg.command == "START_MATCH") {
        resp = GameService::startMatch(msg);
    }
    else if (msg.command == "ROUND1_ANSWER") {
        resp = GameService::submitAnswer(msg);
    }
    else if (msg.command == "REMATCH") {
        resp = RematchService::rematch(msg);
    }
    else if (msg.command == "GET_LEADERBOARD") {
        resp = UserService::getLeaderboard(msg);
    }
    else if (msg.command == "CREATE_MATCH") {
        resp = MatchService::createMatchFromRoom(msg);
    }
    else if (msg.command == "CHANGE_PASS") {
        resp = UserService::resetPassword(msg); 
    }
    else {
        resp.command = "ERR";
        resp.params["msg"] = "UnknownCommand: " + msg.command;
    }

    return resp;
}