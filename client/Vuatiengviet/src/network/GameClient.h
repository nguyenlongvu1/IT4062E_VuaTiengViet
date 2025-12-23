#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QList> // Thêm thư viện này

// --- 1. ĐỊNH NGHĨA CẤU TRÚC DỮ LIỆU TÌM KIẾM ---
// Struct này dùng chung cho cả UI và Logic
struct UserSearchResult {
    QString username;
    QString status; // "Online", "Offline", "InGame"
    bool isFriend;  // true = Đã kết bạn, false = Chưa
};
struct RankItem {
    QString name;
    int score;
    QString rank;
};
// ----------------------------------------------

class GameClient : public QObject {
    Q_OBJECT
public:
    static GameClient& instance() {
        static GameClient _instance;
        return _instance;
    }
    QString getCurrentUserID() const { return m_currentUserID; }

    void connectToServer(const QString &ip, quint16 port);
    void sendLogin(const QString &username, const QString &password);
    void sendRegister(const QString &username, const QString &password);
    bool isConnected() const;
    void sendLogout();

    // --- 2. THÊM HÀM GỬI YÊU CẦU ---
    void sendSearchRequest(const QString &keyword);      // Tìm kiếm bạn bè
    void sendAddFriendRequest(const QString &username);  // Gửi lời mời kết bạn
    // -------------------------------
    void sendGetPendingRequests(); 
    void sendAcceptFriend(QString targetUsername);

    void sendGetFriendList(); 
    void sendJoinRoom(int roomId);
    void sendCreateRoomRequest();
    void sendGetRoomInfo();
    void sendLeaveRoom();

    void sendFindMatch();
    void sendCancelMatch();

    void sendGetLeaderboardRequest();

    void sendStartGame(int roomId);
  

signals:
    void connected();
    void disconnected();
    void loginSuccess();
    void loginFailed(QString msg, QString code);
    void registerSuccess();
    void registerFailed(QString mssg, QString code);

    // --- 3. THÊM TÍN HIỆU NHẬN KẾT QUẢ ---
    void searchResultReceived(const QList<UserSearchResult> &results);
    // -------------------------------------
    void friendRequestReceived(QString senderName); // Có người mời
    void friendRequestAccepted(QString friendName); // Người ta đã đồng ý
    void pendingListReceived(QStringList users);

    void friendListUpdated(); // Tín hiệu báo hiệu cần load lại danh sách
    void friendListReceived(const QList<UserSearchResult>& friends);

    void joinRoomResult(bool success, QString reason);
    void roomUpdated();
    void roomJoined(const QString &roomId);

    void roomInfoReceived(const QString &p1, const QString &p2, const QString &p3);
    
    void matchFound(const QString& roomId);
    void matchStartedDirectly(QString matchId, QString roomId);
    void userInfoReceived(const QString& username, int points, const QString& rankName);
    void leaderboardReceived(const QList<RankItem> &items);
    
    

private slots:
    void onReadyRead();

private:
    GameClient();
    QTcpSocket *m_socket;
    void sendMessage(const QString &cmd, const QString &payload);
    QString m_lastCommand;
    QString m_currentUserID;
};

#endif // GAMECLIENT_H