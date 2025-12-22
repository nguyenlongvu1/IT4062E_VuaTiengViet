#include "UserService.h"
#include "../database/UserDAO.h"
#include "../database/DB.h"
#include <iostream>
#include "../utils/Crypto.h"
#include <regex>

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
        std::cout << "[VALIDATE DEBUG] Username length invalid: " << username.size() << "\n";
        errMsg = "InvalidUsername";
        return false;
    }
    if (password.size() < 6 || password.size() > 32) {
        std::cout << "[VALIDATE DEBUG] Password length invalid: " << password.size() << "\n";
        errMsg = "InvalidPassword";
        return false;
    }
    // Username: alphanumeric, underscore, dot, dash
    std::regex allowed("^[A-Za-z0-9_.-]+$");
    if (!std::regex_match(username, allowed)) {
        std::cout << "[VALIDATE DEBUG] Username regex failed for: '" << username << "'\n";
        errMsg = "InvalidUsernameChars";
        return false;
    }
    std::cout << "[VALIDATE DEBUG] All checks passed for user='" << username << "'\n";
    return true;
}

Message UserService::login(const Message &msg) {
    // Validate input existence
    if (msg.params.count("username") == 0 || msg.params.count("password") == 0) {
        return createErrorResponse("MISSING_PARAMS", "username and password are required");
    }
    std::string username = msg.params.at("username");
    std::string password = msg.params.at("password");

    // Validate credentials format
    std::string validationErr;
    if (!validateCredentials(username, password, validationErr)) {
        return createErrorResponse(validationErr, "Invalid username or password format");
    }

    // Check if user exists
    auto ou = UserDAO::findByUsername(username);
    if (!ou) {
        return createErrorResponse("USER_NOT_FOUND", "Username does not exist");
    }
    User u = *ou;

    // Check if user account is locked (rate limiting)
    if (UserDAO::isUserLocked(u.id)) {
        return createErrorResponse("ACCOUNT_LOCKED", "Account is temporarily locked due to multiple failed login attempts. Try again later.");
    }

    // Verify password
    std::string passwordHash = Crypto::sha256(password);
    if (u.password != passwordHash) {
        // Increment failed login attempts
        UserDAO::incrementFailedLoginAttempts(u.id);
        
        // Get current failed attempts count
        int failedAttempts = UserDAO::getFailedLoginAttempts(u.id);
        
        // Lock account after MAX_FAILED_ATTEMPTS
        if (failedAttempts >= MAX_FAILED_ATTEMPTS) {
            UserDAO::lockUser(u.id, LOCK_DURATION_SECONDS);
            return createErrorResponse("ACCOUNT_LOCKED", "Account locked after 3 failed login attempts. Try again in 15 minutes.");
        }

        // Inform client of remaining attempts
        int remainingAttempts = MAX_FAILED_ATTEMPTS - failedAttempts;
        std::string msg = "Password is incorrect (" + std::to_string(remainingAttempts) + " attempts remaining)";
        return createErrorResponse("WRONG_PASSWORD", msg);
    }

    // Check for duplicate login
    if (UserDAO::hasActiveSession(u.id)) {
        return createErrorResponse("ALREADY_LOGGED_IN", "User is already logged in");
    }

    // Create session token
    std::string token = Crypto::randomHex(32);
    int sid = UserDAO::createSession(u.id, token);
    if (sid == 0) {
        return createErrorResponse("SESSION_CREATE_FAILED", "Failed to create session");
    }

    // Reset failed login attempts on successful login
    UserDAO::resetFailedLoginAttempts(u.id);

    Message resp;
    resp.command = "LOGIN_OK";
    resp.params["user_id"] = std::to_string(u.id);
    resp.params["token"] = token;
    return resp;
}

