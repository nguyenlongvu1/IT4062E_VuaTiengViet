#include "FriendService.h"
#include "../database/FriendDAO.h"
#include "../database/UserDAO.h"
#include "../core/Server.h"
#include <iostream>
#include <sstream>

Message FriendService::sendFriendRequest(const Message& msg) {
    Message resp;
    
    // Validate input
    if (msg.params.count("token") == 0 || msg.params.count("target_username") == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "MISSING_PARAMS";
        resp.params["error_msg"] = "token and target_username are required";
        return resp;
    }
    
    std::string token = msg.params.at("token");
    std::string target_username = msg.params.at("target_username");
    
    // Get user ID from token
    int from_user_id = UserDAO::getUserIdByToken(token);
    if (from_user_id == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "INVALID_TOKEN";
        resp.params["error_msg"] = "Token is invalid or expired";
        return resp;
    }
    
    // Get target user ID from username
    auto target_user = UserDAO::findByUsername(target_username);
    if (!target_user) {
        resp.command = "ERROR";
        resp.params["error_code"] = "USER_NOT_FOUND";
        resp.params["error_msg"] = "Target user does not exist";
        return resp;
    }
    int to_user_id = target_user->id;
    
    // Cannot send friend request to self
    if (from_user_id == to_user_id) {
        resp.command = "ERROR";
        resp.params["error_code"] = "CANNOT_ADD_SELF";
        resp.params["error_msg"] = "Cannot send friend request to yourself";
        return resp;
    }
    
    // Check if already friends or request exists
    if (FriendDAO::requestExists(from_user_id, to_user_id)) {
        resp.command = "ERROR";
        resp.params["error_code"] = "REQUEST_EXISTS";
        resp.params["error_msg"] = "Friend request already exists";
        return resp;
    }
    
    // Send friend request
    if (FriendDAO::sendRequest(from_user_id, to_user_id)) {
        resp.command = "FRIEND_REQUEST_SENT";
        resp.params["target_user_id"] = std::to_string(to_user_id);
        resp.params["target_username"] = target_username;
    } else {
        resp.command = "ERROR";
        resp.params["error_code"] = "REQUEST_FAILED";
        resp.params["error_msg"] = "Failed to send friend request";
    }
    
    return resp;
}

Message FriendService::acceptFriendRequest(const Message& msg) {
    Message resp;
    
    // Validate input
    if (msg.params.count("token") == 0 || msg.params.count("from_username") == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "MISSING_PARAMS";
        resp.params["error_msg"] = "token and from_username are required";
        return resp;
    }
    
    std::string token = msg.params.at("token");
    std::string from_username = msg.params.at("from_username");
    
    // Get user ID from token
    int to_user_id = UserDAO::getUserIdByToken(token);
    if (to_user_id == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "INVALID_TOKEN";
        resp.params["error_msg"] = "Token is invalid or expired";
        return resp;
    }
    
    // Get requester user ID from username
    auto from_user = UserDAO::findByUsername(from_username);
    if (!from_user) {
        resp.command = "ERROR";
        resp.params["error_code"] = "USER_NOT_FOUND";
        resp.params["error_msg"] = "Requester user does not exist";
        return resp;
    }
    int from_user_id = from_user->id;
    
    // Accept friend request
    if (FriendDAO::acceptRequest(from_user_id, to_user_id)) {
        resp.command = "FRIEND_REQUEST_ACCEPTED";
        resp.params["from_user_id"] = std::to_string(from_user_id);
        resp.params["from_username"] = from_username;
    } else {
        resp.command = "ERROR";
        resp.params["error_code"] = "ACCEPT_FAILED";
        resp.params["error_msg"] = "Failed to accept friend request";
    }
    
    return resp;
}

