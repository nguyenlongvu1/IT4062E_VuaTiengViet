#pragma once
#include "../core/MessageParser.h"
#include <vector>

class MatchService {
public:
    static Message createMatchFromRoom(const Message &msg);
    // Return Message containing match_id, players CSV and broadcast
};
