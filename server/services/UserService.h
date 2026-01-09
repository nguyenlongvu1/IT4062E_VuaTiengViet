#pragma once
#include "../core/MessageParser.h"
#include "../models/User.h"
#include <map>

class UserService {
public:
    static Message login(const Message& msg);
    static Message logout(const Message& msg);
    static Message resetPassword(const Message& msg);
    static Message registerUser(const Message& msg);
    static std::string getUsername(int userId);
    static Message searchUsers(const Message& msg);
    static Message getLeaderboard(const Message& msg);
    static Message getMatchHistory(const Message &msg);
    static Message getHistory(const Message& msg);

private:
    // Rate limiting constants
    static constexpr int MAX_FAILED_ATTEMPTS = 3;
    static constexpr int LOCK_DURATION_SECONDS = 15 * 60; // 15 minutes

    // Helper methods
    static bool validateCredentials(const std::string& username, const std::string& password, std::string& errMsg);
    static Message createErrorResponse(const std::string& errorCode, const std::string& errorMsg);
    
};

