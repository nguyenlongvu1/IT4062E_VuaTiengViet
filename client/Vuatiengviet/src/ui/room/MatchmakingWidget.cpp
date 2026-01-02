#include "MatchmakingWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QMovie>
#include <QDebug>
#include <QResizeEvent>
#include "../../network/GameClient.h" 

MatchmakingWidget::MatchmakingWidget(QWidget *parent) : QWidget(parent) {
     this->setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    
    // Timer cập nhật text (...)
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &MatchmakingWidget::updateStatusText);
}
void MatchmakingWidget::resizeEvent(QResizeEvent *event) {
    if (m_backgroundLabel) {
        m_backgroundLabel->resize(this->size());
    }
    QWidget::resizeEvent(event); // Gọi hàm gốc
}
void MatchmakingWidget::setupUi() {
    this->setObjectName("MatchmakingScreen"); 
    m_backgroundLabel = new QLabel(this);
    m_backgroundMovie = new QMovie(":/bgHome(1).gif");

    if (m_backgroundMovie->isValid()) {
        m_backgroundLabel->setMovie(m_backgroundMovie);
        m_backgroundLabel->setScaledContents(true); // Cho phép ảnh co giãn
        m_backgroundMovie->start(); // Bắt đầu chạy động
    }

    // Đưa label nền xuống dưới cùng để không che mất các nút khác
    m_backgroundLabel->lower(); 
    // Resize ngay lập tức cho khớp kích thước hiện tại
    m_backgroundLabel->resize(this->size());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);
    layout->setContentsMargins(0, 0, 0, 0);

    
    // 2. Trạng thái text
    lblStatus = new QLabel("Đang quét máy chủ...", this);
    lblStatus->setObjectName("MatchStatusLabel");
    lblStatus->setAlignment(Qt::AlignCenter);
    lblStatus->setStyleSheet("font-size: 16px; color: white; background: transparent");

    // 3. Nút Hủy
    QPushButton *btnCancel = new QPushButton("Hủy Tìm Trận", this);
    btnCancel->setObjectName("BtnCancelSearch");
    btnCancel->setFixedSize(200, 50);
    btnCancel->setCursor(Qt::PointingHandCursor);

    layout->addStretch();
    layout->addWidget(lblStatus, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(btnCancel, 0, Qt::AlignCenter);
    layout->addSpacing(40);

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