#pragma once
#include <string>
#include <map>

struct Message {
    std::string command;
    std::map<std::string, std::string> params;
};

class MessageParser {
public:
    static Message parse(const std::string& s);
    static std::string build(const Message& msg);
};
