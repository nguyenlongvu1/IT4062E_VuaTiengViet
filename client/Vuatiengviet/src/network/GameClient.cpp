#include "GameClient.h"
#include "Protocol.h"

GameClient::GameClient() {
    m_socket = new QTcpSocket(this);
    m_lastCommand = "";
    connect(m_socket, &QTcpSocket::connected, this, &GameClient::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &GameClient::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &GameClient::onReadyRead);
}

void GameClient::connectToServer(const QString &ip, quint16 port) {
    if(m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->connectToHost(ip, port);
    }
}

bool GameClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

static QString buildTLVPayloadFromKv(const QString &payloadKv) {
    // input format: key=value;key2=value2
    // output: key|len|value;key2|len|value2
    QStringList tokens = payloadKv.split(";", Qt::SkipEmptyParts);
    QStringList out;
    for (const QString &t : tokens) {
        int eq = t.indexOf('=');
        if (eq == -1) {
            out << t.trimmed();
            continue;
        }
        QString k = t.left(eq).trimmed();
        QString v = t.mid(eq + 1).trimmed();
        out << QString("%1|%2|%3").arg(k).arg(v.toUtf8().size()).arg(v);
    }
    return out.join(";");
}

void GameClient::sendMessage(const QString &command, const QString &payload) {
    if (!isConnected()) return;

    // If payload already looks like TLV (has '|'), keep as-is; else convert key=value pairs to TLV style
    QString tlvPayload = payload.contains('|') ? payload : buildTLVPayloadFromKv(payload);

    QString msg = QString("COMMAND: %1\nLENGTH: %2\n\n%3") 
                  .arg(command)
                  .arg(tlvPayload.toUtf8().length())
                  .arg(tlvPayload);
                  
    m_socket->write(msg.toUtf8());
    m_socket->flush();
}

void GameClient::sendLogin(const QString &u, const QString &p) {
    m_lastCommand = CMD_LOGIN;
    QString payload = QString("username=%1;password=%2").arg(u, p);
    sendMessage(CMD_LOGIN, payload);
}

void GameClient::sendRegister(const QString &u, const QString &p) {
    m_lastCommand = CMD_REGISTER;
    QString payload = QString("username=%1;password=%2").arg(u, p);
    sendMessage(CMD_REGISTER, payload);
}

void GameClient::sendLogout() {
    if (m_currentUserID.isEmpty()) return;
    QString payload = QString("user_id=%1").arg(m_currentUserID);
    sendMessage(CMD_LOGOUT, payload); 
    m_currentUserID.clear();
}

