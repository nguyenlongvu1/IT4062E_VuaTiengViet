#pragma once
#include <optional>
#include <string>
#include "../models/User.h"
#include <vector>

class UserDAO {

public:
    struct UserSearchInfo {
    std::string username;
    std::string status; // "Online" hoặc "Offline"
};
struct LeaderboardInfo {
        std::string username;
        int points;
        std::string rankName;
    };
    static std::optional<User> findByUsername(const std::string &username);
    static std::optional<User> findById(int userId);
    // createUser expects the password already hashed (sha256 hex)
    static int createUser(const std::string &username, const std::string &passwordHash);
    static bool updatePassword(int userId, const std::string &newPassword);
    static bool removeSession(int userId); // placeholder to assist logout
    static bool hasActiveSession(int userId);
    static int createSession(int userId, const std::string &token);
    static bool removeSessionByToken(const std::string &token);
    // Session helpers
    static int getUserIdByToken(const std::string &token); // returns 0 if not found

    // Rate limiting methods
    static bool incrementFailedLoginAttempts(int userId);
    static bool resetFailedLoginAttempts(int userId);
    static bool lockUser(int userId, int lockDurationSeconds);
    static bool unlockUser(int userId);
    static bool isUserLocked(int userId);
    static int getFailedLoginAttempts(int userId);
    static std::vector<UserSearchInfo> searchUsers(const std::string &keyword);
    static void clearAllSessions();
    static bool addPoints(int userId, int points);
    static std::string getRankName(int points);
    static int getPoints(int userId);
    static std::vector<LeaderboardInfo> getLeaderboard(int limit = 10);
    static std::string getUsername(int userId);
    
};
