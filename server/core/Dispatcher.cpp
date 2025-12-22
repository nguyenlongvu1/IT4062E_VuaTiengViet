#include "Dispatcher.h"
#include "../services/UserService.h"
#include "../services/FriendService.h"
#include "../services/RoomService.h"
#include "../services/MatchmakingService.h"
#include "../services/GameService.h"
#include "../services/RematchService.h"
#include <iostream>

Message Dispatcher::handleCommand(const Message& msg, ClientHandler*) {
    Message resp;

    if (msg.command == "LOGIN") {
        resp = UserService::login(msg);
    }
    else if (msg.command == "REGISTER") {
            resp = UserService::registerUser(msg);
    }
    else if (msg.command == "LOGOUT") {
        resp = UserService::logout(msg);
    }
    else if (msg.command == "RESET_PASSWORD") {
        resp = UserService::resetPassword(msg);
    }
    else if (msg.command == "SEND_FRIEND_REQUEST") {
        resp = FriendService::sendFriendRequest(msg);
    }
    else if (msg.command == "ACCEPT_FRIEND_REQUEST") {
        resp = FriendService::acceptFriendRequest(msg);
    }
    else if (msg.command == "REJECT_FRIEND_REQUEST") {
        resp = FriendService::rejectFriendRequest(msg);
    }
    else if (msg.command == "LIST_FRIENDS") {
        resp = FriendService::listFriends(msg);
    }
    else if (msg.command == "LIST_PENDING_REQUESTS") {
        resp = FriendService::listPendingRequests(msg);
    }
    else if (msg.command == "CREATE_ROOM") {
        resp = RoomService::createRoom(msg);
    }
    else if (msg.command == "JOIN_ROOM") {
        resp = RoomService::joinRoom(msg);
    }
    else if (msg.command == "FIND_MATCH") {
        resp = MatchmakingService::findMatch(msg);
    }
    else if (msg.command == "ROUND1_ANSWER") {
        resp = GameService::submitAnswer(msg);
    }
    else if (msg.command == "START_MATCH") {
        resp = GameService::startMatch(msg);
    }
    else if (msg.command == "REMATCH") {
        resp = RematchService::rematch(msg);
    }
    else if (msg.command == "GET_ROOM_INFO") {
        resp = RoomService::getRoomInfo(msg);
    }
    else if (msg.command == "FIND_MATCH") {
        resp = MatchmakingService::findMatch(msg);
    }
    else {
        resp.command = "ERR";
        resp.params["msg"] = "UnknownCommand";
    }

    return resp;
}
