#pragma once
#include "../core/MessageParser.h"
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <thread>

class RematchService {
public:
    static Message rematch(const Message& msg);
    // For testing and status
    static bool isRematchPending(int roomId);
    // start a timeout checker thread for rematch - if not all accepted within timeoutMs, then deny
    static void startRematchTimer(int roomId, int timeoutMs);
};
