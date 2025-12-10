#include "GameClient.h"
#include "Protocol.h" // Include file định nghĩa lệnh

GameClient::GameClient() {
    m_socket = new QTcpSocket(this);
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

    // SỬA ĐỔI: Thêm "\n\n" vào trước payload (%3)
    // Cấu trúc server mong đợi:
    // COMMAND: TEN_LENH
    // (Dòng trống để báo hiệu hết header)
    // payload...
    
    QString msg = QString("COMMAND: %1\nLENGTH: %2\n\n%3") 
                  .arg(command)
                  .arg(payload.toUtf8().length())
                  .arg(payload);
                  
    m_socket->write(msg.toUtf8());
    m_socket->flush();
}

void GameClient::sendLogin(const QString &u, const QString &p) {
    QString payload = QString("username=%1;password=%2").arg(u, p);
    sendMessage(CMD_LOGIN, payload);
}

void GameClient::sendRegister(const QString &u, const QString &p) {
    QString payload = QString("username=%1;password=%2").arg(u, p);
    sendMessage(CMD_REGISTER, payload);
}

void GameClient::onReadyRead() {
    // Đọc phản hồi từ Server
    QByteArray data = m_socket->readAll();
    QString response = QString::fromUtf8(data);

     qDebug() << "==================================";
    qDebug() << "[CLIENT DEBUG] Raw Data received:" << response;
    qDebug() << "==================================";

    // Parse đơn giản (Cần nâng cấp Parser ở giai đoạn sau)
    if (response.contains(CMD_LOGIN_OK)) {
        emit loginSuccess();
    } else if (response.contains(CMD_LOGIN_FAIL)) {
        emit loginFailed("Sai tên đăng nhập hoặc mật khẩu!");
    } else if (response.contains(CMD_REG_OK)) {
        emit registerSuccess();
    } else if (response.contains(CMD_REG_FAIL)) {
        emit registerFailed("Tên tài khoản đã tồn tại!");
    }
}
