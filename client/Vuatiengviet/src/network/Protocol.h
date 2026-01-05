#ifndef PROTOCOL_H
#define PROTOCOL_H

// --- AUTH ---
#define CMD_LOGIN               "LOGIN"
#define CMD_LOGIN_OK            "LOGIN_OK"
#define CMD_LOGIN_FAIL          "ERROR" // Server trả về ERROR chung

#define CMD_REGISTER            "REGISTER"
#define CMD_REG_OK              "REGISTER_OK"
#define CMD_REG_FAIL            "REGISTER_FAIL" // Hoặc ERROR

#define CMD_LOGOUT              "LOGOUT"

#define CMD_CHANGE_PASS         "CHANGE_PASS"
#define CMD_CHANGE_PASS_OK      "RESET_PASSWORD_OK"
#define CMD_CHANGE_PASS_FAIL    "ERROR"

// --- SEARCH ---
#define CMD_SEARCH_REQ          "SEARCH_REQ"
#define CMD_SEARCH_RES          "SEARCH_RES"

// --- FRIEND ---
#define CMD_ADD_FRIEND_REQ          "ADD_FRIEND_REQ"
#define CMD_ACCEPT_FRIEND_REQ       "ACCEPT_FRIEND_REQ"
#define CMD_ACCEPT_FRIEND_RES       "ACCEPT_FRIEND_RES"
#define CMD_GET_PENDING_REQ         "GET_PENDING_REQ"
#define CMD_GET_PENDING_RES         "GET_PENDING_RES"
#define CMD_GET_FRIEND_LIST         "GET_FRIEND_LIST"
#define CMD_FRIEND_LIST_RES         "FRIEND_LIST_RES"
#define CMD_NOTIFY_FRIEND_REQ       "NOTIFY_FRIEND_REQ"
#define CMD_NOTIFY_FRIEND_ACCEPTED  "NOTIFY_FRIEND_ACCEPTED"
#define CMD_UPDATE_STATUS_NOTIFY    "UPDATE_STATUS_NOTIFY"

// --- ROOM ---
#define CMD_CREATE_ROOM             "CREATE_ROOM"
#define CMD_ROOM_CREATED            "ROOM_CREATED"
#define CMD_JOIN_ROOM_REQ           "JOIN_ROOM_REQ"
#define CMD_JOIN_ROOM_RES           "JOIN_ROOM_RES"
#define CMD_LEAVE_ROOM_REQ          "LEAVE_ROOM_REQ"
#define CMD_GET_ROOM_INFO           "GET_ROOM_INFO"
#define CMD_ROOM_INFO_RES           "ROOM_INFO_RES"
#define CMD_PLAYER_JOINED_NOTIFY    "PLAYER_JOINED_NOTIFY"
#define CMD_PLAYER_LEFT_NOTIFY      "PLAYER_LEFT_NOTIFY"

// --- MATCHMAKING & GAME ---
#define CMD_FIND_MATCH              "FIND_MATCH"
#define CMD_CANCEL_MATCH_REQ        "CANCEL_MATCH_REQ"
#define CMD_MATCH_FOUND_NOTIFY      "MATCH_FOUND_NOTIFY"
#define CMD_CREATE_MATCH            "CREATE_MATCH" // Host bắt đầu game từ phòng
#define CMD_START_MATCH             "START_MATCH"  // Server báo game bắt đầu (Matchmaking)
#define CMD_MATCH_CREATED           "MATCH_CREATED" // Server báo game bắt đầu (Room)

// --- LEADERBOARD ---
#define CMD_GET_LEADERBOARD         "GET_LEADERBOARD"
#define CMD_LEADERBOARD_RES         "LEADERBOARD_RES"

#endif // PROTOCOL_H