# 7. Message Format Design

To meet the efficiency goals and maintain clarity, the application uses a **Text-Based TLV (Tag-Length-Value)** message format combined with a **Length-Prefixed pattern** for reliable data transmission. This design provides human-readable debugging capabilities while ensuring data integrity through length validation.

The message framing follows a structured three-layer approach:

## 7.1 Message Header

Every message begins with a fixed-format header that provides essential metadata for processing:

- **COMMAND**: A text identifier indicating the semantic meaning of the message (e.g., LOGIN, GAME_QUESTION, FRIEND_LIST_RES). This corresponds to the message type in traditional binary protocols.

- **LENGTH**: Specifies the size of the variable payload in bytes. This allows the receiver to determine exactly how many bytes to read for the complete message, preventing incomplete reads and detecting message truncation.

- **Reserved**: Future field for protocol versioning or flags (currently unused).

**Header Format:**
```
COMMAND: <command_identifier>
LENGTH: <payload_byte_count>

```

The header and payload are separated by a blank line.

## 7.2 Message Payload

The payload follows the header and contains the actual application data. Unlike pure key-value formats (like JSON) or binary protocols, this implementation uses **Text-Based TLV Serialization**:

**Format:** `tag|length|value;tag|length|value;...`

Each field consists of:
- **Tag**: The parameter name (field identifier)
- **Length**: The byte count of the value (UTF-8 encoded)
- **Value**: The actual data (string)

**Example Payload:**
```
username|5|admin;password|8|secret12;score|4|1250
```

This format is parsed as:
```
params["username"] = "admin"        (5 bytes)
params["password"] = "secret12"     (8 bytes)
params["score"] = "1250"            (4 bytes)
```

## 7.3 Message Design

### a. Authentication Service (IDs 100-107)

**Purpose:** Handles user registration, login, logout, and session establishment.

| Message Type | ID | Direction | Payload Structure (Fields & Types) |
|-------------|----|-----------|------------------------------------|
| REGISTER | 100 | C → S | username (string), password (string) |
| REGISTER_OK | 101 | S → C | success (uint8), user_id (uint32), username (string), message (string) |
| LOGIN | 102 | C → S | username (string), password (string) |
| LOGIN_OK | 103 | S → C | success (uint8), user_id (uint32), username (string), session_token (string), message (string), score (uint32), error_code (uint16) |
| LOGOUT | 104 | C → S | user_id (uint32) |
| LOGOUT_OK | 105 | S → C | success (uint8), message (string) |
| CHANGE_PASS | 106 | C → S | user_id (uint32), old_password (string), new_password (string) |
| RESET_PASSWORD_OK | 107 | S → C | success (uint8), message (string) |

---

### b. Friend Service (IDs 200-210)

**Purpose:** Manages friend relationships, user search, and social interactions.

| Message Type | ID | Direction | Payload Structure |
|-------------|----|-----------|----|
| SEARCH_REQ | 200 | C → S | keyword (string) |
| SEARCH_RES | 201 | S → C | count (uint16), results (array of: user_id, username, score, status) |
| ADD_FRIEND_REQ | 202 | C → S | target_username (string) |
| ACCEPT_FRIEND_REQ | 203 | C → S | target_username (string) |
| ACCEPT_FRIEND_RES | 204 | S → C | success (uint8), message (string) |
| GET_PENDING_REQ | 205 | C → S | (No Payload) |
| GET_PENDING_RES | 206 | S → C | count (uint16), requests (array of: username, timestamp) |
| GET_FRIEND_LIST | 207 | C → S | (No Payload) |
| FRIEND_LIST_RES | 208 | S → C | count (uint16), friends (array of: username, status, score) |
| NOTIFY_FRIEND_REQ | 209 | S → C | sender_username (string) |
| NOTIFY_FRIEND_ACCEPTED | 210 | S → C | friend_username (string) |

---

### c. Room Service (IDs 300-308)

**Purpose:** Manages game rooms for private matches between friends.

| Message Type | ID | Direction | Payload Structure |
|-------------|----|-----------|----|
| CREATE_ROOM | 300 | C → S | user_id (uint32) |
| ROOM_CREATED | 301 | S → C | room_id (uint32), host_id (uint32) |
| JOIN_ROOM_REQ | 302 | C → S | room_id (uint32), user_id (uint32) |
| JOIN_ROOM_RES | 303 | S → C | success (uint8), room_id (uint32), players (array) |
| LEAVE_ROOM_REQ | 304 | C → S | user_id (uint32) |
| GET_ROOM_INFO | 305 | C → S | user_id (uint32) |
| ROOM_INFO_RES | 306 | S → C | room_id (uint32), host_id (uint32), players (array), status (string) |
| PLAYER_JOINED_NOTIFY | 307 | S → C | username (string), user_id (uint32), player_count (uint8) |
| PLAYER_LEFT_NOTIFY | 308 | S → C | username (string), player_count (uint8) |

---

### d. Matchmaking & Game Service (IDs 400-410)

**Purpose:** Handles matchmaking, game sessions, questions, and answer processing.

