#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QList> 

// --- 1. ĐỊNH NGHĨA CẤU TRÚC DỮ LIỆU TÌM KIẾM ---
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


class GameClient : public QObject {
    Q_OBJECT
public:
    static GameClient& instance() {
        static GameClient _instance;
        return _instance;
    }
    ~GameClient();
    QString getCurrentUserID() const { return m_currentUserID; }

    void connectToServer(const QString &ip, quint16 port);
    void sendLogin(const QString &username, const QString &password);
    void sendRegister(const QString &username, const QString &password);
    bool isConnected() const;
    void sendLogout();

   
    void sendSearchRequest(const QString &keyword);     
    void sendAddFriendRequest(const QString &username);  
    
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
    void sendChangePassword(const QString &oldPass, const QString &newPass);
    void sendAnswer(int matchId, const QString &answer, int timeElapsed);
    void sendSurrender(int matchId);
  

signals:
    void connected();
    void disconnected();
    void loginSuccess();
    void loginFailed(QString msg, QString code);
    void registerSuccess();
    void registerFailed(QString mssg, QString code);

    void searchResultReceived(const QList<UserSearchResult> &results);
    void friendRequestReceived(QString senderName); 
    void friendRequestAccepted(QString friendName); 
    void pendingListReceived(QStringList users);

    void friendListUpdated(); 
    void friendListReceived(const QList<UserSearchResult>& friends);

    void joinRoomResult(bool success, QString reason);
    void roomUpdated();
    void roomJoined(const QString &roomId);

    void roomInfoReceived(const QString &p1, const QString &p2, const QString &p3);
    
    void matchFound(const QString& roomId);
    void matchStartedDirectly(QString matchId, QString roomId);
    void userInfoReceived(const QString& username, int points, const QString& rankName);
    void leaderboardReceived(const QList<RankItem> &items);
   void friendStatusChanged(const QString& username, const QString& status);
   void changePasswordSuccess();
    void changePasswordFailed(const QString &msg);
     
    void answerResultReceived(bool correct, int pointsEarned, int totalScore);
   
    void gameScoresUpdated(QList<QPair<QString, int>> scores);
    void gameEnded(QList<QPair<QString, int>> rankings, QString winnerId);
   void gameQuestionReceived(int matchId, QString questionNum, QString questionId, QString questionText, QString options, int roundId, int timeLimit);
    // 2. Nhận câu hỏi tiếp theo (Đủ 6 tham số - không cần matchId)
    void nextQuestionReceived(int matchId, QString questionNum, QString questionId, QString questionText, QString options, int roundId, int timeLimit);
    void playerEliminated();
private slots:
    void onReadyRead();
    void onSocketConnected();    // Hàm xử lý nội bộ khi kết nối thành công
    void onSocketDisconnected();

private:
    GameClient();
    QTcpSocket *m_socket;
    void sendMessage(const QString &cmd, const QString &payload);
    QString m_lastCommand;
    QString m_currentUserID;
};

#endif // GAMECLIENT_H