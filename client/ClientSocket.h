#pragma once
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct Message {
    std::string command;
    std::map<std::string, std::string> params;
};

class ClientSocket {
private:
    int sock;
    std::string host;
    int port;

public:
    ClientSocket(const std::string& h = "127.0.0.1", int p = 8080)
        : sock(-1), host(h), port(p) {}

    ~ClientSocket() {
        close_connection();
    }

    bool connect() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Failed to create socket\n";
            return false;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

        if (::connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to connect to server at " << host << ":" << port << "\n";
            return false;
        }
        return true;
    }

    void close_connection() {
        if (sock >= 0) {
            close(sock);
            sock = -1;
        }
    }

    bool send_message(const Message& msg) {
        std::string data = build_message(msg);
        if (send(sock, data.c_str(), data.size(), 0) < 0) {
            std::cerr << "Failed to send message\n";
            return false;
        }
        return true;
    }

    Message receive_message() {
        char buffer[4096] = {0};
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n < 0) {
            std::cerr << "Failed to receive message\n";
            return Message();
        }
        buffer[n] = '\0';
        return parse_message(std::string(buffer));
    }

private:
    std::string build_message(const Message& msg) {
        std::string body;
        bool first = true;
        for (auto& kv : msg.params) {
            if (!first) body += ";";
            first = false;
            body += kv.first + "=" + kv.second;
        }
        
        std::string result = "COMMAND: " + msg.command + "\n";
        result += "LENGTH: " + std::to_string(body.size()) + "\n\n";
        result += body;
        return result;
    }

    Message parse_message(const std::string& s) {
        Message msg;
        size_t pos = s.find("COMMAND:");
        if (pos != std::string::npos) {
            std::istringstream iss(s);
            std::string line;
            std::string body;
            bool in_body = false;
            
            while (std::getline(iss, line)) {
                if (line.empty()) {
                    in_body = true;
                    continue;
                }
                if (!in_body) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string key = line.substr(0, colon);
                        std::string value = line.substr(colon + 1);
                        key.erase(0, key.find_first_not_of(" \t\n\r"));
                        key.erase(key.find_last_not_of(" \t\n\r") + 1);
                        value.erase(0, value.find_first_not_of(" \t\n\r"));
                        value.erase(value.find_last_not_of(" \t\n\r") + 1);
                        if (key == "COMMAND") msg.command = value;
                    }
                } else {
                    if (!body.empty()) body += "\n";
                    body += line;
                }
            }
            
            size_t start = 0;
            while (start < body.size()) {
                size_t semi = body.find(';', start);
                std::string token = (semi == std::string::npos) ? body.substr(start) : body.substr(start, semi - start);
                token.erase(0, token.find_first_not_of(" \t\n\r"));
                token.erase(token.find_last_not_of(" \t\n\r") + 1);
                
                if (!token.empty()) {
                    size_t eq = token.find('=');
                    if (eq != std::string::npos) {
                        std::string k = token.substr(0, eq);
                        std::string v = token.substr(eq + 1);
                        k.erase(0, k.find_first_not_of(" \t\n\r"));
                        k.erase(k.find_last_not_of(" \t\n\r") + 1);
                        v.erase(0, v.find_first_not_of(" \t\n\r"));
                        v.erase(v.find_last_not_of(" \t\n\r") + 1);
                        msg.params[k] = v;
                    }
                }
                if (semi == std::string::npos) break;
                start = semi + 1;
            }
        }
        return msg;
    }
};
