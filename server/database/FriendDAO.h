#pragma once
#include "../models/Friendship.h"
#include <vector>
#include <optional>

class FriendDAO {
public:
    // Send friend request
    static bool sendRequest(int from_user_id, int to_user_id);
    
    // Accept friend request
    static bool acceptRequest(int from_user_id, int to_user_id);
    
    // Reject friend request
    static bool rejectRequest(int from_user_id, int to_user_id);
    
    // Get all accepted friends for a user
    static std::vector<int> getAcceptedFriends(int user_id);
    
    // Get pending friend requests (incoming)
    static std::vector<int> getPendingRequests(int user_id);
    
    // Check if two users are friends
    static bool areFriends(int user_id_1, int user_id_2);
    
    // Check if friend request already exists
    static bool requestExists(int from_user_id, int to_user_id);
};
