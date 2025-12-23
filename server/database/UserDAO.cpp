#include "UserDAO.h"
#include "DB.h"
#include <sqlite3.h>
#include <iostream>
#include <vector>

std::optional<User> UserDAO::findByUsername(const std::string &username) {
    sqlite3 *db = DB::getHandle();
    if (!db) return std::nullopt;
    const char *sql = "SELECT user_id, username, password, total_points FROM Users WHERE username = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            User u;
            u.id = sqlite3_column_int(stmt, 0);
            u.username = (const char *)sqlite3_column_text(stmt, 1);
            u.password = (const char *)sqlite3_column_text(stmt, 2);
            u.total_points = sqlite3_column_int(stmt, 3);
            sqlite3_finalize(stmt);
            return u;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> UserDAO::findById(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return std::nullopt;
    const char *sql = "SELECT user_id, username, password, total_points FROM Users WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            User u;
            u.id = sqlite3_column_int(stmt, 0);
            u.username = (const char *)sqlite3_column_text(stmt, 1);
            u.password = (const char *)sqlite3_column_text(stmt, 2);
            u.total_points = sqlite3_column_int(stmt, 3);
            sqlite3_finalize(stmt);
            return u;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return std::nullopt;
}

// Get failed login attempts count for a user
int UserDAO::getFailedLoginAttempts(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return 0;
    const char *sql = "SELECT failed_login_attempts FROM Users WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    int attempts = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            attempts = sqlite3_column_int(stmt, 0);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return attempts;
}

int UserDAO::createUser(const std::string &username, const std::string &password) {
// Note: password is expected to be SHA-256 hex string
    sqlite3 *db = DB::getHandle();
    if (!db) return 0;
    const char *sql = "INSERT INTO Users (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert user failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return 0;
        }
        sqlite3_finalize(stmt);
        return (int)sqlite3_last_insert_rowid(db);
    } else {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        if (stmt) sqlite3_finalize(stmt);
        return 0;
    }
}

bool UserDAO::updatePassword(int userId, const std::string &newPassword) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "UPDATE Users SET password = ? WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, newPassword.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, userId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

bool UserDAO::removeSession(int userId) {
    // For simplicity we can remove sessions in Sessions table for the user
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "DELETE FROM Sessions WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

bool UserDAO::hasActiveSession(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "SELECT session_id FROM Sessions WHERE user_id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    bool res = false;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) == SQLITE_ROW) res = true;
    }
    if (stmt) sqlite3_finalize(stmt);
    return res;
}

int UserDAO::createSession(int userId, const std::string &token) {
    sqlite3 *db = DB::getHandle();
    if (!db) return 0;
    const char *sql = "INSERT INTO Sessions (user_id, token, status, login_time, last_active) VALUES (?, ?, 'online', CURRENT_TIMESTAMP, CURRENT_TIMESTAMP);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_bind_text(stmt, 2, token.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Create session failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return 0;
        }
    } else {
        if (stmt) sqlite3_finalize(stmt);
        return 0;
    }
    if (stmt) sqlite3_finalize(stmt);
    return DB::lastInsertId();
}

bool UserDAO::removeSessionByToken(const std::string &token) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "DELETE FROM Sessions WHERE token = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

// Return user_id for a given session token, or 0 if not found
int UserDAO::getUserIdByToken(const std::string &token) {
    sqlite3 *db = DB::getHandle();
    if (!db) return 0;
    const char *sql = "SELECT user_id FROM Sessions WHERE token = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    int userId = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            userId = sqlite3_column_int(stmt, 0);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return userId;
}

