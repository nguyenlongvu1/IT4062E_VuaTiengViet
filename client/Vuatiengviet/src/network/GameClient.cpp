#include "GameClient.h"
#include "Protocol.h"
// #include <QTimer> // XÓA: Không dùng giả lập nữa

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

void GameClient::sendMessage(const QString &command, const QString &payload) {
    if (!isConnected()) return;

    QString msg = QString("COMMAND: %1\nLENGTH: %2\n\n%3") 
                  .arg(command)
                  .arg(payload.toUtf8().length())
                  .arg(payload);
                  
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
    sendMessage("LOGOUT", payload); 
    m_currentUserID.clear();
}

// XỬ LÝ DỮ LIỆU NHẬN TỪ SERVER (QUAN TRỌNG - ĐÃ SỬA DÍNH GÓI TIN)
void GameClient::onReadyRead() {
    QByteArray data = m_socket->readAll();
    QString rawData = QString::fromUtf8(data);

    qDebug() << "[CLIENT DEBUG] Raw Data received:" << rawData;

    // 1. Tách các gói tin dựa trên từ khóa "COMMAND: "
    QStringList packets = rawData.split("COMMAND: ", Qt::SkipEmptyParts);

    for (const QString &packet : packets) {
        QString response = "COMMAND: " + packet;
        
        // 2. Tách Payload 
        QString payload = "";
        int splitIndex = response.indexOf("\n\n");
        if (splitIndex != -1) payload = response.mid(splitIndex + 2);

        // Helper lấy giá trị payload (Local Lambda)
        auto getPayloadValue = [&](QString key) -> QString {
            QString search = key + "=";
            int start = payload.indexOf(search);
            if (start == -1) return "";
            int end = payload.indexOf(";", start);
            if (end == -1) end = payload.length();
            return payload.mid(start + search.length(), end - (start + search.length())).trimmed();
        };

        // --- CASE 1: ĐĂNG NHẬP / ĐĂNG KÝ ---
        if (response.contains(CMD_LOGIN_OK)) {
           m_currentUserID = getPayloadValue("user_id");
    
            // Lấy thông tin hiển thị (Rank, Điểm, Tên) 
            QString username = getPayloadValue("username");
            QString rankName = getPayloadValue("rank_name"); 
            int points = getPayloadValue("points").toInt();

            qDebug() << "[INFO] Logged in:" << username << "| Rank:" << rankName << "| Points:" << points;

            // Phát tín hiệu gửi dữ liệu sang HomeWidget
            emit userInfoReceived(username, points, rankName);

            // Báo đăng nhập thành công 
            emit loginSuccess();

            // lấy danh sách bạn bè
            sendGetFriendList();
            sendGetPendingRequests();
        } 
        else if (response.contains(CMD_REG_OK)) {
            emit registerSuccess();
        } 
        else if (response.contains(CMD_LOGIN_FAIL)) {
            emit loginFailed(getPayloadValue("error_msg"), getPayloadValue("error_code"));
        }
        else if (response.contains(CMD_REG_FAIL)) {
            emit registerFailed(getPayloadValue("error_msg"), getPayloadValue("error_code"));
        }

        // --- CASE 2: NHẬN KẾT QUẢ TÌM KIẾM ---
        else if (response.contains("COMMAND: SEARCH_RES")) {
    QList<UserSearchResult> results;
    
    // Lấy toàn bộ chuỗi: "u1,On|u2,Off|u3,On"
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

        // --- CASE 3: KẾT BẠN (CHẤP NHẬN / THÔNG BÁO) ---
        else if (response.contains("COMMAND: ACCEPT_FRIEND_RES")) {
            QString status = getPayloadValue("status");
            if (status == "success") {
                QString target = getPayloadValue("target");
                qDebug() << "[SUCCESS] Đã chấp nhận kết bạn với:" << target;
                emit friendListUpdated(); 
            }
        }
        else if (response.contains("COMMAND: NOTIFY_FRIEND_ACCEPTED")) {
            QString friendName = getPayloadValue("friend_username");
            qDebug() << "[NOTIFY] Chúc mừng! Bạn và " << friendName << " đã trở thành bạn bè.";
            emit friendListUpdated();
        }

       // --- CASE 4: DANH SÁCH BẠN BÈ ---
else if (response.contains("COMMAND: FRIEND_LIST_RES")) {
    QList<UserSearchResult> friends;
    
    // Lấy chính xác chuỗi dữ liệu từ key "friends" hoặc "players"
    QString listStr = getPayloadValue("friends");
    if (listStr.isEmpty()) {
        listStr = getPayloadValue("players");
    }

    // Tách chuỗi clean (ví dụ: "tuphan,Online;nam,Offline")
    QStringList entries = listStr.split("|", Qt::SkipEmptyParts);
    
    for (const QString& entry : entries) {
        QStringList parts = entry.split(",");
        
        if (parts.size() >= 2) {
            UserSearchResult f;
            f.username = parts[0].trimmed(); // Tên: tuphan
            f.status = parts[1].trimmed();   // Trạng thái: Online/Offline
            f.isFriend = true; 
            
            // Loại bỏ chính mình (nếu lỡ Server gửi về) và tên rỗng
            if (!f.username.isEmpty() && f.username != m_currentUserID) {
                friends.append(f);
            }
        }
    }
    emit friendListReceived(friends);
}
else if (response.contains("COMMAND: UPDATE_STATUS_NOTIFY")) {
             QString username = getPayloadValue("username");
             QString status = getPayloadValue("status"); 
             
             qDebug() << "[NOTIFY] Friend Status Update:" << username << " -> " << status;
             
             // Phát tín hiệu sang UI
             emit friendStatusChanged(username, status);
        }

        // --- CASE 5: LỜI MỜI KẾT BẠN & PENDING ---
        else if (response.contains("COMMAND: NOTIFY_FRIEND_REQ")) {
            QString sender = getPayloadValue("sender_username");
            qDebug() << "[NOTIFY] Có lời mời kết bạn từ:" << sender;
            emit friendRequestReceived(sender);
        }
        else if (response.contains("COMMAND: GET_PENDING_RES")) {
            QString listRaw = getPayloadValue("request_list");
            QStringList list = listRaw.split(",", Qt::SkipEmptyParts);
            emit pendingListReceived(list);
        }

        // --- CASE 6: XỬ LÝ PHÒNG (ROOM) ---

        // NHẬN THÔNG TIN CHI TIẾT PHÒNG 
        else if (response.contains("COMMAND: ROOM_INFO_RES")) {
            QString p1 = getPayloadValue("p1"); //host
            QString p2 = getPayloadValue("p2"); 
            QString p3 = getPayloadValue("p3"); 
            
            qDebug() << "[NET] Room Data from DB: P1=" << p1 << " P2=" << p2 << " P3=" << p3;
            emit roomInfoReceived(p1, p2, p3);
        }

        // Tham gia phòng (JOIN_ROOM_RES) -> Là Khách (Guest)
        else if (response.contains("COMMAND: JOIN_ROOM_RES")) {
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

        // Tạo phòng thành công (ROOM_CREATED) -> Là Chủ phòng (Host)
        else if (response.contains("COMMAND: ROOM_CREATED")) {
            QString roomId = getPayloadValue("room_id");
            emit roomJoined(roomId); 
            
            sendGetRoomInfo();
        }

        // Có người vào phòng (PLAYER_JOINED_NOTIFY) -> Host nhận tin này
        else if (response.contains("COMMAND: PLAYER_JOINED_NOTIFY")) {
            emit roomUpdated(); 
            
            sendGetRoomInfo();
        }
        else if (response.contains("COMMAND: MATCH_FOUND_NOTIFY")) {
            QString roomId = getPayloadValue("room_id");
            qDebug() << "[CLIENT] TIM THAY TRAN! Room ID:" << roomId;
            emit matchFound(roomId);
        }
        else if (response.contains("COMMAND: START_MATCH")) {
            QString matchId = getPayloadValue("match_id");
            QString roomId = getPayloadValue("room_id");
            
            qDebug() << "[NET] Ghép trận thành công! Vào game ngay với MatchID:" << matchId;
            
            // Phát tín hiệu để HomeWidget đóng Radar và mở màn hình thi đấu
            emit matchStartedDirectly(matchId, roomId); 
        }
        else if (response.contains("COMMAND: PLAYER_LEFT_NOTIFY")) {
            qDebug() << "[NOTIFY] Có người rời phòng -> Tải lại thông tin...";
            
            // Phát tín hiệu update (nếu cần)
            emit roomUpdated(); 
            
            // lấy danh sách mới nhất (đã trừ người kia ra)
            sendGetRoomInfo(); 
        }
        else if (response.contains("COMMAND: LEADERBOARD_RES")) {
            QList<RankItem> items;
            QString data = getPayloadValue("data");
            qDebug() << "[CLIENT DEBUG] Leaderboard Data RAW:" << data;
            
        
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
            qDebug() << "[CLIENT DEBUG] Parsed items count:" << items.size();
            emit leaderboardReceived(items);
        }
else if (response.contains("COMMAND: MATCH_CREATED")) {
            QString matchId = getPayloadValue("match_id");
            
            // Lấy room_id
            QString roomId = getPayloadValue("room_id"); 
            
            qDebug() << "[NET] Chủ phòng đã bắt đầu game! MatchID:" << matchId;
            emit matchStartedDirectly(matchId, roomId);
        }
    }
}

// HÀM GỬI YÊU CẦU TÌM KIẾM
void GameClient::sendSearchRequest(const QString &keyword) {
    if (!isConnected()) {
        qDebug() << "[CLIENT ERROR] Chưa kết nối Server, không thể tìm kiếm!";
        return;
    }
    qDebug() << "[Client] Đang gửi yêu cầu tìm kiếm lên Server:" << keyword;
    QString payload = QString("keyword=%1").arg(keyword);
    sendMessage("SEARCH_REQ", payload);
}

// HÀM GỬI YÊU CẦU KẾT BẠN
void GameClient::sendAddFriendRequest(const QString &username) {
    if (!isConnected()) return;
    qDebug() << "[Client] Đang gửi lời mời kết bạn tới:" << username;
    QString payload = QString("target_username=%1").arg(username);
    sendMessage("ADD_FRIEND_REQ", payload);
}

void GameClient::sendGetPendingRequests() {
    sendMessage("GET_PENDING_REQ", "");
}

void GameClient::sendAcceptFriend(QString targetUsername) {
    sendMessage("ACCEPT_FRIEND_REQ", "target_username=" + targetUsername);
}

void GameClient::sendGetFriendList() {
    if (!isConnected()) return;
    qDebug() << "[CLIENT] Đang yêu cầu danh sách bạn bè từ Server...";
    sendMessage("GET_FRIEND_LIST", "");
}

void GameClient::sendJoinRoom(int roomId) {
    if (!isConnected()) return;
    QString payload = QString("room_id=%1;user_id=%2")
                      .arg(roomId)
                      .arg(m_currentUserID);
                      
    sendMessage("JOIN_ROOM_REQ", payload);
}
void GameClient::sendCreateRoomRequest() {
    if (!isConnected()) return;
    QString payload = QString("user_id=%1").arg(m_currentUserID);
    sendMessage("CREATE_ROOM", payload); 
}
void GameClient::sendGetRoomInfo() {
    if (!isConnected()) return;
    QString payload = QString("user_id=%1").arg(m_currentUserID);
    sendMessage("GET_ROOM_INFO", payload); 
}
void GameClient::sendLeaveRoom() {
    qDebug() << "[CLIENT] Dang gui lenh LEAVE_ROOM_REQ...";
    if (isConnected()) {
        sendMessage("LEAVE_ROOM_REQ", ""); 
        m_socket->flush(); 
    }
}
void GameClient::sendFindMatch() {
    if (isConnected()) {
        qDebug() << "[CLIENT] Dang tim tran Rank...";
        sendMessage("FIND_MATCH", QString("user_id=%1").arg(m_currentUserID)); 
        m_socket->flush();
    }
}

void GameClient::sendCancelMatch() {
    if (isConnected()) {
        qDebug() << "[CLIENT] Huy tim tran.";
        sendMessage("CANCEL_MATCH_REQ", "");
    }
}
void GameClient::sendGetLeaderboardRequest() {
    sendMessage("GET_LEADERBOARD", "");
}
void GameClient::sendStartGame(int roomId) {
    if (!isConnected()) return;
    qDebug() << "[CLIENT] Host dang bat dau tran dau cho Room ID:" << roomId;
    QString payload = QString("room_id=%1").arg(roomId);
    sendMessage("CREATE_MATCH", payload); 
};