Message UserService::logout(const Message &msg) {
    Message resp;
    int userId = 0;
    std::string token;

    // Accept either token or user_id
    if (msg.params.count("token") > 0) {
        token = msg.params.at("token");
        // find user id for this token so we can cleanup server user_map
        int uid = UserDAO::getUserIdByToken(token);
        if (!UserDAO::removeSessionByToken(token)) {
            return createErrorResponse("LOGOUT_FAILED", "Failed to logout (token)");
        }
        // Return token and user_id so client can copy and server can cleanup
        resp.command = "LOGOUT_OK";
        resp.params["token"] = token;
        if (uid > 0) {
            resp.params["user_id"] = std::to_string(uid);
            resp.params["cleanup_user_map"] = "true";
        }
        return resp;
    }

    if (msg.params.count("user_id") == 0) {
        return createErrorResponse("MISSING_PARAMS", "user_id or token is required");
    }

    userId = std::stoi(msg.params.at("user_id"));
    if (!UserDAO::removeSession(userId)) {
        return createErrorResponse("LOGOUT_FAILED", "Failed to logout (user_id)");
    }

    // Mark for client cleanup (see ClientHandler::run())
    resp.command = "LOGOUT_OK";
    resp.params["user_id"] = std::to_string(userId);
    resp.params["cleanup_user_map"] = "true";
    return resp;
}

Message UserService::resetPassword(const Message &msg) {
    // SECURITY: Require old_password and new_password
    if (msg.params.count("old_password") == 0 || msg.params.count("new_password") == 0) {
        return createErrorResponse("MISSING_PARAMS", "old_password and new_password are required");
    }

    // Accept either token or user_id to identify user
    int userId = 0;
    if (msg.params.count("token") > 0) {
        std::string token = msg.params.at("token");
        userId = UserDAO::getUserIdByToken(token);
        if (userId == 0) {
            return createErrorResponse("INVALID_TOKEN", "Token is invalid or expired");
        }
    } else if (msg.params.count("user_id") > 0) {
        userId = std::stoi(msg.params.at("user_id"));
    } else {
        return createErrorResponse("MISSING_PARAMS", "token or user_id is required");
    }

    std::string oldPassword = msg.params.at("old_password");
    std::string newPassword = msg.params.at("new_password");

    // Fetch user by ID
    auto ou = UserDAO::findById(userId);
    if (!ou) {
        return createErrorResponse("USER_NOT_FOUND", "User ID does not exist");
    }
    User u = *ou;

    // Verify old password
    std::string oldHash = Crypto::sha256(oldPassword);
    if (u.password != oldHash) {
        return createErrorResponse("WRONG_OLD_PASSWORD", "Old password is incorrect");
    }

    // Validate new password format
    std::string validationErr;
    if (!validateCredentials("dummy", newPassword, validationErr)) {
        return createErrorResponse("INVALID_NEW_PASSWORD", "New password does not meet requirements");
    }

    // Hash new password
    std::string newHash = Crypto::sha256(newPassword);

    if (!UserDAO::updatePassword(userId, newHash)) {
        return createErrorResponse("PASSWORD_UPDATE_FAILED", "Failed to update password");
    }

    Message resp;
    resp.command = "RESET_PASSWORD_OK";
    resp.params["user_id"] = std::to_string(userId);
    return resp;
}

// Register handler
Message UserService::registerUser(const Message &msg) {
    if (msg.params.count("username") == 0 || msg.params.count("password") == 0) {
        return createErrorResponse("MISSING_PARAMS", "username and password are required");
    }

    std::string username = msg.params.at("username");
    std::string password = msg.params.at("password");

    // DEBUG: Log received credentials
    std::cout << "[REGISTER DEBUG] username='" << username << "' (len=" << username.size() << "), password='" << password << "' (len=" << password.size() << ")\n";

    // Validate credentials format
    std::string validationErr;
    if (!validateCredentials(username, password, validationErr)) {
        std::cout << "[REGISTER DEBUG] Validation failed: " << validationErr << "\n";
        return createErrorResponse(validationErr, "Invalid username or password format");
    }

    // Check if username already taken
    if (UserDAO::findByUsername(username)) {
        return createErrorResponse("USERNAME_TAKEN", "Username is already registered");
    }

    // Hash and create user
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
std::string UserService::getUsername(int userId) {
   
    auto userOpt = UserDAO::findById(userId);
    
    if (userOpt) {
        return userOpt->username;
    }
    return "Unknown";
}