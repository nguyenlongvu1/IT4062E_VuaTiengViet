#include "DB.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <mutex>

static sqlite3 *g_db = nullptr;
static std::mutex g_db_mutex;

bool DB::open(const std::string &filename) {
    std::lock_guard<std::mutex> lock(g_db_mutex);
    if (g_db) return true;
    int rc = sqlite3_open(filename.c_str(), &g_db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open DB: " << sqlite3_errmsg(g_db) << std::endl;
        sqlite3_close(g_db);
        g_db = nullptr;
        return false;
    }
    sqlite3_exec(g_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(g_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    return true;
}

void DB::close() {
    std::lock_guard<std::mutex> lock(g_db_mutex);
    if (g_db) {
        sqlite3_close(g_db);
        g_db = nullptr;
    }
}

sqlite3 *DB::getHandle() {
    return g_db;
}

bool DB::exec(const std::string &sql) {
    std::lock_guard<std::mutex> lock(g_db_mutex);
    if (!g_db) return false;
    char *err = nullptr;
    int rc = sqlite3_exec(g_db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << (err ? err : "") << std::endl;
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

bool DB::execFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[DB::execFile] Cannot open file: " << filePath << std::endl;
        return false;
    }
    std::string sql((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    bool result = exec(sql);
    if (!result) {
        std::cerr << "[DB::execFile] SQL execution failed for file: " << filePath << std::endl;
    } else {
        std::cerr << "[DB::execFile] Successfully loaded: " << filePath << std::endl;
    }
    return result;
}

int DB::lastInsertId() {
    std::lock_guard<std::mutex> lock(g_db_mutex);
    if (!g_db) return 0;
    return (int)sqlite3_last_insert_rowid(g_db);
}

int DB::queryInt(const std::string &sql) {
    std::lock_guard<std::mutex> lock(g_db_mutex);
    if (!g_db) return 0;
    sqlite3_stmt *stmt = nullptr;
    int result = 0;
    if (sqlite3_prepare_v2(g_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return result;
}
