#include "UserService.h"
#include "../database/UserDAO.h"
#include "../database/DB.h"
#include <iostream>
#include "../utils/Crypto.h"
#include <regex>
#include "../database/FriendDAO.h"
#include "../core/Server.h"
#include "../core/ClientHandler.h"
// Helper: Unified error response
Message UserService::createErrorResponse(const std::string& errorCode, const std::string& errorMsg) {
    Message resp;
    resp.command = "ERROR";
    resp.params["error_code"] = errorCode;
    resp.params["error_msg"] = errorMsg;
    return resp;
}

// Helper: Validate username and password format
bool UserService::validateCredentials(const std::string& username, const std::string& password, std::string& errMsg) {
    if (username.size() < 4 || username.size() > 20) {
        errMsg = "InvalidUsername";
        return false;
    }
    if (password.size() < 6 || password.size() > 32) {
        errMsg = "InvalidPassword";
        return false;
    }
    std::regex allowed("^[A-Za-z0-9_.-]+$");
    if (!std::regex_match(username, allowed)) {
        errMsg = "InvalidUsernameChars";
        return false;
    }
    return true;
}

Message UserService::login(const Message &msg) {
    if (msg.params.count("username") == 0 || msg.params.count("password") == 0) {
        return createErrorResponse("MISSING_PARAMS", "username and password are required");
    }
    std::string username = msg.params.at("username");
    std::string password = msg.params.at("password");

    std::string validationErr;
    if (!validateCredentials(username, password, validationErr)) {
        return createErrorResponse(validationErr, "Invalid username or password format");
    }

    auto ou = UserDAO::findByUsername(username);
    if (!ou) {
        return createErrorResponse("USER_NOT_FOUND", "Username does not exist");
    }
    User u = *ou;

    if (UserDAO::isUserLocked(u.id)) {
        return createErrorResponse("ACCOUNT_LOCKED", "Account is temporarily locked.");
    }

    std::string passwordHash = Crypto::sha256(password);
    if (u.password != passwordHash) {
        UserDAO::incrementFailedLoginAttempts(u.id);
        int failedAttempts = UserDAO::getFailedLoginAttempts(u.id);
        
        if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
            UserDAO::lockUser(u.id, LOCK_DURATION_SECONDS);
            return createErrorResponse("ACCOUNT_LOCKED", "Account locked due to failed attempts.");
        }

        int remainingAttempts = MAX_FAILED_ATTEMPTS - failedAttempts;
        return createErrorResponse("WRONG_PASSWORD", "Incorrect password (" + std::to_string(remainingAttempts) + " attempts left)");
    }

    if (UserDAO::hasActiveSession(u.id)) {
        return createErrorResponse("ALREADY_LOGGED_IN", "User is already logged in");
    }

    std::string token = Crypto::randomHex(32);
    int sid = UserDAO::createSession(u.id, token);
    if (sid == 0) {
        return createErrorResponse("SESSION_CREATE_FAILED", "Failed to create session");
    }

    UserDAO::resetFailedLoginAttempts(u.id);

    Message resp;
    resp.command = "LOGIN_OK";
    resp.params["user_id"] = std::to_string(u.id);
    resp.params["token"] = token;

    // 1. Gửi tên đăng nhập
    resp.params["username"] = u.username;

    // 2. Gửi điểm số
    resp.params["points"] = std::to_string(u.total_points);

    // 3. Gửi tên Rank (Lấy từ DB thông qua UserDAO)
    std::string rankName = UserDAO::getRankName(u.total_points);
    resp.params["rank_name"] = rankName;
    return resp;
}

Message UserService::logout(const Message &msg) {
    Message resp;
    resp.command = "LOGOUT_OK";
    std::string username = "";
    if (msg.params.count("user_id")) {
        username = msg.params.at("user_id");
    }

 
    

    // 1. Ưu tiên logout bằng User ID (Do ClientHandler inject vào)
    if (msg.params.count("user_id") > 0) {
        int userId = std::stoi(msg.params.at("user_id"));
        if (UserDAO::removeSession(userId)) {
            resp.params["user_id"] = std::to_string(userId);
            resp.params["cleanup_user_map"] = "true";
            return resp;
        }
    }

    // 2. Logout bằng Token (Fallback)
    if (msg.params.count("token") > 0) {
        std::string token = msg.params.at("token");
        int uid = UserDAO::getUserIdByToken(token);
        if (UserDAO::removeSessionByToken(token)) {
            resp.params["token"] = token;
            if (uid > 0) {
                resp.params["user_id"] = std::to_string(uid);
                resp.params["cleanup_user_map"] = "true";
            }
            return resp;
        }
    }

    return createErrorResponse("LOGOUT_FAILED", "Failed to logout");
}

