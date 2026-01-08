#include "core/Server.h"
#include "database/DB.h"
#include "database/QuestionDAO.h"
#include <iostream>
#include "database/UserDAO.h"
#include <ctime>   // <-- BẮT BUỘC THÊM
#include <cstdlib>
int main() {
    std::srand(std::time(0));
    // initialize database
    if (!DB::open("vua.db")) {
        std::cerr << "Failed to open database\n";
        return 1;
    }

    // run schema (create tables if not exists)
    // main runs from server/ working directory; schema lives in database/schema.sql
    std::cerr << "[MAIN] Loading schema from database/schema.sql..." << std::endl;
    if (!DB::execFile("database/schema.sql")) {
        std::cerr << "[MAIN] SQL schema import FAILED - check path and permissions" << std::endl;
    } else {
        std::cerr << "[MAIN] Schema loaded successfully" << std::endl;
    }
    UserDAO::clearAllSessions();

    // Seed questions from SQL file if database is empty or insufficient
    int qcount = DB::queryInt("SELECT COUNT(*) FROM Questions;");
    if (qcount < 10) {
        // questions_v2.sql is located at repo root; server binary runs from server/ working directory
        if (!DB::execFile("../questions_v2.sql")) {
            std::cerr << "Warning: Failed to import questions_v2.sql" << std::endl;
        }
    }

    Server server(8080);
    server.start();

    DB::close();
    return 0;
}
