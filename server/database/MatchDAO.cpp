#include "MatchDAO.h"
#include "DB.h"
#include <sqlite3.h>
#include <iostream>

int MatchDAO::createMatch(int roomId, const std::vector<int> &players) {
    sqlite3 *db = DB::getHandle();
    if (!db) return 0;
    const char *sql = "INSERT INTO Match (room_id) VALUES (?);";
    sqlite3_stmt *stmt = nullptr;
    int matchId = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, roomId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert match failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return 0;
        }
        sqlite3_finalize(stmt);
        matchId = (int)sqlite3_last_insert_rowid(db);
        if (matchId == 0) return 0;

        // insert players
        const char *sql2 = "INSERT INTO MatchPlayers (match_id, user_id, rank_position, match_score) VALUES (?, ?, ?, ?);";
        for (size_t i = 0; i < players.size(); i++) {
            sqlite3_stmt *st2 = nullptr;
            if (sqlite3_prepare_v2(db, sql2, -1, &st2, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(st2, 1, matchId);
                sqlite3_bind_int(st2, 2, players[i]);
                sqlite3_bind_int(st2, 3, (int)i + 1);
                sqlite3_bind_int(st2, 4, 0);
                if (sqlite3_step(st2) != SQLITE_DONE) {
                    std::cerr << "Insert match player failed: " << sqlite3_errmsg(db) << std::endl;
                }
            }
            if (st2) sqlite3_finalize(st2);
        }
    } else {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        if (stmt) sqlite3_finalize(stmt);
        return 0;
    }
    // choose questions for match: 10 per round (30 total) randomly from Questions
    const char *sqlSelect = "SELECT question_id FROM Questions ORDER BY RANDOM() LIMIT 30;";
    sqlite3_stmt *stmtSelect = nullptr;
    std::vector<int> selected;
    if (sqlite3_prepare_v2(db, sqlSelect, -1, &stmtSelect, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmtSelect) == SQLITE_ROW) {
            selected.push_back(sqlite3_column_int(stmtSelect, 0));
        }
    }
    if (stmtSelect) sqlite3_finalize(stmtSelect);

    const char *sqlInsertQ = "INSERT INTO MatchQuestions (match_id, question_id, round, q_order) VALUES (?, ?, ?, ?);";
    for (size_t i = 0; i < selected.size(); ++i) {
        int round = (i / 10) + 1; // 1,2,3
        int order = (int)(i % 10) + 1;
        sqlite3_stmt *s2 = nullptr;
        if (sqlite3_prepare_v2(db, sqlInsertQ, -1, &s2, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(s2, 1, matchId);
            sqlite3_bind_int(s2, 2, selected[i]);
            sqlite3_bind_int(s2, 3, round);
            sqlite3_bind_int(s2, 4, order);
            if (sqlite3_step(s2) != SQLITE_DONE) {
                std::cerr << "Insert match question failed: " << sqlite3_errmsg(db) << std::endl;
            }
        }
        if (s2) sqlite3_finalize(s2);
    }
    return matchId;
}

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
    if (stmt) sqlite3_finalize(stmt);
    return players;
}

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
    if (stmt) sqlite3_finalize(stmt);
    return qs;
}
