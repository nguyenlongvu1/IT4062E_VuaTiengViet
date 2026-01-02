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
    // Detect TLV header: look for COMMAND: and LENGTH:
    size_t pos = s.find("COMMAND:");
    if (pos != std::string::npos) {
        // parse lines
        std::istringstream iss(s);
        std::string line;
        std::string body;
        bool inBody = false;
        while (std::getline(iss, line)) {
            if (line.empty()) { inBody = true; continue; }
            if (!inBody) {
                size_t p = line.find(':');
                if (p != std::string::npos) {
                    std::string key = trim(line.substr(0, p));
                    std::string value = trim(line.substr(p+1));
                    if (key == "COMMAND") msg.command = value;
                    // ignore LENGTH for now
                }
            } else {
                if (!body.empty()) body += "\n";
                body += line;
            }
        }
        // body is like: key=value;key=value;...
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
    // Build TLV format with body using semicolon separators
    std::ostringstream oss;
    oss << "COMMAND: " << msg.command << "\n";
    // build body
    std::ostringstream body;
    bool first = true;
    for (auto &kv : msg.params) {
        if (!first) body << ";";
        first = false;
        body << kv.first << "=" << kv.second;
    }
    std::string bodyStr = body.str();
    oss << "LENGTH: " << bodyStr.size() << "\n\n";
    oss << bodyStr;
    return oss.str();
}