Message UserService::registerUser(const Message &msg) {
    if (msg.params.count("username") == 0 || msg.params.count("password") == 0) {
        return createErrorResponse("MISSING_PARAMS", "username and password are required");
    }

    std::string username = msg.params.at("username");
    std::string password = msg.params.at("password");

    std::string validationErr;
    if (!validateCredentials(username, password, validationErr)) {
        return createErrorResponse(validationErr, "Invalid username or password format");
    }

    if (UserDAO::findByUsername(username)) {
        return createErrorResponse("USERNAME_TAKEN", "Username is already registered");
    }

    std::string hash = Crypto::sha256(password);
    int uid = UserDAO::createUser(username, hash);
    if (uid == 0) {
        return createErrorResponse("CREATE_USER_FAILED", "Failed to create user account");
    }

    Message resp;
    resp.command = "REGISTER_OK";
    resp.params["user_id"] = std::to_string(uid);
    return resp;
}

Message UserService::resetPassword(const Message &msg) {
    if (msg.params.count("old_password") == 0 || msg.params.count("new_password") == 0) {
        return createErrorResponse("MISSING_PARAMS", "Missing passwords");
    }

    int userId = 0;
    if (msg.params.count("user_id") > 0) {
        userId = std::stoi(msg.params.at("user_id"));
    } else if (msg.params.count("token") > 0) {
        userId = UserDAO::getUserIdByToken(msg.params.at("token"));
    }

    if (userId == 0) return createErrorResponse("AUTH_FAILED", "User not identified");

    auto ou = UserDAO::findById(userId);
    if (!ou) return createErrorResponse("USER_NOT_FOUND", "User not found");
    
    std::string oldHash = Crypto::sha256(msg.params.at("old_password"));
    if (ou->password != oldHash) return createErrorResponse("WRONG_OLD_PASSWORD", "Old password incorrect");

    std::string newPass = msg.params.at("new_password");
    std::string dummyErr;
    if (!validateCredentials("dummy", newPass, dummyErr)) {
        return createErrorResponse("INVALID_NEW_PASSWORD", "New password invalid");
    }

    if (!UserDAO::updatePassword(userId, Crypto::sha256(newPass))) {
        return createErrorResponse("UPDATE_FAILED", "DB Error");
    }

    Message resp;
    resp.command = "RESET_PASSWORD_OK";
    resp.params["user_id"] = std::to_string(userId);
    return resp;
}

// HÀM TÌM KIẾM USER (ĐỂ DISPATCHER GỌI)
Message UserService::searchUsers(const Message& msg) {
    Message resp;
    resp.command = "SEARCH_RES";

    std::string keyword = "";
    if (msg.params.count("keyword")) {
        keyword = msg.params.at("keyword");
    }
    // Gọi DAO lấy danh sách
    std::vector<UserDAO::UserSearchInfo> results = UserDAO::searchUsers(keyword);
    std::string payload = "";
    for (const auto& u : results) {
        if (!payload.empty()) payload += "|"; 
        payload += u.username + "," + u.status;
    }

    // ClientHandler sẽ lấy giá trị này để build packet
    resp.params["users"] = payload; 
    return resp;
}

std::string UserService::getUsername(int userId) {
    auto userOpt = UserDAO::findById(userId);
    if (userOpt) return userOpt->username;
    return "Unknown";
}
Message UserService::getLeaderboard(const Message& msg) {
    Message resp;
    resp.command = "LEADERBOARD_RES";
    auto topList = UserDAO::getLeaderboard(10);
    std::string payload = "";
    for (const auto& item : topList) {
        if (!payload.empty()) payload += "|";
        payload += item.username + "," + std::to_string(item.points) + "," + item.rankName;
    }

    resp.params["data"] = payload;
    return resp;
}
