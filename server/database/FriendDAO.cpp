#include "FriendDAO.h"
#include "UserDAO.h"
#include "DB.h"
#include "UserDAO.h"
#include <iostream>

bool FriendDAO::sendRequest(int from_user_id, int to_user_id) {
    // Check if request already exists
    if (requestExists(from_user_id, to_user_id)) {
        return false;
    }
    
    // Insert friend request (pending status)
    std::string query = "INSERT INTO Friends (user_id, friend_user_id, status) VALUES (" +
                        std::to_string(from_user_id) + ", " + 
                        std::to_string(to_user_id) + ", 'pending');";
    return DB::exec(query);
}

bool FriendDAO::acceptRequest(int from_user_id, int to_user_id) {
    // Update status to accepted for the original request
    std::string query = std::string("UPDATE Friends SET status = 'accepted' WHERE ") +
                        "user_id = " + std::to_string(from_user_id) + " AND friend_user_id = " + std::to_string(to_user_id) + ";";
    
    if (!DB::exec(query)) {
        return false;
    }
    
    // Insert reverse relationship so both see each other as friends (bidirectional)
    std::string reverse_query = std::string("INSERT OR IGNORE INTO Friends (user_id, friend_user_id, status) VALUES (") +
                                std::to_string(to_user_id) + ", " + std::to_string(from_user_id) + ", 'accepted');";
    return DB::exec(reverse_query);
}

bool FriendDAO::rejectRequest(int from_user_id, int to_user_id) {
    // Delete friend request
    std::string query = std::string("DELETE FROM Friends WHERE ") +
                        "user_id = " + std::to_string(from_user_id) + " AND friend_user_id = " + std::to_string(to_user_id) + ";";
    return DB::exec(query);
}

std::vector<int> FriendDAO::getAcceptedFriends(int user_id) {
    std::vector<int> friends;
    // Return accepted friends in both directions so friendship is visible to both users
    std::string query = std::string("SELECT friend_user_id FROM Friends WHERE status = 'accepted' AND user_id = ") +
                        std::to_string(user_id) + " UNION SELECT user_id FROM Friends WHERE status = 'accepted' AND friend_user_id = " + std::to_string(user_id) + ";";
    
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB::getHandle(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int friend_id = sqlite3_column_int(stmt, 0);
            friends.push_back(friend_id);
        }
    }
    sqlite3_finalize(stmt);
    return friends;
}

std::vector<int> FriendDAO::getPendingRequests(int user_id) {
    std::vector<int> requests;
    // Pending requests where user_id is the receiver
    std::string query = "SELECT user_id FROM Friends WHERE status = 'pending' AND friend_user_id = " + 
                        std::to_string(user_id) + ";";
    
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB::getHandle(), query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int uid = sqlite3_column_int(stmt, 0);
            requests.push_back(uid);
        }
    }
    sqlite3_finalize(stmt);
    return requests;
}

bool FriendDAO::areFriends(int user_id_1, int user_id_2) {
    std::string query = std::string("SELECT COUNT(*) FROM Friends WHERE status = 'accepted' AND ") +
                        "user_id = " + std::to_string(user_id_1) + " AND friend_user_id = " + std::to_string(user_id_2) + ";";
    int count = DB::queryInt(query);
    return count > 0;
}

bool FriendDAO::requestExists(int from_user_id, int to_user_id) {
    std::string query = std::string("SELECT COUNT(*) FROM Friends WHERE ") +
                        "(user_id = " + std::to_string(from_user_id) + " AND friend_user_id = " + std::to_string(to_user_id) + ") OR " +
                        "(user_id = " + std::to_string(to_user_id) + " AND friend_user_id = " + std::to_string(from_user_id) + ");";
    int count = DB::queryInt(query);
    return count > 0;
}
// FriendDAO.cpp

std::vector<UserDAO::UserSearchInfo> FriendDAO::getFriends(int userId) {
    std::vector<UserDAO::UserSearchInfo> friends;
    sqlite3* db = DB::getHandle();
    if (!db) return friends;

std::string sql = 
    "SELECT DISTINCT u.username, CASE WHEN s.status IS NOT NULL THEN 'Online' ELSE 'Offline' END as status "
    "FROM Friends f "
    "JOIN Users u ON (f.user_id = u.user_id OR f.friend_user_id = u.user_id) "
    "LEFT JOIN Sessions s ON u.user_id = s.user_id "
    "WHERE (f.user_id = " + std::to_string(userId) + " OR f.friend_user_id = " + std::to_string(userId) + ") "
    "AND f.status = 'accepted' AND u.user_id != " + std::to_string(userId) + ";";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            UserDAO::UserSearchInfo info;
            info.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            info.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            friends.push_back(info);
        }
        sqlite3_finalize(stmt);
    } else {
        // In lỗi chi tiết để debug nếu vẫn hỏng
        std::cerr << "[DB ERROR] getFriends failed: " << sqlite3_errmsg(db) << std::endl;
    }

    return friends;
}
std::vector<std::string> FriendDAO::getFriendList(const std::string& username) {
    std::vector<std::string> friendNames;
    sqlite3* db = DB::getHandle();
    
    // 1. Lấy ID của user hiện tại
    std::string idQuery = "SELECT user_id FROM Users WHERE username = '" + username + "'";
    int userId = DB::queryInt(idQuery);
    if (userId <= 0) return friendNames;

    // 2. Lấy danh sách tên bạn bè (status = 'accepted')
    std::string sql = 
        "SELECT DISTINCT u.username "
        "FROM Friends f "
        "JOIN Users u ON (f.user_id = u.user_id OR f.friend_user_id = u.user_id) "
        "WHERE (f.user_id = " + std::to_string(userId) + " OR f.friend_user_id = " + std::to_string(userId) + ") "
        "AND f.status = 'accepted' AND u.user_id != " + std::to_string(userId) + ";";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (name) {
                friendNames.push_back(std::string(name));
            }
        }
        sqlite3_finalize(stmt);
    }
    return friendNames;
}