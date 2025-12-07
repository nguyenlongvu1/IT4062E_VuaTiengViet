#pragma once
#include "../core/MessageParser.h"
#include "../models/Room.h"
#include <map>
#include <vector>

class RoomService {
public:
    static Message createRoom(const Message& msg);
    static Message joinRoom(const Message& msg);
    static std::vector<int> getPlayers(int roomId);
    static int getPlayerCount(int roomId);
};
