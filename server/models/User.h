#pragma once
#include <string>

struct User {
    int id = 0;
    std::string username;
    std::string password;
    bool online = false;
    int total_points = 0;
};
