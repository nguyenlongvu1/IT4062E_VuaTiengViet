#include "GameClient.h"
#include "Protocol.h"
#include <QDebug>

// ============================================================================
// CONSTRUCTOR & KẾT NỐI
// ============================================================================
GameClient::GameClient() {
    m_socket = new QTcpSocket(this);
    m_lastCommand = "";
    m_buffer = "";             // Init
    m_lastQuestionNum = "";
    // connect(m_socket, &QTcpSocket::connected, this, &GameClient::connected);
    // connect(m_socket, &QTcpSocket::disconnected, this, &GameClient::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &GameClient::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &GameClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &GameClient::onSocketDisconnected);
    // Xử lý lỗi socket
    connect(m_socket, &QTcpSocket::errorOccurred, this, 
        [=](QAbstractSocket::SocketError socketError){
            qDebug() << "[NET ERROR]" << m_socket->errorString() << "Code:" << socketError;
        });
}

GameClient::~GameClient() {
    if (m_socket->isOpen()) {
        m_socket->close();
    }
}
void GameClient::connectToServer(const QString &ip, quint16 port) {
    if(m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->connectToHost(ip, port);
    }
}

bool GameClient::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void GameClient::onSocketConnected() {
    qDebug() << "[NET] Connected to Server!";
    // Sau khi xử lý xong, bắn signal ra ngoài cho UI biết
    emit connected(); 
}

// CŨ (Gây lỗi): void GameClient::disconnected() { ... }
// MỚI (Đúng):
void GameClient::onSocketDisconnected() {
    qDebug() << "[NET] Disconnected from Server!";
    // Bắn signal ra ngoài
    emit disconnected();
}

// ============================================================================
// HÀM GỬI DỮ LIỆU (SEND)
// ============================================================================

static QString buildTLVPayloadFromKv(const QString &payloadKv) {
    QStringList tokens = payloadKv.split(";", Qt::SkipEmptyParts);
    QStringList out;
    for (const QString &t : tokens) {
        int eq = t.indexOf('=');
        if (eq == -1) { out << t.trimmed(); continue; }
        QString k = t.left(eq).trimmed();
        QString v = t.mid(eq + 1).trimmed();
        out << QString("%1|%2|%3").arg(k).arg(v.toUtf8().size()).arg(v);
    }
    return out.join(";");
}

void GameClient::sendMessage(const QString &command, const QString &payload) {
    if (!isConnected()) return;
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
    sendMessage(CMD_LOGIN, QString("username=%1;password=%2").arg(u, p));
}

void GameClient::sendRegister(const QString &u, const QString &p) {
    m_lastCommand = CMD_REGISTER;
    sendMessage(CMD_REGISTER, QString("username=%1;password=%2").arg(u, p));
}

void GameClient::sendLogout() {
    if (!m_currentUserID.isEmpty()) sendMessage(CMD_LOGOUT, "user_id=" + m_currentUserID); 
    m_currentUserID.clear();
}

void GameClient::sendSearchRequest(const QString &keyword) {
    sendMessage(CMD_SEARCH_REQ, "keyword=" + keyword);
}

void GameClient::sendAddFriendRequest(const QString &username) {
    sendMessage(CMD_ADD_FRIEND_REQ, "target_username=" + username);
}

void GameClient::sendGetPendingRequests() { sendMessage(CMD_GET_PENDING_REQ, ""); }
void GameClient::sendAcceptFriend(QString targetUsername) { sendMessage(CMD_ACCEPT_FRIEND_REQ, "target_username=" + targetUsername); }
void GameClient::sendGetFriendList() { sendMessage(CMD_GET_FRIEND_LIST, ""); }

void GameClient::sendJoinRoom(int roomId) {
    sendMessage(CMD_JOIN_ROOM_REQ, QString("room_id=%1;user_id=%2").arg(roomId).arg(m_currentUserID));
}

void GameClient::sendCreateRoomRequest() { sendMessage(CMD_CREATE_ROOM, "user_id=" + m_currentUserID); }
void GameClient::sendGetRoomInfo() { sendMessage(CMD_GET_ROOM_INFO, "user_id=" + m_currentUserID); }
void GameClient::sendLeaveRoom() { sendMessage(CMD_LEAVE_ROOM_REQ, ""); }
void GameClient::sendFindMatch() { 
    m_lastQuestionNum = "";
    sendMessage(CMD_FIND_MATCH, "user_id=" + m_currentUserID); 
}
void GameClient::sendCancelMatch() { sendMessage(CMD_CANCEL_MATCH_REQ, ""); }
void GameClient::sendGetLeaderboardRequest() { sendMessage(CMD_GET_LEADERBOARD, ""); }
void GameClient::sendStartGame(int roomId) { 
    m_lastQuestionNum = "";
    sendMessage(CMD_CREATE_MATCH, QString("room_id=%1").arg(roomId)); }

