#include "MatchmakingWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include "../../network/GameClient.h" // Đảm bảo đường dẫn này đúng

MatchmakingWidget::MatchmakingWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
    
    // Timer cập nhật text (...)
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &MatchmakingWidget::updateStatusText);
}

void MatchmakingWidget::setupUi() {
    this->setObjectName("MatchmakingScreen"); 
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);

    // 1. Radar Animation
    lblRadar = new QLabel(this);
    lblRadar->setObjectName("RadarLabel");
    lblRadar->setFixedSize(200, 200);
    lblRadar->setAlignment(Qt::AlignCenter);
    
    QPixmap pix(":/images/radar.png"); 
    if(!pix.isNull()) {
        lblRadar->setPixmap(pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        lblRadar->setText("RADAR");
        lblRadar->setStyleSheet("font-size: 20px; font-weight: bold; color: white; border: 2px solid white; border-radius: 100px;");
    }

    // 2. Trạng thái text
    lblStatus = new QLabel("Đang quét máy chủ...", this);
    lblStatus->setObjectName("MatchStatusLabel");
    lblStatus->setAlignment(Qt::AlignCenter);
    lblStatus->setStyleSheet("font-size: 16px; color: white;");

    // 3. Nút Hủy
    QPushButton *btnCancel = new QPushButton("Hủy Tìm Trận", this);
    btnCancel->setObjectName("BtnCancelSearch");
    btnCancel->setFixedSize(200, 50);
    btnCancel->setCursor(Qt::PointingHandCursor);

    layout->addWidget(lblRadar, 0, Qt::AlignCenter);
    layout->addWidget(lblStatus, 0, Qt::AlignCenter);
    layout->addSpacing(20);
    layout->addWidget(btnCancel, 0, Qt::AlignCenter);

    // KẾT NỐI NÚT HỦY VỚI HÀM cancelSearch
    connect(btnCancel, &QPushButton::clicked, this, &MatchmakingWidget::cancelSearch);
}

// --- HÀM LOGIC ---

void MatchmakingWidget::startSearching() {
    handledMatchFound = false; 
    dotCount = 0;
    
    // 1. Bật hiệu ứng chữ chạy
    statusTimer->start(500); 
    lblStatus->setText("Đang tìm đối thủ...");

    // 2. Gửi lệnh lên Server
    GameClient::instance().sendFindMatch();

    // 3. Lắng nghe kết quả (Dùng UniqueConnection để tránh duplicate signal)
    connect(&GameClient::instance(), &GameClient::matchFound, 
            this, &MatchmakingWidget::onMatchFoundNetwork, Qt::UniqueConnection);
}

void MatchmakingWidget::cancelSearch() {
    qDebug() << "[UI] User canceled matchmaking.";

    // 1. Gửi lệnh hủy lên Server
    GameClient::instance().sendCancelMatch();
    
    // 2. Dừng timer
    statusTimer->stop();
    lblStatus->setText("Đã hủy.");
    
    // 3. Ngắt kết nối lắng nghe (để tránh nhận thông báo rác)
    disconnect(&GameClient::instance(), &GameClient::matchFound, 
               this, &MatchmakingWidget::onMatchFoundNetwork);

    // 4. Báo ra ngoài để đóng Dialog
    emit cancelSearchSignal(); 
}

void MatchmakingWidget::onMatchFoundNetwork(const QString& roomId) {
    if (handledMatchFound) return;
    handledMatchFound = true;

    lblStatus->setText("ĐÃ TÌM THẤY ĐỐI THỦ!");
    lblStatus->setStyleSheet("color: #2ecc71; font-weight: bold; font-size: 20px;");
    statusTimer->stop();

    QTimer::singleShot(500, this, [=](){
        emit matchFound(roomId);
    });
}


void MatchmakingWidget::updateStatusText() {
    dotCount = (dotCount + 1) % 4;
    QString dots = QString(".").repeated(dotCount);
    lblStatus->setText(QString("Đang tìm đối thủ%1").arg(dots));
}