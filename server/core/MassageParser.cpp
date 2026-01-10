#include "MessageParser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

static inline std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) start++;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end-1])) end--;
    return s.substr(start, end-start);
}

Message MessageParser::parse(const std::string &s) {
    Message msg;
    msg.length = 0;  // Default: no length specified
    
    size_t pos = s.find("COMMAND:");
    if (pos != std::string::npos) {
        // parse lines
        std::istringstream iss(s);
        std::string line;
        std::string body;
        int expectedLength = -1;
        bool inBody = false;
        while (std::getline(iss, line)) {
            if (line.empty()) { inBody = true; continue; }
            if (!inBody) {
                size_t p = line.find(':');
                if (p != std::string::npos) {
                    std::string key = trim(line.substr(0, p));
                    std::string value = trim(line.substr(p+1));
                    if (key == "COMMAND") {
                        msg.command = value;
                    } else if (key == "LENGTH") {
                        try {
                            expectedLength = std::stoi(value);
                            msg.length = expectedLength;  // Store LENGTH in struct
                        } catch (...) {
                            expectedLength = -1;
                            msg.length = 0;
                        }
                    }
                }
            } else {
                if (!body.empty()) body += "\n";
                body += line;
            }
        }
        
        // Validate payload length if LENGTH was specified
        if (expectedLength >= 0 && static_cast<int>(body.size()) != expectedLength) {
        
            msg.length = static_cast<int>(body.size());  // Update with actual size
        }
        // Try TLV body: token format "tag|len|value" separated by ';'
        bool parsedTLV = false;
        {
            std::string cur = body;
            size_t start = 0;
            while (start < cur.size()) {
                size_t posSemi = cur.find(';', start);
                std::string token = (posSemi == std::string::npos) ? cur.substr(start) : cur.substr(start, posSemi - start);
                token = trim(token);
                if (!token.empty()) {
                    size_t p1 = token.find('|');
                    size_t p2 = (p1 == std::string::npos) ? std::string::npos : token.find('|', p1 + 1);
                    if (p1 != std::string::npos && p2 != std::string::npos) {
                        std::string k = trim(token.substr(0, p1));
                        std::string lenStr = trim(token.substr(p1 + 1, p2 - p1 - 1));
                        std::string v = token.substr(p2 + 1);
                        try {
                            int declared = std::stoi(lenStr);
                            if (declared == static_cast<int>(v.size())) {
                                msg.params[k] = v;
                                parsedTLV = true;
                            }
                        } catch (...) {
                            // ignore malformed token
                        }
                    }
                }
                if (posSemi == std::string::npos) break;
                start = posSemi + 1;
            }
        }

        // Fallback: key=value;key=value;
        if (!parsedTLV) {
            std::string cur = body;
            size_t start = 0;
            while (start < cur.size()) {
                size_t posSemi = cur.find(';', start);
                std::string token = (posSemi == std::string::npos) ? cur.substr(start) : cur.substr(start, posSemi - start);
                token = trim(token);
                if (!token.empty()) {
                    size_t eq = token.find('=');
                    if (eq != std::string::npos) {
                        std::string k = trim(token.substr(0, eq));
                        std::string v = trim(token.substr(eq+1));
                        msg.params[k] = v;
                    }
                }
                if (posSemi == std::string::npos) break;
                start = posSemi + 1;
            }
        }
        return msg;
    }

    // Fallback: old whitespace-separated format
    std::istringstream iss(s);
    iss >> msg.command;
    std::string kv;
    while (iss >> kv) {
        size_t eq = kv.find('=');
        if (eq != std::string::npos) {
            msg.params[kv.substr(0, eq)] = kv.substr(eq+1);
        }
    }
    return msg;
}

std::string MessageParser::build(const Message &msg) {
    // Build body in TLV format: key|len|value;...
    std::ostringstream oss;
    oss << "COMMAND: " << msg.command << "\n";
    
    std::ostringstream body;
    bool first = true;
    for (auto &kv : msg.params) {
        if (!first) body << ";";
        first = false;
        body << kv.first << "|" << kv.second.size() << "|" << kv.second;
    }
    
    std::string bodyStr = body.str();
    int payloadSize = static_cast<int>(bodyStr.size());
    
    // Write LENGTH header with actual payload size
    oss << "LENGTH: " << payloadSize << "\n\n";
    oss << bodyStr;
    
    return oss.str();
}