Message FriendService::rejectFriendRequest(const Message& msg) {
    Message resp;
    
    // Validate input
    if (msg.params.count("token") == 0 || msg.params.count("from_username") == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "MISSING_PARAMS";
        resp.params["error_msg"] = "token and from_username are required";
        return resp;
    }
    
    std::string token = msg.params.at("token");
    std::string from_username = msg.params.at("from_username");
    
    // Get user ID from token
    int to_user_id = UserDAO::getUserIdByToken(token);
    if (to_user_id == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "INVALID_TOKEN";
        resp.params["error_msg"] = "Token is invalid or expired";
        return resp;
    }
    
    // Get requester user ID from username
    auto from_user = UserDAO::findByUsername(from_username);
    if (!from_user) {
        resp.command = "ERROR";
        resp.params["error_code"] = "USER_NOT_FOUND";
        resp.params["error_msg"] = "Requester user does not exist";
        return resp;
    }
    int from_user_id = from_user->id;
    
    // Reject friend request
    if (FriendDAO::rejectRequest(from_user_id, to_user_id)) {
        resp.command = "FRIEND_REQUEST_REJECTED";
        resp.params["from_username"] = from_username;
    } else {
        resp.command = "ERROR";
        resp.params["error_code"] = "REJECT_FAILED";
        resp.params["error_msg"] = "Failed to reject friend request";
    }
    
    return resp;
}

Message FriendService::listFriends(const Message& msg) {
    Message resp;
    
    // Validate input
    if (msg.params.count("token") == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "MISSING_PARAMS";
        resp.params["error_msg"] = "token is required";
        return resp;
    }
    
    std::string token = msg.params.at("token");
    
    // Get user ID from token
    int user_id = UserDAO::getUserIdByToken(token);
    if (user_id == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "INVALID_TOKEN";
        resp.params["error_msg"] = "Token is invalid or expired";
        return resp;
    }
    
    // Get list of friends
    std::vector<int> friend_ids = FriendDAO::getAcceptedFriends(user_id);
    
    // Build response
    resp.command = "FRIENDS_LIST";
    if (friend_ids.empty()) {
        resp.params["friend_count"] = "0";
        resp.params["friends"] = "";
    } else {
        resp.params["friend_count"] = std::to_string(friend_ids.size());
        
        // Get friend details (username + online status)
        std::string friends_str;
        for (size_t i = 0; i < friend_ids.size(); i++) {
            auto friend_user = UserDAO::findById(friend_ids[i]);
            if (friend_user) {
                if (i > 0) friends_str += "|";
                // Check if friend is online via Server's user_map
                bool is_online = false;
                Server* srv = Server::getInstance();
                if (srv) {
                    is_online = srv->isUserOnline(friend_ids[i]);
                }
                std::string status = is_online ? "[ONLINE]" : "[OFFLINE]";
                friends_str += friend_user->username + " " + status;
            }
        }
        resp.params["friends"] = friends_str;
    }
    
    return resp;
}

Message FriendService::listPendingRequests(const Message& msg) {
    Message resp;
    
    // Validate input
    if (msg.params.count("token") == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "MISSING_PARAMS";
        resp.params["error_msg"] = "token is required";
        return resp;
    }
    
    std::string token = msg.params.at("token");
    
    // Get user ID from token
    int user_id = UserDAO::getUserIdByToken(token);
    if (user_id == 0) {
        resp.command = "ERROR";
        resp.params["error_code"] = "INVALID_TOKEN";
        resp.params["error_msg"] = "Token is invalid or expired";
        return resp;
    }
    
    // Get list of pending requests
    std::vector<int> request_ids = FriendDAO::getPendingRequests(user_id);
    
    // Build response
    resp.command = "PENDING_REQUESTS";
    if (request_ids.empty()) {
        resp.params["request_count"] = "0";
        resp.params["requests"] = "";
    } else {
        resp.params["request_count"] = std::to_string(request_ids.size());
        
        // Get requester details (username only)
        std::string requests_str;
        for (size_t i = 0; i < request_ids.size(); i++) {
            auto requester = UserDAO::findById(request_ids[i]);
            if (requester) {
                if (i > 0) requests_str += "|";
                requests_str += requester->username;
            }
        }
        resp.params["requests"] = requests_str;
    }
    
    return resp;
}
