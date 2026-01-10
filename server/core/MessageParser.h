#pragma once
#include <string>
#include <map>

/**
 * Text-Based TLV Message Structure
 * 
 * Format:
 * COMMAND: <command_name>
 * LENGTH: <payload_length>
 * 
 * <payload in TLV format: tag|length|value;tag|length|value;...>
 */
struct Message {
    std::string command;                        // Command identifier (e.g., "LOGIN", "GAME_QUESTION")
    int length;                                 // Payload length in bytes (0 if not set)
    std::map<std::string, std::string> params;  // TLV payload parameters
    
    // Constructor
    Message() : command(""), length(0) {}
    
    /**
     * Get size of payload when serialized as TLV format
     * Calculates: sum of (tag_len + "|" + length_str_len + "|" + value_len + ";")
     */
    int getPayloadSize() const {
        int size = 0;
        bool first = true;
        for (const auto& kv : params) {
            if (!first) size++; // for ";"
            first = false;
            size += kv.first.length();      // tag
            size += 1;                       // "|"
            size += std::to_string(kv.second.length()).length(); // length as string
            size += 1;                       // "|"
            size += kv.second.length();      // value
        }
        return size;
    }
};

class MessageParser {
public:
    /**
     * Parse incoming text-based TLV message
     * Validates LENGTH field and extracts command + parameters
     */
    static Message parse(const std::string& s);
    
    /**
     * Build outgoing text-based TLV message
     * Automatically calculates LENGTH field
     */
    static std::string build(const Message& msg);
};
