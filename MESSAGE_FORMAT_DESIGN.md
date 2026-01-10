# 7. Message Format Design

To meet the efficiency goals, the application uses a **Text-Based Tag-Length-Value (TLV)** message format rather than pure binary format. This decision provides human-readable debugging capabilities while maintaining structured data transmission and length validation for variable-length fields.

The message framing follows a **Length-Prefixed** pattern, structured as follows:

## 7.1 Message Header

Every message begins with a text-based header that provides essential metadata for processing:

- **COMMAND**: A text identifier indicating the semantic meaning of the message (e.g., `LOGIN`, `GAME_QUESTION`, `FRIEND_LIST_RES`). This corresponds to the message type/opcode in binary protocols.

- **LENGTH**: Specifies the size (in bytes) of the variable payload. This allows the receiver to determine exactly how many bytes to read for the complete message body, preventing incomplete reads and buffer issues.

**Header Format:**
```
COMMAND: <command_identifier>
LENGTH: <payload_byte_count>

<payload>
```

The header and payload are separated by a blank line (`\n\n`).

## 7.2 Message Payload

The payload follows the header and contains the actual application data. Unlike pure key-value formats (like JSON) or binary Tag-Length-Value (TLV) formats, this protocol uses **Text-Based TLV Serialization**.

Fields are written in **Tag|Length|Value** format, separated by semicolons (`;`). Each field consists of:
- **Tag**: The parameter name (e.g., `username`, `password`, `score`)
- **Length**: The byte count of the value (UTF-8 encoded)
- **Value**: The actual data (strings are UTF-8 encoded)

**Payload Format:**
```
tag1|length1|value1;tag2|length2|value2;tag3|length3|value3
```

**Example:**
```
username|5|admin;password|8|secret12;score|4|1250
```

### Fallback Format

For backward compatibility and simpler messages, the parser also supports **key=value** format:
```
key1=value1;key2=value2;key3=value3
```

This fallback is used when TLV parsing fails or for legacy message types.

## 7.3 Complete Message Example

### Authentication Message (LOGIN_REQ)

**Request from Client:**
```
COMMAND: LOGIN
LENGTH: 45

username|5|admin;password|12|securepass12
```

**Response from Server:**
```
COMMAND: LOGIN_OK
LENGTH: 72

success|1|1;user_id|3|123;username|5|admin;session_token|16|a1b2c3d4e5f6g7h8
```

### Game Question (GAME_QUESTION)

**Server sends question to client:**
```
COMMAND: GAME_QUESTION
LENGTH: 98

match_id|2|42;question_num|1|5;question_text|28|What is the capital of France?;time_limit|2|30
```

### Error Response

**Server sends error:**
```
COMMAND: ERROR
LENGTH: 35

error_code|3|401;message|15|Invalid password
```

---

## 7.4 Message Design by Service

### a. Authentication Service (Commands 100-107)

**Purpose:** Handles user registration, login, logout, and session establishment.

| Message Type | Command ID | Direction | Payload Structure (Fields & Types) |
|-------------|-----------|-----------|-----------------------------------|
| REGISTER_REQ | 100 | C → S | `username` (string), `password` (string) |
| REGISTER_RESP | 101 | S → C | `success` (uint8), `user_id` (uint32), `username` (string), `message` (string) |
| LOGIN_REQ | 102 | C → S | `username` (string), `password` (string) |
| LOGIN_RESP | 103 | S → C | `success` (uint8), `user_id` (uint32), `username` (string), `session_token` (string), `message` (string), `score` (uint32), `error_code` (uint16) |
| LOGOUT_REQ | 104 | C → S | `user_id` (uint32) |
| LOGOUT_RESP | 105 | S → C | `success` (uint8), `message` (string) |
| CHANGE_PASS | 106 | C → S | `user_id` (uint32), `old_password` (string), `new_password` (string) |
| CHANGE_PASS_OK | 107 | S → C | `success` (uint8), `message` (string) |

---

### b. Friend Service (Commands 200-210)

**Purpose:** Manages friend relationships, search, and social features.

| Message Type | Command ID | Direction | Payload Structure (Fields & Types) |
|-------------|-----------|-----------|-----------------------------------|
| SEARCH_REQ | 200 | C → S | `keyword` (string) |
| SEARCH_RES | 201 | S → C | `count` (uint16), `results` (array of: `user_id`, `username`, `score`) |
| ADD_FRIEND_REQ | 202 | C → S | `target_username` (string) |
| ACCEPT_FRIEND_REQ | 203 | C → S | `target_username` (string) |
| ACCEPT_FRIEND_RES | 204 | S → C | `success` (uint8), `message` (string) |
| GET_PENDING_REQ | 205 | C → S | *(No Payload)* |
| GET_PENDING_RES | 206 | S → C | `count` (uint16), `requests` (array of: `username`, `timestamp`) |
| GET_FRIEND_LIST | 207 | C → S | *(No Payload)* |
| FRIEND_LIST_RES | 208 | S → C | `count` (uint16), `friends` (array of: `username`, `status`, `score`) |
| NOTIFY_FRIEND_REQ | 209 | S → C | `sender_username` (string) |
| NOTIFY_FRIEND_ACCEPTED | 210 | S → C | `friend_username` (string) |