// XỬ LÝ DỮ LIỆU NHẬN TỪ SERVER
void GameClient::onReadyRead() {
    QByteArray data = m_socket->readAll();
    QString rawData = QString::fromUtf8(data);

    qDebug() << "[CLIENT DEBUG] Raw Data received:" << rawData;

    // 1. Tách các gói tin
    QStringList packets = rawData.split("COMMAND: ", Qt::SkipEmptyParts);

    for (const QString &packet : packets) {
        QString response = "COMMAND: " + packet;
        
        // 2. Tách Payload 
        QString payload = "";
        int splitIndex = response.indexOf("\n\n");
        if (splitIndex != -1) payload = response.mid(splitIndex + 2);

        // Parse payload: prefer TLV (key|len|value) else fallback to key=value
        QMap<QString, QString> payloadMap;
        auto tryParseTLV = [&]() {
            bool ok = false;
            QStringList tokens = payload.split(";", Qt::SkipEmptyParts);
            for (const QString &t : tokens) {
                int p1 = t.indexOf('|');
                int p2 = (p1 == -1) ? -1 : t.indexOf('|', p1 + 1);
                if (p1 != -1 && p2 != -1) {
                    QString k = t.left(p1).trimmed();
                    QString lenStr = t.mid(p1 + 1, p2 - p1 - 1).trimmed();
                    QString v = t.mid(p2 + 1);
                    bool lenOk = false;
                    int declared = lenStr.toInt(&lenOk);
                    if (lenOk && declared == v.toUtf8().size()) {
                        payloadMap[k] = v;
                        ok = true;
                    }
                }
            }
            return ok;
        };

        bool parsed = tryParseTLV();
        if (!parsed) {
            QStringList tokens = payload.split(";", Qt::SkipEmptyParts);
            for (const QString &t : tokens) {
                int eq = t.indexOf('=');
                if (eq != -1) {
                    QString k = t.left(eq).trimmed();
                    QString v = t.mid(eq + 1).trimmed();
                    payloadMap[k] = v;
                }
            }
        }

        auto getPayloadValue = [&](QString key) -> QString {
            return payloadMap.value(key);
        };

        // --- NHÓM 1: XỬ LÝ LỖI TỔNG QUÁT (ERROR / FAIL) ---
        // CMD_LOGIN_FAIL và CMD_CHANGE_PASS_FAIL thường là "ERROR"
        if (response.contains(CMD_LOGIN_FAIL) || response.contains("FAIL")) {
            
            QString code = getPayloadValue("error_code");
            QString msg = getPayloadValue("error_msg");

            qDebug() << "[CLIENT ERROR HANDLER] Code:" << code << "| Msg:" << msg << "| LastCmd:" << m_lastCommand;

            // 1.1 Lỗi Đổi Mật Khẩu
            if (code == "WRONG_OLD_PASSWORD") {
                emit changePasswordFailed("Mật khẩu cũ không chính xác!");
            }
            else if (code == "INVALID_NEW_PASSWORD") {
                emit changePasswordFailed("Mật khẩu mới không hợp lệ!");
            }
            else if (m_lastCommand == CMD_CHANGE_PASS) {
                emit changePasswordFailed(msg.isEmpty() ? "Đổi mật khẩu thất bại" : msg);
            }
            // 1.2 Lỗi Đăng Ký
            else if (m_lastCommand == CMD_REGISTER) {
                emit registerFailed(msg, code);
            }
            // 1.3 Lỗi Đăng Nhập
            else if (m_lastCommand == CMD_LOGIN) {
                emit loginFailed(msg, code);
            }
            // 1.4 Các lỗi khác
            else {
                qDebug() << "Unhandled Error Context:" << m_lastCommand;
            }
        }

        // --- NHÓM 2: XỬ LÝ THÀNH CÔNG (SUCCESS CASES) ---
        
        // CASE: ĐĂNG NHẬP THÀNH CÔNG
        else if (response.contains(CMD_LOGIN_OK)) {
           m_currentUserID = getPayloadValue("user_id");
    
            // Lấy thông tin hiển thị
            QString username = getPayloadValue("username");
            QString rankName = getPayloadValue("rank_name"); 
            int points = getPayloadValue("points").toInt();

            qDebug() << "[INFO] Logged in:" << username << "| Rank:" << rankName << "| Points:" << points;

            emit userInfoReceived(username, points, rankName);
            emit loginSuccess();

            // lấy danh sách bạn bè
            sendGetFriendList();
            sendGetPendingRequests();
        } 
        
        // CASE: ĐĂNG KÝ THÀNH CÔNG
        else if (response.contains(CMD_REG_OK)) {
            emit registerSuccess();
        } 

        // CASE: ĐỔI MẬT KHẨU THÀNH CÔNG
        else if (response.contains(CMD_CHANGE_PASS_OK)) {
            qDebug() << "[SUCCESS] Đổi mật khẩu thành công!";
            emit changePasswordSuccess();
        }

        // CASE: KẾT QUẢ TÌM KIẾM
        else if (response.contains(CMD_SEARCH_RES)) {
            QList<UserSearchResult> results;
            QString listStr = getPayloadValue("users"); 
            QStringList users = listStr.split("|", Qt::SkipEmptyParts);
            
            for (const QString& uStr : users) {
                QStringList parts = uStr.split(",");
                if (parts.size() >= 2) {
                    UserSearchResult item;
                    item.username = parts[0].trimmed();
                    item.status = parts[1].trimmed();
                    item.isFriend = false;
                    
                    if (item.username != m_currentUserID) { 
                          results.append(item);
                    }
                }
            }
            emit searchResultReceived(results);
        }

        // CASE: KẾT BẠN
        else if (response.contains(CMD_ACCEPT_FRIEND_RES)) {
            QString status = getPayloadValue("status");
            if (status == "success") {
                QString target = getPayloadValue("target");
                qDebug() << "[SUCCESS] Đã chấp nhận kết bạn với:" << target;
                emit friendListUpdated(); 
            }
        }
        else if (response.contains(CMD_NOTIFY_FRIEND_ACCEPTED)) {
            QString friendName = getPayloadValue("friend_username");
            qDebug() << "[NOTIFY] Chúc mừng! Bạn và " << friendName << " đã trở thành bạn bè.";
            emit friendListUpdated();
        }

        // CASE: DANH SÁCH BẠN BÈ
        else if (response.contains(CMD_FRIEND_LIST_RES)) {
            QList<UserSearchResult> friends;
            QString listStr = getPayloadValue("friends");
            if (listStr.isEmpty()) listStr = getPayloadValue("players");

            QStringList entries = listStr.split("|", Qt::SkipEmptyParts);
            for (const QString& entry : entries) {
                QStringList parts = entry.split(",");
                if (parts.size() >= 2) {
                    UserSearchResult f;
                    f.username = parts[0].trimmed();
                    f.status = parts[1].trimmed();  
                    f.isFriend = true; 
                    
                    if (!f.username.isEmpty() && f.username != m_currentUserID) {
                        friends.append(f);
                    }
                }
            }
            emit friendListReceived(friends);
        }
        else if (response.contains(CMD_UPDATE_STATUS_NOTIFY)) {
             QString username = getPayloadValue("username");
             QString status = getPayloadValue("status"); 
             qDebug() << "[NOTIFY] Friend Status Update:" << username << " -> " << status;
             emit friendStatusChanged(username, status);
        }

        // CASE: LỜI MỜI KẾT BẠN & PENDING
        else if (response.contains(CMD_NOTIFY_FRIEND_REQ)) {
            QString sender = getPayloadValue("sender_username");
            qDebug() << "[NOTIFY] Có lời mời kết bạn từ:" << sender;
            emit friendRequestReceived(sender);
        }
        else if (response.contains(CMD_GET_PENDING_RES)) {
            QString listRaw = getPayloadValue("request_list");
            QStringList list = listRaw.split(",", Qt::SkipEmptyParts);
            emit pendingListReceived(list);
        }

        // CASE: XỬ LÝ PHÒNG (ROOM)
        else if (response.contains(CMD_ROOM_INFO_RES)) {
            QString p1 = getPayloadValue("p1"); //host
            QString p2 = getPayloadValue("p2"); 
            QString p3 = getPayloadValue("p3"); 
            qDebug() << "[NET] Room Data from DB: P1=" << p1 << " P2=" << p2 << " P3=" << p3;
            emit roomInfoReceived(p1, p2, p3);
        }
        else if (response.contains(CMD_JOIN_ROOM_RES)) {
            QString status = getPayloadValue("status");
            if (status == "success") {
                emit joinRoomResult(true, "");
                QString rId = getPayloadValue("room_id");
                if (!rId.isEmpty()) {
                    emit roomJoined(rId); 
                    sendGetRoomInfo(); 
                }
            } else {
                emit joinRoomResult(false, getPayloadValue("reason"));
            }
        }
        else if (response.contains(CMD_ROOM_CREATED)) {
            QString roomId = getPayloadValue("room_id");
            emit roomJoined(roomId); 
            sendGetRoomInfo();
        }
        else if (response.contains(CMD_PLAYER_JOINED_NOTIFY)) {
            emit roomUpdated(); 
            sendGetRoomInfo();
        }
        else if (response.contains(CMD_MATCH_FOUND_NOTIFY)) {
            QString roomId = getPayloadValue("room_id");
            qDebug() << "[CLIENT] TIM THAY TRAN! Room ID:" << roomId;
            emit matchFound(roomId);
        }
        else if (response.contains(CMD_START_MATCH)) {
            QString matchId = getPayloadValue("match_id");
            QString roomId = getPayloadValue("room_id");
            qDebug() << "[NET] Ghép trận thành công! Vào game ngay với MatchID:" << matchId;
            emit matchStartedDirectly(matchId, roomId); 
        }
        else if (response.contains(CMD_PLAYER_LEFT_NOTIFY)) {
            qDebug() << "[NOTIFY] Có người rời phòng -> Tải lại thông tin...";
            emit roomUpdated(); 
            sendGetRoomInfo(); 
        }
        else if (response.contains(CMD_LEADERBOARD_RES)) {
            QList<RankItem> items;
            QString data = getPayloadValue("data");
            QStringList entries = data.split("|", Qt::SkipEmptyParts);
            
            for (const QString& entry : entries) {
                QStringList parts = entry.split(",");
                if (parts.size() >= 3) {
                    RankItem item;
                    item.name = parts[0];
                    item.score = parts[1].toInt();
                    item.rank = parts[2];
                    items.append(item);
                }
            }
            emit leaderboardReceived(items);
        }
        else if (response.contains(CMD_MATCH_CREATED)) {
            QString matchId = getPayloadValue("match_id");
            QString roomId = getPayloadValue("room_id"); 
            QString players = getPayloadValue("players");
            qDebug() << "[NET] Chủ phòng đã tạo match! MatchID:" << matchId << "Players:" << players;

            // Auto-start the match so server sends the first question
            // Server expects: match_id, room_id, players (comma-separated)
            QString payload = QString("match_id=%1;room_id=%2;players=%3")
                                .arg(matchId)
                                .arg(roomId)
                                .arg(players);
            sendMessage("START_MATCH", payload);
            emit matchStartedDirectly(matchId, roomId);
        }
        else if (response.contains("COMMAND: GAME_QUESTION")) {
            int matchId = getPayloadValue("match_id").toInt();
            QString questionNum = getPayloadValue("question_num");
            QString questionId = getPayloadValue("question_id");
            QString questionText = getPayloadValue("question_text");
            int timeLimit = getPayloadValue("time_limit").toInt();
            
            qDebug() << "[CLIENT] Game question received:" << questionNum << questionText;
            emit gameQuestionReceived(matchId, questionNum, questionId, questionText, timeLimit);

            // optional scores snapshot
            QString scoresStr = getPayloadValue("scores");
            if (!scoresStr.isEmpty()) {
                QList<QPair<QString, int>> scores;
                QStringList entries = scoresStr.split(",", Qt::SkipEmptyParts);
                for (const QString &entry : entries) {
                    QStringList parts = entry.split(":");
                    if (parts.size() == 2) {
                        scores.append({parts[0], parts[1].toInt()});
                    }
                }
                emit gameScoresUpdated(scores);
            }
        }
        
        // Answer result received
        else if (response.contains("COMMAND: ANSWER_RESULT")) {
            bool correct = (getPayloadValue("correct") == "true");
            int pointsEarned = getPayloadValue("points_earned").toInt();
            int totalScore = getPayloadValue("total_score").toInt();
            
            qDebug() << "[CLIENT] Answer result:" << (correct ? "Correct" : "Wrong") 
                     << "Points:" << pointsEarned << "Total:" << totalScore;
            emit answerResultReceived(correct, pointsEarned, totalScore);

            // Live scores broadcast
            QString scoresStr = getPayloadValue("scores");
            if (!scoresStr.isEmpty()) {
                QList<QPair<QString, int>> scores;
                QStringList entries = scoresStr.split(",", Qt::SkipEmptyParts);
                for (const QString &entry : entries) {
                    QStringList parts = entry.split(":");
                    if (parts.size() == 2) {
                        scores.append({parts[0], parts[1].toInt()});
                    }
                }
                emit gameScoresUpdated(scores);
            }
            
            // Check if there's a next question
            if (getPayloadValue("next_question") == "true") {
                QString questionNum = getPayloadValue("question_num");
                QString questionId = getPayloadValue("question_id");
                QString questionText = getPayloadValue("question_text");
                int timeLimit = getPayloadValue("time_limit").toInt();
                
                qDebug() << "[CLIENT] Next question:" << questionNum;
                emit nextQuestionReceived(questionNum, questionId, questionText, timeLimit);
            }
            // Check if game ended
            else if (getPayloadValue("game_ended") == "true") {
                QString rankingsStr = getPayloadValue("rankings");
                QString winnerId = getPayloadValue("winner_id");
                
                // Parse rankings: "userId1:score1,userId2:score2,userId3:score3"
                QList<QPair<QString, int>> rankings;
                QStringList entries = rankingsStr.split(",", Qt::SkipEmptyParts);
                for (const QString &entry : entries) {
                    QStringList parts = entry.split(":");
                    if (parts.size() == 2) {
                        QString userId = parts[0];
                        int score = parts[1].toInt();
                        rankings.append({userId, score});
                    }
                }
                
                qDebug() << "[CLIENT] Game ended! Winner:" << winnerId;
                emit gameEnded(rankings, winnerId);
            }
        }
    }
}

