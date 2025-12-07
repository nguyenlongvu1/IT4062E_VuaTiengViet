#pragma once
#include <string>

struct Friendship {
    int id;
    int user_id_1;
    int user_id_2;
    std::string status; // "pending", "accepted"
    std::string created_at;
};
