#pragma once
#include "MessageParser.h"
class ClientHandler;

class Dispatcher {
public:
    static Message handleCommand(const Message& msg, ClientHandler* client);
};
