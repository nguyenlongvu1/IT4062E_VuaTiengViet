#include "FriendService.h"
#include "../database/FriendDAO.h"
#include "../database/UserDAO.h"
#include "../core/Server.h"
#include "../core/MessageParser.h" 
#include <iostream>
#include <sstream>
#include <vector>


//GỬI LỜI MỜI KẾT BẠN
Message FriendService::sendFriendRequest(const Message& msg) {
    Message resp;
    resp.command = "ADD_FRIEND_RES"; 

    // Validate Input
    if (!msg.params.count("user_id") || !msg.params.count("target_username")) {
        resp.params["status"] = "fail";
        resp.params["msg"] = "Missing params";
        return resp;
    }

    int myId = std::stoi(msg.params.at("user_id"));
    std::string targetUsername = msg.params.at("target_username");
    // 1. Tìm user mục tiêu
    auto targetUser = UserDAO::findByUsername(targetUsername);
    if (!targetUser) {
        resp.params["status"] = "fail";
        resp.params["msg"] = "User not found";
        return resp;
    }

    int targetId = targetUser->id;
    if (targetId == myId) {
        resp.params["status"] = "fail";
        resp.params["msg"] = "Cannot add yourself";
        return resp;
    }

    // 2. Gửi yêu cầu vào DB
    bool success = FriendDAO::sendRequest(myId, targetId);

    if (success) {
        // --- PHẢN HỒI CHO NGƯỜI GỬI ---
        resp.params["status"] = "success";
        resp.params["msg"] = "Request sent to " + targetUsername;

        // Lấy tên người gửi để báo cho người nhận biết ai đang mời
        auto myUser = UserDAO::findById(myId);
        std::string myName = myUser ? myUser->username : "Unknown";

        // ClientHandler sẽ đọc 2 tham số này để gửi tin nhắn realtime
        resp.params["notify_id"] = std::to_string(targetId);
        resp.params["notify_msg"] = "COMMAND: NOTIFY_FRIEND_REQ\n\nsender_username=" + myName;
    } else {
        resp.params["status"] = "fail";
        resp.params["msg"] = "Request already exists or already friends";
    }

    return resp;
}

// 2. CHẤP NHẬN KẾT BẠN
Message FriendService::acceptFriendRequest(const Message& msg) {
    Message resp;
    resp.command = "ACCEPT_FRIEND_RES";

    if (!msg.params.count("user_id") || !msg.params.count("target_username")) {
        return resp; 
    }

    int myId = std::stoi(msg.params.at("user_id"));
    std::string targetUsername = msg.params.at("target_username");

    auto targetUser = UserDAO::findByUsername(targetUsername);
    if (!targetUser) {
        resp.params["status"] = "fail";
        resp.params["msg"] = "Target user not found";
        return resp;
    }
    int targetId = targetUser->id;

    bool success = FriendDAO::acceptRequest(targetId, myId);

    if (success) {
        // --- PHẢN HỒI CHO MÌNH ---
        resp.params["status"] = "success";
        resp.params["target"] = targetUsername;

        // --- THÔNG BÁO CHO NGƯỜI KIA ---
        auto myUser = UserDAO::findById(myId);
        std::string myName = myUser ? myUser->username : "Unknown";

        resp.params["notify_id"] = std::to_string(targetId);
        resp.params["notify_msg"] = "COMMAND: NOTIFY_FRIEND_ACCEPTED\n\nfriend_username=" + myName;
    } else {
        resp.params["status"] = "fail";
        resp.params["msg"] = "Database error";
    }

    return resp;
}

// 3. TỪ CHỐI KẾT BẠN (Tùy chọn)
Message FriendService::rejectFriendRequest(const Message& msg) {
    Message resp;
    resp.command = "REJECT_FRIEND_RES"; // Tùy Client có xử lý không

    if (!msg.params.count("user_id") || !msg.params.count("target_username")) return resp;
    
    int myId = std::stoi(msg.params.at("user_id"));
    std::string targetUsername = msg.params.at("target_username");
    
    auto targetUser = UserDAO::findByUsername(targetUsername);
    if(targetUser) {
        FriendDAO::rejectRequest(targetUser->id, myId);
        resp.params["status"] = "success";
    }
    return resp;
}

// 4. LẤY DANH SÁCH BẠN BÈ
Message FriendService::listFriends(const Message& msg) {
    Message resp;
    resp.command = "FRIEND_LIST_RES";

    if (!msg.params.count("user_id")) return resp;
    int myId = std::stoi(msg.params.at("user_id"));

    std::vector<UserDAO::UserSearchInfo> friends = FriendDAO::getFriends(myId);

    std::string friends_str;
    Server* srv = Server::getInstance(); 

    for (const auto& f : friends) {
        std::string status = "Offline";
        auto userObj = UserDAO::findByUsername(f.username);
        
        if (userObj && srv) {
            if (srv->isUserOnline(userObj->id)) {
                status = "Online";
            }
        }

        if (!friends_str.empty()) friends_str += "|"; 
        friends_str += f.username + "," + status;
    }

    resp.params["friends"] = friends_str;    
    return resp;
}

// 5. LẤY DANH SÁCH LỜI MỜI ĐANG CHỜ
Message FriendService::listPendingRequests(const Message& msg) {
    Message resp;
    resp.command = "GET_PENDING_RES";

    if (!msg.params.count("user_id")) return resp;
    int myId = std::stoi(msg.params.at("user_id"));

    std::vector<int> pendingIds = FriendDAO::getPendingRequests(myId);
    
    std::string listStr = "";
    for (int uid : pendingIds) {
        auto u = UserDAO::findById(uid);
        if (u) {
            if (!listStr.empty()) listStr += ",";
            listStr += u->username;
        }
    }

    resp.params["request_list"] = listStr;
    return resp;
}