// HÀM GỬI YÊU CẦU TÌM KIẾM
void GameClient::sendSearchRequest(const QString &keyword) {
    if (!isConnected()) return;
    QString payload = QString("keyword=%1").arg(keyword);
    sendMessage(CMD_SEARCH_REQ, payload);
}

// HÀM GỬI YÊU CẦU KẾT BẠN
void GameClient::sendAddFriendRequest(const QString &username) {
    if (!isConnected()) return;
    qDebug() << "[Client] Đang gửi lời mời kết bạn tới:" << username;
    QString payload = QString("target_username=%1").arg(username);
    sendMessage(CMD_ADD_FRIEND_REQ, payload);
}

void GameClient::sendGetPendingRequests() {
    sendMessage(CMD_GET_PENDING_REQ, "");
}

void GameClient::sendAcceptFriend(QString targetUsername) {
    sendMessage(CMD_ACCEPT_FRIEND_REQ, "target_username=" + targetUsername);
}

void GameClient::sendGetFriendList() {
    if (!isConnected()) return;
    qDebug() << "[CLIENT] Đang yêu cầu danh sách bạn bè từ Server...";
    sendMessage(CMD_GET_FRIEND_LIST, "");
}

void GameClient::sendJoinRoom(int roomId) {
    if (!isConnected()) return;
    QString payload = QString("room_id=%1;user_id=%2")
                      .arg(roomId)
                      .arg(m_currentUserID);
                      
    sendMessage(CMD_JOIN_ROOM_REQ, payload);
}

