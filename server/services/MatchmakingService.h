#pragma once
#include "../core/MessageParser.h"

class MatchmakingService {
public:
    static Message findMatch(const Message& msg);
};
