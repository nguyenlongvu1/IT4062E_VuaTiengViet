CREATE TABLE IF NOT EXISTS Match_Log (
        log_id INTEGER PRIMARY KEY AUTOINCREMENT,
        match_id INTEGER NOT NULL,
        round_id INTEGER NOT NULL,
        question_id INTEGER NOT NULL,
        user_id INTEGER NOT NULL,
        user_answer TEXT,
        is_correct INTEGER,     -- 0: Sai, 1: Đúng
        points_earned INTEGER,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(match_id) REFERENCES Match(match_id),
        FOREIGN KEY(user_id) REFERENCES Users(user_id)
    );