void GameClient::sendCreateRoomRequest() {
    if (!isConnected()) return;
    QString payload = QString("user_id=%1").arg(m_currentUserID);
    sendMessage(CMD_CREATE_ROOM, payload); 
}

void GameClient::sendGetRoomInfo() {
    if (!isConnected()) return;
    QString payload = QString("user_id=%1").arg(m_currentUserID);
    sendMessage(CMD_GET_ROOM_INFO, payload); 
}

void GameClient::sendLeaveRoom() {
    qDebug() << "[CLIENT] Dang gui lenh LEAVE_ROOM_REQ...";
    if (isConnected()) {
        sendMessage(CMD_LEAVE_ROOM_REQ, ""); 
        m_socket->flush(); 
    }
}

void GameClient::sendFindMatch() {
    if (isConnected()) {
        qDebug() << "[CLIENT] Dang tim tran Rank...";
        sendMessage(CMD_FIND_MATCH, QString("user_id=%1").arg(m_currentUserID)); 
        m_socket->flush();
    }
}

void GameClient::sendCancelMatch() {
    if (isConnected()) {
        qDebug() << "[CLIENT] Huy tim tran.";
        sendMessage(CMD_CANCEL_MATCH_REQ, "");
    }
}

void GameClient::sendGetLeaderboardRequest() {
    sendMessage(CMD_GET_LEADERBOARD, "");
}

