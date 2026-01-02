#include "QuestionDAO.h"
#include "DB.h"
#include <sqlite3.h>
#include <iostream>

std::optional<Question> QuestionDAO::findById(int questionId) {
    sqlite3 *db = DB::getHandle();
    if (!db) return std::nullopt;
    const char *sql = "SELECT question_id, content, answer, category FROM Questions WHERE question_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, questionId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Question q;
            q.id = sqlite3_column_int(stmt, 0);
            q.text = (const char *)sqlite3_column_text(stmt, 1);
            q.correctAnswer = (const char *)sqlite3_column_text(stmt, 2);
            q.category = (const char *)sqlite3_column_text(stmt, 3);
            sqlite3_finalize(stmt);
            return q;
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    return std::nullopt;
}

int QuestionDAO::createQuestion(const Question &q) {
    sqlite3 *db = DB::getHandle();
    if (!db) return 0;
    const char *sql = "INSERT INTO Questions (content, answer, category) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, q.text.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, q.correctAnswer.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, q.category.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert question failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return 0;
        }
    } else {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        if (stmt) sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    return DB::lastInsertId();
}
