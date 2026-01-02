PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS Users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL,
    total_points INTEGER DEFAULT 0,
    failed_login_attempts INTEGER DEFAULT 0,
    is_locked INTEGER DEFAULT 0,
    locked_until DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS Sessions (
    session_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    token TEXT,
    room_id INTEGER,
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_active DATETIME DEFAULT CURRENT_TIMESTAMP,
    status TEXT,
    FOREIGN KEY(user_id) REFERENCES Users(user_id)
);

CREATE TABLE IF NOT EXISTS Rooms (
    room_id INTEGER PRIMARY KEY AUTOINCREMENT,
    host_id INTEGER NOT NULL,
    status TEXT,
    invite_code TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    started_at DATETIME,
    ended_at DATETIME,
    rank_id INTEGER,
    FOREIGN KEY(host_id) REFERENCES Users(user_id)
);

CREATE TABLE IF NOT EXISTS Match (
    match_id INTEGER PRIMARY KEY AUTOINCREMENT,
    room_id INTEGER NOT NULL,
    rank_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(room_id) REFERENCES Rooms(room_id)
);

CREATE TABLE IF NOT EXISTS MatchPlayers (
    match_player_id INTEGER PRIMARY KEY AUTOINCREMENT,
    match_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    rank_position INTEGER,
    match_score INTEGER,
    FOREIGN KEY(match_id) REFERENCES Match(match_id),
    FOREIGN KEY(user_id) REFERENCES Users(user_id)
);

CREATE TABLE IF NOT EXISTS Questions (
    question_id INTEGER PRIMARY KEY AUTOINCREMENT,
    content TEXT,
    answer TEXT,
    category TEXT
);

CREATE TABLE IF NOT EXISTS MatchQuestions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    match_id INTEGER NOT NULL,
    question_id INTEGER NOT NULL,
    round INTEGER,
    q_order INTEGER,
    FOREIGN KEY(match_id) REFERENCES Match(match_id),
    FOREIGN KEY(question_id) REFERENCES Questions(question_id)
);

CREATE TABLE IF NOT EXISTS Friends (
    friend_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    friend_user_id INTEGER NOT NULL,
    status TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES Users(user_id),
    FOREIGN KEY(friend_user_id) REFERENCES Users(user_id)
);

CREATE TABLE IF NOT EXISTS Ranks (
    rank_id INTEGER PRIMARY KEY AUTOINCREMENT,
    rank_name TEXT,
    min_point INTEGER,
    max_point INTEGER
);