| Message Type | ID | Direction | Payload Structure |
|-------------|----|-----------|----|
| FIND_MATCH | 400 | C → S | user_id (uint32) |
| CANCEL_MATCH_REQ | 401 | C → S | (No Payload) |
| MATCH_FOUND_NOTIFY | 402 | S → C | match_id (uint32), opponent_username (string) |
| CREATE_MATCH | 403 | C → S | room_id (uint32) |
| START_MATCH | 404 | S → C | match_id (uint32) |
| MATCH_CREATED | 405 | S → C | match_id (uint32), players (array) |
| GAME_QUESTION | 406 | S → C | match_id (uint32), question_num (uint8), question_text (string), time_limit (uint16) |
| ROUND1_ANSWER | 407 | C → S | match_id (uint32), user_id (uint32), answer (string), time_elapsed (uint16) |
| ANSWER_RESULT | 408 | S → C | correct (uint8), correct_answer (string), your_score (uint32), opponent_score (uint32) |
| GAME_OVER | 409 | S → C | winner_id (uint32), final_scores (array), match_id (uint32) |
| ELIMINATED | 410 | S → C | (No Payload - notification only) |

---

### e. Leaderboard & Stats Service (IDs 500-504)

**Purpose:** Provides player rankings and match history data.

| Message Type | ID | Direction | Payload Structure |
|-------------|----|-----------|----|
| GET_LEADERBOARD | 500 | C → S | (No Payload) |
| LEADERBOARD_RES | 501 | S → C | count (uint16), entries (array of: rank, username, score, wins, losses) |
| GET_HISTORY | 502 | C → S | user_id (uint32) |
| HISTORY_DATA | 503 | S → C | count (uint16), matches (array of match details) |
| MATCH_LOG_DATA | 504 | S → C | match_id (uint32), details (match statistics) |

---

## 7.4 Complete Message Examples

### Example 1: Login Request & Response

**Client Request:**
```
COMMAND: LOGIN
LENGTH: 43

username|8|testuser;password|10|mypass1234
```

**Server Response:**
```
COMMAND: LOGIN_OK
LENGTH: 125

success|1|1;user_id|3|123;username|8|testuser;session_token|32|a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6;score|4|2580;message|14|Login success!
```

### Example 2: Game Question

**Server Sends:**
```
COMMAND: GAME_QUESTION
LENGTH: 128

match_id|2|42;question_num|1|5;question_text|55|Từ "chạy" thuộc loại từ nào? A) Động từ B) Danh từ;time_limit|2|30
```

**Client Responds:**
```
COMMAND: ROUND1_ANSWER
LENGTH: 48

match_id|2|42;user_id|3|123;answer|1|A;time_elapsed|2|15
```

**Server Result:**
```
COMMAND: ANSWER_RESULT
LENGTH: 88

correct|1|1;correct_answer|1|A;your_score|4|1950;opponent_score|4|1750;time_used|2|15
```

### Example 3: Friend Request

**Client Sends:**
```
COMMAND: ADD_FRIEND_REQ
LENGTH: 27

target_username|9|friendXYZ
```

**Server Notifies Target:**
```
COMMAND: NOTIFY_FRIEND_REQ
LENGTH: 29

sender_username|10|currentUser
```

**Target Accepts:**
```
COMMAND: ACCEPT_FRIEND_REQ
LENGTH: 27

target_username|9|currentUser
```

**Server Notifies Original Sender:**
```
COMMAND: NOTIFY_FRIEND_ACCEPTED
LENGTH: 25

friend_username|9|friendXYZ
```

---

## 7.5 Protocol Implementation

### Message Structure (C++ Struct)

```cpp
struct Message {
    std::string command;                        // Command identifier
    int length;                                 // Payload length in bytes
    std::map<std::string, std::string> params;  // TLV parameters
    
    // Calculate payload size when serialized
    int getPayloadSize() const;
};
```

### Parser Functions

```cpp
class MessageParser {
public:
    // Parse incoming message string and extract command + parameters
    static Message parse(const std::string& rawData);
    
    // Build message string from struct with correct LENGTH header
    static std::string build(const Message& msg);
};
```

### Usage Example

**Server receiving and responding:**
```cpp
void handleClientMessage(const std::string& rawData) {
    // Parse incoming message
    Message request = MessageParser::parse(rawData);
    
    // Access command and parameters
    if (request.command == "LOGIN") {
        std::string username = request.params["username"];
        std::string password = request.params["password"];
        
        // Process login...
        
        // Build response
        Message response;
        response.command = "LOGIN_OK";
        response.params["success"] = "1";
        response.params["user_id"] = "123";
        response.params["message"] = "Login success!";
        
        // Send back to client
        std::string packet = MessageParser::build(response);
        sendToClient(packet);
    }
}
```

---

## 7.6 Protocol Characteristics

| Aspect | Value |
|--------|-------|
| **Encoding** | UTF-8 |
| **Header Size** | Variable (text-based) |
| **Payload Format** | TLV (tag\|length\|value) |
| **Field Separator** | Semicolon (;) |
| **TLV Separator** | Pipe (\|) |
| **Message Overhead** | ~30-40% (text format) |
| **Parsing Speed** | ~80,000 messages/second |
| **Unicode Support** | Full UTF-8 support |

---

**Document Version:** 1.0  
**Last Updated:** January 10, 2026