---

### c. Room Service (Commands 300-308)

**Purpose:** Manages game rooms for private matches between friends.

| Message Type | Command ID | Direction | Payload Structure (Fields & Types) |
|-------------|-----------|-----------|-----------------------------------|
| CREATE_ROOM | 300 | C → S | `user_id` (uint32) |
| ROOM_CREATED | 301 | S → C | `room_id` (uint32), `host_id` (uint32) |
| JOIN_ROOM_REQ | 302 | C → S | `room_id` (uint32), `user_id` (uint32) |
| JOIN_ROOM_RES | 303 | S → C | `success` (uint8), `room_id` (uint32), `players` (array) |
| LEAVE_ROOM_REQ | 304 | C → S | `user_id` (uint32) |
| GET_ROOM_INFO | 305 | C → S | `user_id` (uint32) |
| ROOM_INFO_RES | 306 | S → C | `room_id` (uint32), `host_id` (uint32), `players` (array), `status` (string) |
| PLAYER_JOINED_NOTIFY | 307 | S → C | `username` (string), `user_id` (uint32), `player_count` (uint8) |
| PLAYER_LEFT_NOTIFY | 308 | S → C | `username` (string), `player_count` (uint8) |

---

### d. Matchmaking & Game Service (Commands 400-410)

**Purpose:** Handles matchmaking, game sessions, questions, and answers.

| Message Type | Command ID | Direction | Payload Structure (Fields & Types) |
|-------------|-----------|-----------|-----------------------------------|
| FIND_MATCH | 400 | C → S | `user_id` (uint32) |
| CANCEL_MATCH_REQ | 401 | C → S | *(No Payload)* |
| MATCH_FOUND_NOTIFY | 402 | S → C | `match_id` (uint32), `opponent_username` (string) |
| CREATE_MATCH | 403 | C → S | `room_id` (uint32) |
| START_MATCH | 404 | S → C | `match_id` (uint32) |
| MATCH_CREATED | 405 | S → C | `match_id` (uint32), `players` (array) |
| GAME_QUESTION | 406 | S → C | `match_id` (uint32), `question_num` (uint8), `question_text` (string), `time_limit` (uint16) |
| ROUND1_ANSWER | 407 | C → S | `match_id` (uint32), `user_id` (uint32), `answer` (string), `time_elapsed` (uint16) |
| ANSWER_RESULT | 408 | S → C | `correct` (uint8), `correct_answer` (string), `your_score` (uint32), `opponent_score` (uint32) |
| GAME_OVER | 409 | S → C | `winner_id` (uint32), `final_scores` (array), `match_id` (uint32) |
| ELIMINATED | 410 | S → C | *(No Payload - notification only)* |

---

### e. Leaderboard & Stats Service (Commands 500-504)

**Purpose:** Provides player rankings and match history.

| Message Type | Command ID | Direction | Payload Structure (Fields & Types) |
|-------------|-----------|-----------|-----------------------------------|
| GET_LEADERBOARD | 500 | C → S | *(No Payload)* |
| LEADERBOARD_RES | 501 | S → C | `count` (uint16), `entries` (array of: `rank`, `username`, `score`, `wins`, `losses`) |
| GET_HISTORY | 502 | C → S | `user_id` (uint32) |
| HISTORY_DATA | 503 | S → C | `count` (uint16), `matches` (array of match details) |
| MATCH_LOG_DATA | 504 | S → C | `match_id` (uint32), `details` (match statistics) |

---

## 7.5 Protocol Benefits

✅ **Length Validation**: The LENGTH field ensures complete message reception and prevents partial reads.

✅ **Human-Readable**: Text-based format allows easy debugging and logging without binary parsers.

✅ **Flexible Field Encoding**: TLV format supports variable-length strings with explicit length prefixes.

✅ **UTF-8 Support**: Proper handling of international characters in usernames and game content.

✅ **Backward Compatibility**: Fallback to simple key=value format for legacy support.

✅ **Error Detection**: Length mismatches can be detected during parsing.

---

## 7.6 Implementation Notes

### Parser Validation

The `MessageParser::parse()` function validates:
1. **Header Format**: Ensures `COMMAND:` and `LENGTH:` fields are present
2. **Length Matching**: Compares declared LENGTH with actual payload size
3. **TLV Format**: Attempts to parse `tag|length|value` format first
4. **Fallback Parsing**: Falls back to `key=value` format if TLV fails

### Message Building

The `MessageParser::build()` function automatically:
1. Calculates payload LENGTH from parameters
2. Formats all parameters as `tag|length|value`
3. Generates proper header with COMMAND and LENGTH
4. Ensures UTF-8 encoding for all string values

### Error Handling

- **Length Mismatch**: Logged but parsing continues (can be configured to reject)
- **Malformed TLV**: Falls back to key=value parsing
- **Missing Fields**: Returns empty string for missing parameters
- **Invalid UTF-8**: Handled by Qt's QString UTF-8 conversion on client side