void GameClient::sendStartGame(int roomId) {
    if (!isConnected()) return;
    qDebug() << "[CLIENT] Host dang bat dau tran dau cho Room ID:" << roomId;
    QString payload = QString("room_id=%1").arg(roomId);
    sendMessage(CMD_CREATE_MATCH, payload); 
}
void GameClient::sendAnswer(int matchId, const QString &answer, int timeElapsed) {
    if (!isConnected()) return;
    qDebug() << "[CLIENT] Sending answer:" << answer << "for match" << matchId;
    QString payload = QString("match_id=%1;user_id=%2;answer=%3;time_elapsed=%4")
                      .arg(matchId)
                      .arg(m_currentUserID)
                      .arg(answer)
                      .arg(timeElapsed);
    // Server Dispatcher expects ROUND1_ANSWER for gameplay answers
    sendMessage("ROUND1_ANSWER", payload);
}

void GameClient::sendChangePassword(const QString &oldPass, const QString &newPass) {
    if (!isConnected()) return;
    
    // --- CẬP NHẬT TRẠNG THÁI CUỐI CÙNG ---
    m_lastCommand = CMD_CHANGE_PASS; 
    // -------------------------------------

    // Server yêu cầu các params: user_id, old_password, new_password
    QString payload = QString("user_id=%1;old_password=%2;new_password=%3")
                      .arg(m_currentUserID)
                      .arg(oldPass)
                      .arg(newPass);
                      
    sendMessage(CMD_CHANGE_PASS, payload);
}