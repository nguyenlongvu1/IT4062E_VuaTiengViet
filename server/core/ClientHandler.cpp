#include "ClientHandler.h"
#include "MessageParser.h"
#include "Dispatcher.h"
#include "Server.h"
#include "../services/RoomService.h"
#include "../database/UserDAO.h"
#include <unistd.h>
#include <iostream>

ClientHandler::ClientHandler(int fd, Server* srv)
    : client_fd(fd), server(srv) {}

void ClientHandler::run() {
    while (running) {
        std::string msg = readMessage();
        if (msg.empty()) break;

        std::cout << "[CLIENT] >> " << msg;

        auto parsed = MessageParser::parse(msg);
        Message response = Dispatcher::handleCommand(parsed, this);
        std::string responseStr = MessageParser::build(response);

        // after login, register user mapping
        if (response.command == "LOGIN_OK" && response.params.count("user_id")) {
            int uid = std::stoi(response.params.at("user_id"));
            setUserId(uid);
            server->registerUser(uid, this);
        }

        // after logout, unregister user mapping
        if (response.command == "LOGOUT_OK" && response.params.count("cleanup_user_map")) {
            if (response.params.count("user_id")) {
                int uid = std::stoi(response.params.at("user_id"));
                server->unregisterUser(uid);
                setUserId(0);
            }
        }

        // special handling for broadcasts (room-scoped messages)
        if (response.params.count("broadcast") && response.params.at("broadcast") == "true") {
            std::vector<int> players;
            if (response.params.count("room_id")) {
                int roomId = std::stoi(response.params.at("room_id"));
                players = RoomService::getPlayers(roomId);
            } else if (response.params.count("players")) {
                // players comma-separated
                std::string list = response.params.at("players");
                size_t start = 0;
                while (start < list.size()) {
                    size_t pos = list.find(',', start);
                    std::string token = (pos==std::string::npos) ? list.substr(start) : list.substr(start, pos-start);
                    players.push_back(std::stoi(token));
                    if (pos==std::string::npos) break;
                    start = pos + 1;
                }
            }
            // if it's an 'ask' we probably don't need to notify requester
            if (response.command == "REMATCH_ASK") {
                std::vector<int> targets;
                for (int p : players) if (p != getUserId()) targets.push_back(p);
                server->sendToUsers(targets, responseStr);
            } else {
                server->sendToUsers(players, responseStr);
            }
            // if rematch ok and auto-start set, start match
            if (response.params.count("auto_start_match") && response.params.at("auto_start_match") == "true") {
                // call GameService START_MATCH via dispatcher
                Message startMsg;
                startMsg.command = "START_MATCH";
                startMsg.params["room_id"] = response.params.at("room_id");
                Message startResp = Dispatcher::handleCommand(startMsg, this);
                std::string startRespStr = MessageParser::build(startResp);
                if (startRespStr.size() > 0) {
                    // broadcast start response (handled by same broadcast mechanism)
                    if (startResp.params.count("players")) {
                        // broadcast to players in response
                        if (startResp.params.count("broadcast") && startResp.params.at("broadcast") == "true") {
                            // server will send to players found in players param
                            std::vector<int> targets2;
                            std::string list = startResp.params.at("players");
                            size_t start = 0;
                            while (start < list.size()) {
                                size_t pos = list.find(',', start);
                                std::string token = (pos==std::string::npos) ? list.substr(start) : list.substr(start, pos-start);
                                targets2.push_back(std::stoi(token));
                                if (pos==std::string::npos) break;
                                start = pos + 1;
                            }
                            server->sendToUsers(targets2, startRespStr);
                        } else {
                            sendMessage(startRespStr);
                        }
                    } else {
                        // broadcast to all players of the room if no players field
                        if (response.params.count("room_id")) {
                            auto playersList = RoomService::getPlayers(std::stoi(response.params.at("room_id")));
                            server->sendToUsers(playersList, startRespStr);
                        }
                    }
                }
            }
        } else {
            if (!responseStr.empty()) {
                sendMessage(responseStr);
            }
        }
    }

    // Auto-cleanup: if user was logged in, remove session on disconnect
    int uid = getUserId();
    if (uid > 0) {
        std::cout << "[CLEANUP] User (ID: " << uid << ") disconnected. Removing session from DB and unregistering from server...\n";
        UserDAO::removeSession(uid);  // Remove session row from DB
        server->unregisterUser(uid);   // Remove from server's user_map (marks offline)
        std::cout << "[CLEANUP] User (ID: " << uid << ") cleaned up successfully.\n";
    } else {
        std::cout << "[CLEANUP] Client disconnected without logging in.\n";
    }

    close(client_fd);
    server->removeClient(this);
    // DO NOT delete this - let Server manage lifecycle
}

std::string ClientHandler::readMessage() {
    char buffer[1024];
    int n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n <= 0) return "";
    return std::string(buffer, n);
}

void ClientHandler::sendMessage(const std::string& msg) {
    send(client_fd, msg.c_str(), msg.size(), 0);
}

void ClientHandler::stop() {
    running = false;
}
