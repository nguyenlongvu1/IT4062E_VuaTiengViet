#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <netinet/in.h>

class ClientHandler;

class Server {
private:
    int port;
    int server_fd;
    sockaddr_in server_addr;

    std::vector<ClientHandler*> clients;
    std::map<int, ClientHandler*> user_map; // userId -> client handler
    std::mutex clients_mutex;

public:
    Server(int port);
    ~Server();

    void start();
    void acceptLoop();

    void removeClient(ClientHandler* handler);
    void registerUser(int userId, ClientHandler* handler);
    void unregisterUser(int userId);
    void sendToUser(int userId, const std::string& msg);
    void sendToUsers(const std::vector<int>& userIds, const std::string& msg);
    bool isUserOnline(int userId);
    static Server* getInstance();
    static ClientHandler* findClient(const std::string& username);
};
