#include "MatchDAO.h"
#include "DB.h"
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>

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
bool MatchDAO::saveMove(int matchId, int roundId, int questionId, int userId, 
                        const std::string& answer, bool isCorrect, int points) {
    sqlite3* db = DB::getHandle();
    const char* sql = "INSERT INTO Match_Log (match_id, round_id, question_id, user_id, user_answer, is_correct, points_earned) VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, matchId);
    sqlite3_bind_int(stmt, 2, roundId);
    sqlite3_bind_int(stmt, 3, questionId);
    sqlite3_bind_int(stmt, 4, userId);
    sqlite3_bind_text(stmt, 5, answer.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, isCorrect ? 1 : 0);
    sqlite3_bind_int(stmt, 7, points);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<MatchLogItem> MatchDAO::getHistoryByUser(int userId) {
    std::vector<MatchLogItem> history;
    sqlite3* db = DB::getHandle();
    
    // Lấy match_id và match_score (Biến động điểm) từ bảng MatchPlayers
    // Sắp xếp match_id DESC để trận mới nhất luôn nằm ở đầu danh sách
    const char* sql = R"(
        SELECT match_id, match_score 
        FROM MatchPlayers 
        WHERE user_id = ? 
        ORDER BY match_id DESC 
        LIMIT 50;
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MatchLogItem item;
            item.matchId = sqlite3_column_int(stmt, 0);
            item.points = sqlite3_column_int(stmt, 1); // Đây chính là biến động điểm (match_score)
            
            // Các trường này để trống hoặc điền mặc định vì bảng MatchPlayers không có nội dung câu hỏi
            item.roundId = 1;
            item.questionText = "Trận đấu #" + std::to_string(item.matchId);
            item.userAnswer = "";
            item.timestamp = ""; 

            history.push_back(item);
        }
    }
    sqlite3_finalize(stmt);
    return history;
}
void MatchDAO::updateMatchScore(int matchId, int userId, int scoreChange, int rankPos) {
    sqlite3* db = DB::getHandle();
    std::string sql = "UPDATE MatchPlayers SET match_score = ?, rank_position = ? WHERE match_id = ? AND user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, scoreChange);
        sqlite3_bind_int(stmt, 2, rankPos);
        sqlite3_bind_int(stmt, 3, matchId);
        sqlite3_bind_int(stmt, 4, userId);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}
std::vector<MatchLogItem> MatchDAO::getMatchDetails(int matchId, int userId) {
    std::vector<MatchLogItem> details;
    sqlite3* db = DB::getHandle();
    if (!db) return details;

    sqlite3_stmt* stmt;
    // Chỉ lấy: Round, Câu trả lời, Trạng thái (is_correct) và Điểm
    std::string sql = "SELECT round_id, user_answer, is_correct, points_earned "
                      "FROM Match_Log "
                      "WHERE match_id = ? AND user_id = ? "
                      "ORDER BY log_id ASC";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, matchId);
        sqlite3_bind_int(stmt, 2, userId);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MatchLogItem item;
            item.roundId = sqlite3_column_int(stmt, 0);
            const char* ans = (const char*)sqlite3_column_text(stmt, 1);
            item.userAnswer = ans ? ans : "[Bỏ trống]";
            item.isCorrect = sqlite3_column_int(stmt, 2); // Lấy 0 hoặc 1
            item.points = sqlite3_column_int(stmt, 3);
            details.push_back(item);
        }
    }
    sqlite3_finalize(stmt);
    return details;
}
bool MatchDAO::deleteHistory(int matchId, int userId) {
    sqlite3* db = DB::getHandle();
    // Xóa trong MatchPlayers để mất khỏi danh sách lịch sử
    const char* sql = "DELETE FROM MatchPlayers WHERE match_id = ? AND user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, matchId);
    sqlite3_bind_int(stmt, 2, userId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}