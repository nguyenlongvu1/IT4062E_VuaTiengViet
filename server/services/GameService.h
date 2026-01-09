#pragma once
#include "../core/MessageParser.h"
#include <map>
#include <vector>

class GameService {
public:
    static Message submitAnswer(const Message& msg);
    static Message startMatch(const Message &msg);
    static void forfeitMatch(int matchId);
    static Message surrenderMatch(const Message &msg);
    static Message getMatchLog(const Message& msg);
};
