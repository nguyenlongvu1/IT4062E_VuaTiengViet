#include "core/Server.h"
#include "database/DB.h"
#include "database/QuestionDAO.h"
#include <iostream>

int main() {
    // initialize database
    if (!DB::open("vua.db")) {
        std::cerr << "Failed to open database\n";
        return 1;
    }

    // run schema (create tables if not exists)
    DB::execFile("server/database/schema.sql");

    // seed sample question if none

    Server server(8080);
    server.start();

    DB::close();
    return 0;
}
