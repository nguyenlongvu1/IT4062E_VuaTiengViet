#pragma once
#include <string>
#include <sqlite3.h>

class DB {
public:
    static bool open(const std::string &filename);
    static void close();
    static sqlite3 *getHandle();
    static bool exec(const std::string &sql);
    static bool execFile(const std::string &filePath);
    static int lastInsertId();
    static int queryInt(const std::string &sql); // returns single int result or 0 if fail
};
