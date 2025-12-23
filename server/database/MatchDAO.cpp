#include "MatchDAO.h"
#include "DB.h"
#include <sqlite3.h>
#include <iostream>
#include <vector>

int MatchDAO::createMatch(int rankId, const std::vector<int> &players) {
    sqlite3 *db = DB::getHandle();
    if (!db || players.empty()) return 0;

    sqlite3_stmt *stmt = nullptr;
    int roomId = 0;
    int matchId = 0;

    // 1. TẠO PHÒNG TRONG BẢNG 'Rooms'
    const char *sqlRoom = "INSERT INTO Rooms (host_id, status, rank_id) VALUES (?, 'playing', ?);";
    if (sqlite3_prepare_v2(db, sqlRoom, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, players[0]); 
        sqlite3_bind_int(stmt, 2, rankId);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            roomId = (int)sqlite3_last_insert_rowid(db);
        }
    }
    sqlite3_finalize(stmt);
    if (roomId == 0) return 0;

    // 2. TẠO TRẬN ĐẤU TRONG BẢNG 'Match'
    const char *sqlMatch = "INSERT INTO Match (room_id, rank_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sqlMatch, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, roomId);
        sqlite3_bind_int(stmt, 2, rankId);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            matchId = (int)sqlite3_last_insert_rowid(db);
        }
    }
    sqlite3_finalize(stmt);
    if (matchId == 0) return 0;

    // 3. THÊM NGƯỜI CHƠI VÀO 'MatchPlayers'
    const char *sqlPlayers = "INSERT INTO MatchPlayers (match_id, user_id, rank_position, match_score) VALUES (?, ?, ?, 0);";
    for (size_t i = 0; i < players.size(); i++) {
        sqlite3_stmt *stP = nullptr;
        if (sqlite3_prepare_v2(db, sqlPlayers, -1, &stP, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stP, 1, matchId);
            sqlite3_bind_int(stP, 2, players[i]);
            sqlite3_bind_int(stP, 3, (int)i + 1);
            sqlite3_step(stP);
        }
        sqlite3_finalize(stP);
    }

    // 4. CHỌN 30 CÂU HỎI NGẪU NHIÊN VÀ LƯU VÀO 'MatchQuestions'
    const char *sqlSelectQ = "SELECT question_id FROM Questions ORDER BY RANDOM() LIMIT 30;";
    std::vector<int> selectedQs;
    if (sqlite3_prepare_v2(db, sqlSelectQ, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            selectedQs.push_back(sqlite3_column_int(stmt, 0));
        }
    }
    sqlite3_finalize(stmt);

    const char *sqlInsertQ = "INSERT INTO MatchQuestions (match_id, question_id, round, q_order) VALUES (?, ?, ?, ?);";
    for (size_t i = 0; i < selectedQs.size(); ++i) {
        int round = (i / 10) + 1;
        int order = (int)(i % 10) + 1;
        sqlite3_stmt *stQ = nullptr;
        if (sqlite3_prepare_v2(db, sqlInsertQ, -1, &stQ, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stQ, 1, matchId);
            sqlite3_bind_int(stQ, 2, selectedQs[i]);
            sqlite3_bind_int(stQ, 3, round);
            sqlite3_bind_int(stQ, 4, order);
            sqlite3_step(stQ);
        }
        sqlite3_finalize(stQ);
    }

    return matchId;
}


// LẤY DANH SÁCH NGƯỜI CHƠI TRONG TRẬN
std::vector<int> MatchDAO::getPlayersForMatch(int matchId) {
    std::vector<int> players;
    sqlite3 *db = DB::getHandle();
    if (!db) return players;

    const char *sql = "SELECT user_id FROM MatchPlayers WHERE match_id = ? ORDER BY rank_position ASC;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, matchId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            players.push_back(sqlite3_column_int(stmt, 0));
        }
    }
    sqlite3_finalize(stmt);
    return players;
}

// LẤY CÂU HỎI CHO TỪNG VÒNG (ROUND)
std::vector<int> MatchDAO::getQuestionsForMatch(int matchId, int round) {
    std::vector<int> qs;
    sqlite3 *db = DB::getHandle();
    if (!db) return qs;

    const char *sql = "SELECT question_id FROM MatchQuestions WHERE match_id = ? AND round = ? ORDER BY q_order ASC;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, matchId);
        sqlite3_bind_int(stmt, 2, round);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            qs.push_back(sqlite3_column_int(stmt, 0));
        }
    }
    sqlite3_finalize(stmt);
    return qs;
}