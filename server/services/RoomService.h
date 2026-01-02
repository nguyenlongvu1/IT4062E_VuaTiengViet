#pragma once
#include "../core/MessageParser.h"
#include "../models/Room.h"
#include <map>
#include <vector>
#include <deque>     
#include <algorithm>
class RoomService {
public:
    static Message createRoom(const Message& msg);
    static Message joinRoom(const Message& msg);
    static Message getRoomInfo(const Message& msg);
    static Message leaveRoom(const Message& msg);
    static std::vector<int> getPlayers(int roomId);
    static int getPlayerCount(int roomId);
    static int leaveRoom(int userId);
    struct MatchResult {
        int roomId;
        int p1;
        int p2;
        int p3;
        bool success;
    };
    static void addToQueue(int userId);
    static void removeFromQueue(int userId);
    static MatchResult processMatchmaking();
private:
    static std::deque<int> matchmakingQueue;
};