// Rate limiting: Increment failed login attempts
bool UserDAO::incrementFailedLoginAttempts(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "UPDATE Users SET failed_login_attempts = failed_login_attempts + 1 WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

// Rate limiting: Reset failed login attempts on successful login
bool UserDAO::resetFailedLoginAttempts(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "UPDATE Users SET failed_login_attempts = 0, is_locked = 0, locked_until = NULL WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

// Rate limiting: Lock user account for specified duration (seconds)
bool UserDAO::lockUser(int userId, int lockDurationSeconds) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "UPDATE Users SET is_locked = 1, locked_until = datetime('now', '+' || ? || ' seconds') WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, lockDurationSeconds);
        sqlite3_bind_int(stmt, 2, userId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

// Rate limiting: Unlock user account
bool UserDAO::unlockUser(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "UPDATE Users SET is_locked = 0, locked_until = NULL, failed_login_attempts = 0 WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return true;
}

// Rate limiting: Check if user account is currently locked
bool UserDAO::isUserLocked(int userId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return false;
    const char *sql = "SELECT is_locked, locked_until FROM Users WHERE user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    bool isLocked = false;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int lockedFlag = sqlite3_column_int(stmt, 0);
            const char *lockedUntil = (const char *)sqlite3_column_text(stmt, 1);

            if (lockedFlag == 1) {
                // Check if lock has expired
                if (lockedUntil) {
                    const char *checkSql = "SELECT CASE WHEN datetime(?) > datetime('now') THEN 1 ELSE 0 END;";
                    sqlite3_stmt *checkStmt = nullptr;
                    if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, nullptr) == SQLITE_OK) {
                        sqlite3_bind_text(checkStmt, 1, lockedUntil, -1, SQLITE_TRANSIENT);
                        if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                            isLocked = (sqlite3_column_int(checkStmt, 0) == 1);
                        }
                        sqlite3_finalize(checkStmt);
                    }
                } else {
                    isLocked = true; // Permanent lock
                }

                // Auto-unlock if lock expired
                if (!isLocked && lockedUntil) {
                    unlockUser(userId);
                }
            }
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return isLocked;
}
std::vector<UserDAO::UserSearchInfo> UserDAO::searchUsers(const std::string &keyword) {
    sqlite3 *db = DB::getHandle();
    // Lưu ý: Cũng sửa kiểu dữ liệu của biến results
    std::vector<UserDAO::UserSearchInfo> results; 
    
    if (!db) return results;

    const char *sql = R"(
        SELECT u.username, 
               CASE WHEN s.token IS NOT NULL THEN 'Online' ELSE 'Offline' END as status
        FROM Users u
        LEFT JOIN Sessions s ON u.user_id = s.user_id
        WHERE u.username LIKE ? 
        LIMIT 20;
    )";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::string likePattern = "%" + keyword + "%";
        sqlite3_bind_text(stmt, 1, likePattern.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            UserDAO::UserSearchInfo info;
            info.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            info.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            results.push_back(info);
        }
    } else {
        std::cerr << "Search Error: " << sqlite3_errmsg(db) << std::endl;
    }
    
    if (stmt) sqlite3_finalize(stmt);
    return results;
}
void UserDAO::clearAllSessions() {
    sqlite3 *db = DB::getHandle();
    if (!db) return;
    
    char *errMsg = nullptr;
    const char *sql = "DELETE FROM Sessions;"; // Xóa hết

    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "[DB] Da xoa sach Session cu." << std::endl;
    }
}
// UPDATE total_points
bool UserDAO::addPoints(int userId, int points) {
    sqlite3* db = DB::getHandle();
    
    std::string sql = "UPDATE Users SET total_points = total_points + ? WHERE user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, points);
    sqlite3_bind_int(stmt, 2, userId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    if (success) {
        std::cout << "[UserDAO] Added " << points << " points for User ID " << userId << "\n";
    }
    return success;
}

// LẤY TÊN RANK (Tra cứu bảng Ranks)
std::string UserDAO::getRankName(int points) {
    sqlite3* db = DB::getHandle();
    std::string sql = "SELECT rank_name FROM Ranks WHERE ? >= min_point AND ? <= max_point LIMIT 1;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "Không xác định"; 
    }

    sqlite3_bind_int(stmt, 1, points);
    sqlite3_bind_int(stmt, 2, points);

    std::string rankName = "Mù chữ";
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* text = (const char*)sqlite3_column_text(stmt, 0);
        if (text) {
            rankName = std::string(text);
        }
    }
    
    sqlite3_finalize(stmt);
    return rankName;
}


// LẤY ĐIỂM HIỆN TẠI
int UserDAO::getPoints(int userId) {
    sqlite3* db = DB::getHandle();
    std::string sql = "SELECT total_points FROM Users WHERE user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return 0;

    sqlite3_bind_int(stmt, 1, userId);
    
    int points = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        points = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return points;
}
std::vector<UserDAO::LeaderboardInfo> UserDAO::getLeaderboard(int limit) {
    std::vector<UserDAO::LeaderboardInfo> list;
    sqlite3* db = DB::getHandle();

    std::string sql = "SELECT username, total_points FROM Users ORDER BY total_points DESC LIMIT ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return list; 
    }

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserDAO::LeaderboardInfo info; 
        
        // Cột 0: username
        const unsigned char* nameText = sqlite3_column_text(stmt, 0);
        info.username = nameText ? std::string(reinterpret_cast<const char*>(nameText)) : "";
        
        // Cột 1: total_points
        info.points = sqlite3_column_int(stmt, 1);
        
        // Tính Rank Name
        info.rankName = getRankName(info.points);
        
        list.push_back(info);
    }
    sqlite3_finalize(stmt);
    return list;
}