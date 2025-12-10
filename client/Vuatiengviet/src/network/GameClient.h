#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QObject>
#include <QTcpSocket>

class GameClient : public QObject {
    Q_OBJECT
public:
    // Singleton Pattern: Truy cập mọi nơi qua GameClient::instance()
    static GameClient& instance() {
        static GameClient _instance;
        return _instance;
    }

    void connectToServer(const QString &ip, quint16 port);
    void sendLogin(const QString &username, const QString &password);
    void sendRegister(const QString &username, const QString &password);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void loginSuccess();
    void loginFailed(QString reason);
    void registerSuccess();
    void registerFailed(QString reason);

private slots:
    void onReadyRead(); // Xử lý dữ liệu đến

private:
    GameClient(); // Private Constructor
    QTcpSocket *m_socket;
    void sendMessage(const QString &cmd, const QString &payload);
};

#endif // GAMECLIENT_H