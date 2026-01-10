#pragma once
#include <string>

/**
 * Message Type Identifiers (Command Codes)
 * Text-based command strings for the application protocol
 */

// ============================================================================
// AUTHENTICATION SERVICE (Commands 100-105)
// ============================================================================
namespace AuthCommands {
    const std::string REGISTER_REQ      = "REGISTER";          // ID: 100
    const std::string REGISTER_RESP     = "REGISTER_OK";       // ID: 101
    const std::string LOGIN_REQ         = "LOGIN";             // ID: 102
    const std::string LOGIN_RESP        = "LOGIN_OK";          // ID: 103
    const std::string LOGOUT_REQ        = "LOGOUT";            // ID: 104
    const std::string LOGOUT_RESP       = "LOGOUT_OK";         // ID: 105
    const std::string CHANGE_PASS       = "CHANGE_PASS";       // ID: 106
    const std::string CHANGE_PASS_OK    = "RESET_PASSWORD_OK"; // ID: 107
}

// ============================================================================
// FRIEND SERVICE (Commands 200-210)
// ============================================================================
namespace FriendCommands {
    const std::string SEARCH_REQ            = "SEARCH_REQ";             // ID: 200
    const std::string SEARCH_RES            = "SEARCH_RES";             // ID: 201
    const std::string ADD_FRIEND_REQ        = "ADD_FRIEND_REQ";         // ID: 202
    const std::string ACCEPT_FRIEND_REQ     = "ACCEPT_FRIEND_REQ";      // ID: 203
    const std::string ACCEPT_FRIEND_RES     = "ACCEPT_FRIEND_RES";      // ID: 204
    const std::string GET_PENDING_REQ       = "GET_PENDING_REQ";        // ID: 205
    const std::string GET_PENDING_RES       = "GET_PENDING_RES";        // ID: 206
    const std::string GET_FRIEND_LIST       = "GET_FRIEND_LIST";        // ID: 207
    const std::string FRIEND_LIST_RES       = "FRIEND_LIST_RES";        // ID: 208
    const std::string NOTIFY_FRIEND_REQ     = "NOTIFY_FRIEND_REQ";      // ID: 209
    const std::string NOTIFY_FRIEND_ACCEPTED = "NOTIFY_FRIEND_ACCEPTED";// ID: 210
}

// ============================================================================
// ROOM SERVICE (Commands 300-310)
// ============================================================================
namespace RoomCommands {
    const std::string CREATE_ROOM           = "CREATE_ROOM";            // ID: 300
    const std::string ROOM_CREATED          = "ROOM_CREATED";           // ID: 301
    const std::string JOIN_ROOM_REQ         = "JOIN_ROOM_REQ";          // ID: 302
    const std::string JOIN_ROOM_RES         = "JOIN_ROOM_RES";          // ID: 303
    const std::string LEAVE_ROOM_REQ        = "LEAVE_ROOM_REQ";         // ID: 304
    const std::string GET_ROOM_INFO         = "GET_ROOM_INFO";          // ID: 305
    const std::string ROOM_INFO_RES         = "ROOM_INFO_RES";          // ID: 306
    const std::string PLAYER_JOINED_NOTIFY  = "PLAYER_JOINED_NOTIFY";   // ID: 307
    const std::string PLAYER_LEFT_NOTIFY    = "PLAYER_LEFT_NOTIFY";     // ID: 308
}

// ============================================================================
// MATCHMAKING & GAME SERVICE (Commands 400-420)
// ============================================================================
namespace GameCommands {
    const std::string FIND_MATCH            = "FIND_MATCH";             // ID: 400
    const std::string CANCEL_MATCH_REQ      = "CANCEL_MATCH_REQ";       // ID: 401
    const std::string MATCH_FOUND_NOTIFY    = "MATCH_FOUND_NOTIFY";     // ID: 402
    const std::string CREATE_MATCH          = "CREATE_MATCH";           // ID: 403
    const std::string START_MATCH           = "START_MATCH";            // ID: 404
    const std::string MATCH_CREATED         = "MATCH_CREATED";          // ID: 405
    const std::string GAME_QUESTION         = "GAME_QUESTION";          // ID: 406
    const std::string ROUND1_ANSWER         = "ROUND1_ANSWER";          // ID: 407
    const std::string ANSWER_RESULT         = "ANSWER_RESULT";          // ID: 408
    const std::string GAME_OVER             = "GAME_OVER";              // ID: 409
    const std::string ELIMINATED            = "ELIMINATED";             // ID: 410
}

// ============================================================================
// LEADERBOARD & STATS SERVICE (Commands 500-510)
// ============================================================================
namespace LeaderboardCommands {
    const std::string GET_LEADERBOARD       = "GET_LEADERBOARD";        // ID: 500
    const std::string LEADERBOARD_RES       = "LEADERBOARD_RES";        // ID: 501
    const std::string GET_HISTORY           = "GET_HISTORY";            // ID: 502
    const std::string HISTORY_DATA          = "HISTORY_DATA";           // ID: 503
    const std::string MATCH_LOG_DATA        = "MATCH_LOG_DATA";         // ID: 504
}

// ============================================================================
// COMMON RESPONSES
// ============================================================================
namespace CommonResponses {
    const std::string ERROR                 = "ERROR";
    const std::string SUCCESS               = "SUCCESS";
    const std::string UPDATE_STATUS_NOTIFY  = "UPDATE_STATUS_NOTIFY";
}
