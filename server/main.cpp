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
    DB::execFile("server/database/schema.sql");
    UserDAO::clearAllSessions();

    // seed sample question if none

    Server server(8080);
    server.start();

    DB::close();
    return 0;
}
