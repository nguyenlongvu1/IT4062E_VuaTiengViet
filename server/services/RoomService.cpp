#include "RoomService.h"
#include <iostream>
#include <mutex>

static int NEXT_ROOM_ID = 1;
static std::map<int, Room> roomTable;
static std::mutex room_mutex;

Message RoomService::createRoom(const Message&) {
    Room room;
    room.roomId = NEXT_ROOM_ID++;
    {
        std::lock_guard<std::mutex> lock(room_mutex);
        roomTable[room.roomId] = room;
    }

    Message resp;
    resp.command = "ROOM_CREATED";
    resp.params["room_id"] = std::to_string(room.roomId);
    return resp;
}

Message RoomService::joinRoom(const Message& msg) {
    int roomId = std::stoi(msg.params.at("room_id"));
    int userId = std::stoi(msg.params.at("user_id"));

    size_t playerCount = 0;
    {
        std::lock_guard<std::mutex> lock(room_mutex);
        auto& room = roomTable[roomId];
        room.players.push_back(userId);
        playerCount = room.players.size();
    }

    Message resp;
    resp.command = "ROOM_UPDATE";
    resp.params["count"] = std::to_string(playerCount);
    resp.params["room_id"] = std::to_string(roomId);
    resp.params["broadcast"] = "true"; // let dispatcher/client handler broadcast
    return resp;
}

std::vector<int> RoomService::getPlayers(int roomId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    if (roomTable.count(roomId)) {
        return roomTable[roomId].players;
    }
    return {};
}

int RoomService::getPlayerCount(int roomId) {
    std::lock_guard<std::mutex> lock(room_mutex);
    if (roomTable.count(roomId)) return roomTable[roomId].players.size();
    return 0;
}
