#pragma once
#include <string>

class Server;

class ClientHandler {
private:
    int client_fd;
    Server* server;
    bool running = true;
    int user_id = 0; // 0 indicates not logged-in

public:
    ClientHandler(int fd, Server* server);

    void run();
    std::string readMessage();
    void sendMessage(const std::string& msg);
    void stop();
    void setUserId(int id) { user_id = id; }
    int getUserId() const { return user_id; }
};