void GameClient::sendAnswer(int matchId, const QString &answer, int timeElapsed) {
    sendMessage("ROUND1_ANSWER", QString("match_id=%1;user_id=%2;answer=%3;time_elapsed=%4")
                .arg(matchId).arg(m_currentUserID).arg(answer).arg(timeElapsed));
}

void GameClient::sendChangePassword(const QString &oldPass, const QString &newPass) {
    m_lastCommand = CMD_CHANGE_PASS; 
    sendMessage(CMD_CHANGE_PASS, QString("user_id=%1;old_password=%2;new_password=%3").arg(m_currentUserID, oldPass, newPass));
}

// ============================================================================
// HÀM XỬ LÝ DỮ LIỆU NHẬN TỪ SERVER (RECEIVE) - ĐÃ FIX LỖI UTF-8
// ============================================================================
void GameClient::onReadyRead() {
    // static QString m_buffer; // Bộ đệm lưu dữ liệu bị cắt dở
    QByteArray data = m_socket->readAll();
    m_buffer.append(QString::fromUtf8(data));

    while (true) {
        int cmdIndex = m_buffer.indexOf("COMMAND: ");
        
        // Nếu không tìm thấy lệnh nào -> Đợi dữ liệu tiếp
        if (cmdIndex == -1) {
            if (m_buffer.length() > 50000) m_buffer.clear(); // Chống tràn
            break; 
        }

        // Bỏ phần rác ở đầu
        if (cmdIndex > 0) { m_buffer = m_buffer.mid(cmdIndex); cmdIndex = 0; }

        // Tìm điểm kết thúc của gói tin (là COMMAND tiếp theo hoặc hết buffer)
        int nextCmdIndex = m_buffer.indexOf("COMMAND: ", 9);
        QString packet;
        
        if (nextCmdIndex == -1) {
            packet = m_buffer; 
            m_buffer.clear();
        } else {
            packet = m_buffer.left(nextCmdIndex);
            m_buffer = m_buffer.mid(nextCmdIndex);
        }

        // --- BẮT ĐẦU PARSE GÓI TIN ---
        QString response = packet;
        QString payload = "";
        int splitIndex = response.indexOf("\n\n");
        if (splitIndex != -1) payload = response.mid(splitIndex + 2);

        QMap<QString, QString> payloadMap;
        QStringList tokens = payload.split(";", Qt::SkipEmptyParts);
        for (const QString &t : tokens) {
            int p1 = t.indexOf('|');
            int p2 = (p1 == -1) ? -1 : t.indexOf('|', p1 + 1);
            if (p1 != -1 && p2 != -1) {
                QString k = t.left(p1).trimmed();
                // FIX: Lấy hết phần còn lại, bỏ qua check length để tránh lỗi UTF-8
                QString v = t.mid(p2 + 1); 
                payloadMap[k] = v;
            } else {
                int eq = t.indexOf('=');
                if (eq != -1) payloadMap[t.left(eq).trimmed()] = t.mid(eq + 1).trimmed();
            }
        }
        auto getPayloadValue = [&](QString key) -> QString { return payloadMap.value(key); };

        qDebug() << "[CLIENT PACKET]" << (response.left(60) + "...");

        // --- XỬ LÝ LOGIC ---

        // 1. GAME QUESTION
        if (response.contains("COMMAND: GAME_QUESTION")) {
            int matchId = getPayloadValue("match_id").toInt();
            QString qNum = getPayloadValue("question_num");
            
            // static QString lastQNum = "";
            if (qNum == m_lastQuestionNum && !qNum.isEmpty()) continue; // Chặn trùng
            m_lastQuestionNum = qNum; // Cập nhật

            emit gameQuestionReceived(matchId, qNum, 
                                      getPayloadValue("question_id"), 
                                      getPayloadValue("question_text"), 
                                      getPayloadValue("options"), 
                                      getPayloadValue("round_id").toInt(), 
                                      getPayloadValue("time_limit").toInt());
            QString scoresStr = getPayloadValue("scores");
            if (!scoresStr.isEmpty()) {
                // SỬA 1: Đổi từ int sang QString
                QList<QPair<QString, QString>> scores; 
                
                QStringList entries = scoresStr.split(",", Qt::SkipEmptyParts);
                for (const QString &entry : entries) {
                    QStringList parts = entry.split(":");
                    if (parts.size() == 2) {
                        // SỬA 2: Bỏ .toInt() đi, giữ nguyên là String
                        scores.append({parts[0], parts[1]}); 
                    }
                }
                emit gameScoresUpdated(scores); 
            }
        }
        
        // 2. ANSWER RESULT (Quan trọng: Xử lý next_question)
        else if (response.contains("COMMAND: ANSWER_RESULT")) {
            bool correct = (getPayloadValue("correct") == "true");
            int pts = getPayloadValue("points_earned").toInt();
            int total = getPayloadValue("total_score").toInt();
            emit answerResultReceived(correct, pts, total);

            if (getPayloadValue("next_question") == "true") {
                int matchId = getPayloadValue("match_id").toInt();
                QString qNum = getPayloadValue("question_num");

                m_lastQuestionNum = qNum;
                
                // Update lastQNum để không bị chặn
                static QString lastQNum = ""; 
                lastQNum = qNum; 

                QString opts = getPayloadValue("options");
                int rId = getPayloadValue("round_id").toInt();
                
                qDebug() << "[CLIENT] Next Question Found:" << qNum;
                emit nextQuestionReceived(matchId, qNum, 
                                          getPayloadValue("question_id"), 
                                          getPayloadValue("question_text"), 
                                          opts, rId, 
                                          getPayloadValue("time_limit").toInt());
            }
            else if (getPayloadValue("game_ended") == "true") {
                QString rankingsStr = getPayloadValue("rankings");
                QList<QPair<QString, int>> rankings;
                QStringList entries = rankingsStr.split(",", Qt::SkipEmptyParts);
                for (const QString &entry : entries) {
                    QStringList parts = entry.split(":");
                    if (parts.size() == 2) rankings.append({parts[0], parts[1].toInt()});
                }
                emit gameEnded(rankings, getPayloadValue("winner_id"));
            }
            // Update Score Realtime
            QString scoresStr = getPayloadValue("scores");
            if (!scoresStr.isEmpty()) {
            QList<QPair<QString, QString>> scores; // SỬA: QString, QString
            QStringList entries = scoresStr.split(",", Qt::SkipEmptyParts);
            for (const QString &entry : entries) {
                QStringList parts = entry.split(":");
                if (parts.size() == 2) {
                    // SỬA: Giữ nguyên parts[1] là string, không toInt()
                    scores.append({parts[0], parts[1]}); 
                }
            }
            emit gameScoresUpdated(scores);
        }
        }

        // 3. LOGIN & REGISTER
        else if (response.contains(CMD_LOGIN_FAIL) || response.contains("FAIL")) {
            QString code = getPayloadValue("error_code");
            QString msg = getPayloadValue("error_msg");
            if (m_lastCommand == CMD_LOGIN) emit loginFailed(msg, code);
            else if (m_lastCommand == CMD_REGISTER) emit registerFailed(msg, code);
            else if (m_lastCommand == CMD_CHANGE_PASS) emit changePasswordFailed(msg);
        }
        else if (response.contains(CMD_LOGIN_OK)) {
            m_currentUserID = getPayloadValue("user_id");
            emit userInfoReceived(getPayloadValue("username"), getPayloadValue("points").toInt(), getPayloadValue("rank_name"));
            emit loginSuccess();
            sendGetFriendList();
            sendGetPendingRequests();
        }
        else if (response.contains(CMD_REG_OK)) { emit registerSuccess(); }
        else if (response.contains(CMD_CHANGE_PASS_OK)) { emit changePasswordSuccess(); }

        // 4. ROOM & MATCHMAKING
        else if (response.contains(CMD_ROOM_INFO_RES)) {
            emit roomInfoReceived(getPayloadValue("p1"), getPayloadValue("p2"), getPayloadValue("p3"));
        }
        else if (response.contains(CMD_JOIN_ROOM_RES)) {
            if (getPayloadValue("status") == "success") {
                emit joinRoomResult(true, "");
                emit roomJoined(getPayloadValue("room_id"));
                sendGetRoomInfo();
            } else {
                emit joinRoomResult(false, getPayloadValue("reason"));
            }
        }
        else if (response.contains(CMD_ROOM_CREATED)) {
            emit roomJoined(getPayloadValue("room_id"));
            sendGetRoomInfo();
        }
        else if (response.contains(CMD_PLAYER_JOINED_NOTIFY) || response.contains(CMD_PLAYER_LEFT_NOTIFY)) {
            emit roomUpdated();
            sendGetRoomInfo();
        }
        else if (response.contains(CMD_MATCH_CREATED)) {
            QString mId = getPayloadValue("match_id");
            QString rId = getPayloadValue("room_id");
            QString pls = getPayloadValue("players");
            sendMessage("START_MATCH", QString("match_id=%1;room_id=%2;players=%3").arg(mId, rId, pls));
            emit matchStartedDirectly(mId, rId);
        }
        else if (response.contains(CMD_START_MATCH)) {
    QString mIdStr = getPayloadValue("match_id");
    QString rId = getPayloadValue("room_id");

    // LOG KIỂM TRA:
    qDebug() << "[NET] Nhận START_MATCH. Raw MatchID:" << mIdStr;

    // Nếu Server gửi "AUTO_GEN_ID", đây là lỗi phía Server. 
    // Nhưng để Client chạy được, ta sẽ gán tạm nếu nó không phải số.
    bool ok;
    mIdStr.toInt(&ok);
    if (!ok) {
        qDebug() << "[WARNING] Server gửi MatchID không hợp lệ, đang dùng ID tạm thời!";
    }

    emit matchStartedDirectly(mIdStr, rId); 
    emit matchFound(rId); 
}
        // 5. FRIEND & SEARCH
        else if (response.contains(CMD_SEARCH_RES)) {
            QList<UserSearchResult> results;
            QStringList users = getPayloadValue("users").split("|", Qt::SkipEmptyParts);
            for (const QString& uStr : users) {
                QStringList parts = uStr.split(",");
                if (parts.size() >= 2) {
                    UserSearchResult item;
                    item.username = parts[0].trimmed();
                    item.status = parts[1].trimmed();
                    item.isFriend = false;
                    if (item.username != m_currentUserID) results.append(item);
                }
            }
            emit searchResultReceived(results);
        }
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
                    if (f.username != m_currentUserID) friends.append(f);
                }
            }
            emit friendListReceived(friends);
        }
        else if (response.contains(CMD_NOTIFY_FRIEND_REQ)) {
            emit friendRequestReceived(getPayloadValue("sender_username"));
        }
        else if (response.contains(CMD_GET_PENDING_RES)) {
            emit pendingListReceived(getPayloadValue("request_list").split(",", Qt::SkipEmptyParts));
        }
        else if (response.contains(CMD_ACCEPT_FRIEND_RES) || response.contains(CMD_NOTIFY_FRIEND_ACCEPTED)) {
            emit friendListUpdated();
        }
        else if (response.contains(CMD_UPDATE_STATUS_NOTIFY)) {
            emit friendStatusChanged(getPayloadValue("username"), getPayloadValue("status"));
        }
        else if (response.contains("COMMAND: HISTORY_DATA")) {
            QString data = getPayloadValue("data");
            emit historyReceived(data);
        }
        // 6. LEADERBOARD
        else if (response.contains(CMD_LEADERBOARD_RES)) {
            QList<RankItem> items;
            QStringList entries = getPayloadValue("data").split("|", Qt::SkipEmptyParts);
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
        else if (response.contains("COMMAND: ELIMINATED")) {
            qDebug() << "[CLIENT] Nhận lệnh ELIMINATED -> Bị loại!";
            emit playerEliminated(); // Bắn tín hiệu sang MainWindow
        }
        else if (response.contains("COMMAND: MATCH_LOG_DATA")) {
    // Nếu bạn dùng hàm split hoặc regex để lấy data, hãy cẩn thận với tiền tố độ dài
    QString fullData = getPayloadValue("data"); 
    
    // Nếu fullData đang là "387|1|kết cục...", chúng ta cần bỏ số 387 và dấu | đầu tiên
    if (fullData.contains("|")) {
        fullData = fullData.mid(fullData.indexOf("|") + 1);
    }
    
    emit matchLogReceived(fullData);
}
    }
}
void GameClient::sendSurrender(int matchId) {
    if (!isConnected()) return;
    qDebug() << "[CLIENT] Gửi lệnh đầu hàng cho Match:" << matchId;
    
    // Gửi lệnh SURRENDER_MATCH kèm match_id và user_id
    QString payload = QString("match_id=%1;user_id=%2")
                      .arg(matchId)
                      .arg(m_currentUserID);
                      
    sendMessage("SURRENDER_MATCH", payload);
}
void GameClient::sendGetHistory() {
    sendMessage("GET_HISTORY", "user_id=" + m_currentUserID);
}
void GameClient::sendGetMatchLog(int matchId) {
    if (!isConnected()) return;
    
    // THÊM user_id vào payload để Server biết lấy log của ai
    QString payload = QString("match_id=%1;user_id=%2")
                      .arg(matchId)
                      .arg(m_currentUserID);
                      
    sendMessage("GET_MATCH_LOG", payload);
}