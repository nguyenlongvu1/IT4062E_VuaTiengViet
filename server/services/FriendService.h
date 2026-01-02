#pragma once
#include "../core/MessageParser.h"
#include <vector>

class FriendService {
public:
    // Send friend request
    static Message sendFriendRequest(const Message& msg);
    
    // Accept friend request
    static Message acceptFriendRequest(const Message& msg);
    
    // Reject friend request
    static Message rejectFriendRequest(const Message& msg);
    
    // Get list of accepted friends
    static Message listFriends(const Message& msg);
    
    // Get list of pending friend requests
    static Message listPendingRequests(const Message& msg);
};

