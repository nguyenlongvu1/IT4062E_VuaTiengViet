#include "HomeWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QToolButton>
#include "SettingsDialog.h"
#include "ProfileDialog.h"
#include "../room/MatchmakingWidget.h"
#include "../room/FriendRoomWidget.h"
#include "NotificationDialog.h"
#include <QMessageBox>

HomeWidget::HomeWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("HomeScreen"); // ID để set background cầu vồng
    setupUi();
}

void HomeWidget::setupUi() {
    // Layout chính toàn màn hình
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 40);
    mainLayout->setSpacing(20);

    // =================================================
    // 1. THANH TRÊN (PROFILE BAR)
    // =================================================
    QHBoxLayout *topLayout = new QHBoxLayout();

    QFrame *profileFrame = new QFrame(this);
    profileFrame->setObjectName("ProfileFrame");
    profileFrame->setFixedSize(280, 80);

    QHBoxLayout *profileLayout = new QHBoxLayout(profileFrame);
    profileLayout->setContentsMargins(15, 10, 15, 10);

    lblAvatar = new QLabel("A", this);
    lblAvatar->setObjectName("AvatarLabel");
    lblAvatar->setFixedSize(50, 50);
    lblAvatar->setAlignment(Qt::AlignCenter);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    textLayout->setAlignment(Qt::AlignVCenter);
    
    lblUsername = new QLabel("Người Chơi", this);
    lblUsername->setObjectName("ProfileName");
    lblRank = new QLabel("Rank: Đồng I (0 điểm)", this);
    lblRank->setObjectName("ProfileRank");
    
    textLayout->addWidget(lblUsername);
    textLayout->addWidget(lblRank);

    profileLayout->addWidget(lblAvatar);
    profileLayout->addSpacing(10);
    profileLayout->addLayout(textLayout);
    profileLayout->addStretch();

    QToolButton *btnHistory = new QToolButton(this);
    btnHistory->setObjectName("LogoutBtnIcon"); 
    btnHistory->setFixedSize(50, 50);
    btnHistory->setCursor(Qt::PointingHandCursor);
    QIcon historyIcon(":/history.png");
    if (!historyIcon.isNull()) {
        btnHistory->setIcon(historyIcon);
        btnHistory->setIconSize(QSize(48, 48)); 
    }

    // --- NÚT 2: CÀI ĐẶT ---
    QToolButton *btnSettings = new QToolButton(this);
    btnSettings->setObjectName("LogoutBtnIcon"); 
    btnSettings->setFixedSize(50, 50);
    btnSettings->setCursor(Qt::PointingHandCursor);
    QIcon settingIcon(":/setting.png");
    if (!settingIcon.isNull()) {
        btnSettings->setIcon(settingIcon);
        btnSettings->setIconSize(QSize(48, 48)); 
    }

    

   btnInbox = new QPushButton(this); 
    btnInbox->setObjectName("LogoutBtnIcon"); 
    btnInbox->setFixedSize(50, 50);
    btnInbox->setCursor(Qt::PointingHandCursor);
    btnInbox->setToolTip("Hộp thư thông báo");
    QIcon notiIcon(":/noti.png");
    if (!notiIcon.isNull()) {
        btnInbox->setIcon(notiIcon);
        btnInbox->setIconSize(QSize(32, 32)); 
    }

    // Nút Đăng xuất
    QPushButton *btnLogout = new QPushButton(this);
    btnLogout->setObjectName("LogoutBtnIcon"); 
    btnLogout->setFixedSize(50, 50);
    btnLogout->setCursor(Qt::PointingHandCursor);

    // Load icon logout (Nếu có)
    QIcon logoutIcon(":/logout.png");
    if (!logoutIcon.isNull()) {
        btnLogout->setIcon(logoutIcon);
        btnLogout->setIconSize(QSize(30, 30)); 
    }

    topLayout->addWidget(profileFrame, 0, Qt::AlignTop);
    topLayout->addStretch();
    topLayout->addWidget(btnHistory, 0, Qt::AlignTop);
    topLayout->addSpacing(5);                        // Khoảng cách giữa 2 nút
    topLayout->addWidget(btnSettings, 0, Qt::AlignTop);
    topLayout->addSpacing(5);                        // Khoảng cách giữa 2 nút
    topLayout->addWidget(btnInbox, 0, Qt::AlignTop);  // Nút Chuông trước
    topLayout->addSpacing(5);                        // Khoảng cách giữa 2 nút
    topLayout->addWidget(btnLogout, 0, Qt::AlignTop);

    // =================================================
    // 2. KHU VỰC GIỮA (3 CỘT: BXH - GAME - BẠN BÈ)
    // =================================================
    QHBoxLayout *midLayout = new QHBoxLayout();
    midLayout->setSpacing(20);

    // --- CỘT TRÁI: BXH ---
    leaderboardWidget = new LeaderboardWidget(this);
    leaderboardWidget->setFixedWidth(250); 

    // --- CỘT GIỮA: LOGO & NÚT CHƠI ---
    QVBoxLayout *centerContainer = new QVBoxLayout();
    centerContainer->setAlignment(Qt::AlignCenter);
    centerContainer->setSpacing(20);

   
    QHBoxLayout *playBtnLayout = new QHBoxLayout();
    playBtnLayout->setSpacing(20);
    
    // --- SỬA LỖI QUAN TRỌNG: PHẢI NEW NÚT TRƯỚC KHI DÙNG ---
    btnRank = new QToolButton(this);
    btnRank->setText("TÌM TRẬN\n(Đấu Rank)");
    btnRank->setObjectName("BtnMode_Rank");
    btnRank->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnRank->setCursor(Qt::PointingHandCursor);
    btnRank->setFixedSize(180, 180);

    QIcon rankIcon(":/findRank.png"); 
    if (!rankIcon.isNull()) {
        btnRank->setIcon(rankIcon);
        btnRank->setIconSize(QSize(80, 80));
    }

    btnFriend = new QToolButton(this);
    btnFriend->setText("CHƠI VỚI BẠN\n(Tạo Phòng)");
    btnFriend->setObjectName("BtnMode_Friend");
    btnFriend->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnFriend->setCursor(Qt::PointingHandCursor);
    btnFriend->setFixedSize(180, 180);

    QIcon friendIcon(":/friends.png"); 
    if (!friendIcon.isNull()) {
        btnFriend->setIcon(friendIcon);
        btnFriend->setIconSize(QSize(70, 70)); // Chỉnh kích thước icon
    }
    // -------------------------------------------------------

    playBtnLayout->addWidget(btnRank);
    playBtnLayout->addWidget(btnFriend);
    
    // centerContainer->addWidget(lblLogo);
    centerContainer->addLayout(playBtnLayout);

    // --- CỘT PHẢI: BẠN BÈ ---
    socialWidget = new SocialWidget(this);
    socialWidget->setFixedWidth(250);

    // ADD 3 CỘT VÀO MID LAYOUT
    midLayout->addWidget(leaderboardWidget);      // Trái
    midLayout->addLayout(centerContainer, 1);     // Giữa (Co giãn)
    midLayout->addWidget(socialWidget);           // Phải

   // =================================================
    // 3. THANH DƯỚI (MENU PHỤ)
    // =================================================
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(20);
    bottomLayout->setAlignment(Qt::AlignCenter);

   
  

    // =================================================
    // LẮP RÁP LAYOUT (ĐÃ SỬA GỌN)
    // =================================================
    // Chỉ add mỗi thứ 1 lần theo thứ tự từ trên xuống dưới
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout, 1); // Phần giữa chiếm phần lớn màn hình
    mainLayout->addLayout(bottomLayout);

    // Kết nối
    connect(btnInbox, &QPushButton::clicked, this, &HomeWidget::openInbox);
    connect(btnLogout, &QPushButton::clicked, this, &HomeWidget::logout);
    connect(btnRank, &QToolButton::clicked, this, &HomeWidget::playRanked);
    connect(btnFriend, &QToolButton::clicked, this, &HomeWidget::playWithFriend);
    connect(btnSettings, &QToolButton::clicked, this, &HomeWidget::openSettings);
    connect(btnHistory, &QToolButton::clicked, this, &HomeWidget::openHistory);
   connect(&GameClient::instance(), &GameClient::friendRequestReceived, [=](const QString &senderName){
    m_notifyDialog->addFriendRequest(senderName); // Lưu dữ liệu vào Dialog ngay cả khi đang ẩn
    btnInbox->setStyleSheet("background-color: #e74c3c; border-radius: 25px;");
    QApplication::beep();
});

connect(&GameClient::instance(), &GameClient::pendingListReceived, [=](const QStringList &users){
    if (!users.isEmpty()) {
        for(const QString &u : users) m_notifyDialog->addFriendRequest(u);
        btnInbox->setStyleSheet("background-color: #e74c3c; border-radius: 25px;");
    }
});
// Kết nối tín hiệu nhận thông tin phòng với hàm cập nhật giao diện

}


QString HomeWidget::getRankName(int score) {
    // Dựa theo ảnh Database bạn gửi:
    if (score <= 100)  return "Mù chữ";
    if (score <= 200)  return "Biết chữ sương sương";
    if (score <= 500)  return "Ngôn từ cấp 1";
    if (score <= 800)  return "Đủ đậu cấp 3";
    if (score <= 1200) return "Thủ khoa khối D";
    if (score <= 1600) return "Thánh Chém gió";
    if (score <= 2000) return "Bậc Thầy Văn Phong";
    
    // Từ 2001 trở lên (Max 2 tỷ)
    return "Đế Vương Ngôn Ngữ";
}

void HomeWidget::setPlayerInfo(const QString& name, int score) {
    m_currentUsername = name;
    m_currentScore = score;
    m_currentRankName = getRankName(score);
    m_notifyDialog = new NotificationDialog(this);
    m_notifyDialog->hide();
    // 1. Cập nhật Tên
    lblUsername->setText(name);

    // 2. Cập nhật Avatar (Chữ cái đầu in hoa)
    if (!name.isEmpty()) {
        lblAvatar->setText(name.left(1).toUpper());
    }
    
    // 3. Tính tên Rank từ điểm số
    QString rankName = getRankName(score);

    // 4. Hiển thị ra Label
    // Ví dụ: Rank: Thủ khoa khối D (950 điểm)
    lblRank->setText(QString("Rank: %1 (%2 điểm)").arg(rankName).arg(score));
}
void HomeWidget::openSettings() {
    SettingsDialog dlg(this);
    dlg.exec(); 
}
void HomeWidget::openHistory() {
    // Code mở lịch sử (ví dụ dùng ProfileDialog tab lịch sử)
   ProfileDialog dlg(m_currentUsername, m_currentScore, m_currentRankName, this);
    dlg.exec();
}
// src/ui/home/HomeWidget.cpp

void HomeWidget::joinRankedRoom(const QString& roomId) {
    QDialog dlg(this);
    dlg.setFixedSize(1280, 720);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(0,0,0,0);

    FriendRoomWidget *roomWidget = new FriendRoomWidget(m_currentUsername, true, &dlg);
    layout->addWidget(roomWidget);

    // 1. Gán RoomID ngay lập tức để hết chữ "Đang tạo phòng"
    roomWidget->setRoomID(roomId);

    // 2. Kết nối Signal cập nhật 3 người
    connect(&GameClient::instance(), &GameClient::roomInfoReceived, 
            roomWidget, &FriendRoomWidget::updateMembers);

    // 3. QUAN TRỌNG: Gửi yêu cầu lấy dữ liệu TRƯỚC khi gọi exec()
    // GameClient::instance().sendGetRoomInfo(); 

    // 4. Cuối cùng mới thực thi Dialog
    dlg.exec(); 
}
void HomeWidget::playWithFriend() {
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Phòng Chờ");
    dlg->resize(1024, 768);
    dlg->setAttribute(Qt::WA_DeleteOnClose); // Tự xóa bộ nhớ khi đóng

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(0,0,0,0);
    
    // ========================================================================
    // [FIX QUAN TRỌNG] TRUYỀN m_currentUsername VÀO ĐÂY
    // Để bên trong FriendRoomWidget biết ai là mình mà hiện nút Bắt đầu
    // ========================================================================
    FriendRoomWidget *roomWidget = new FriendRoomWidget(m_currentUsername, true, dlg);
    layout->addWidget(roomWidget);

    // Kết nối nhận thông tin phòng (Host/Guest) để cập nhật giao diện
    // Dùng 'dlg' làm context để tự ngắt kết nối khi đóng Dialog (Tránh Crash)
    connect(&GameClient::instance(), &GameClient::roomInfoReceived, 
        dlg, [roomWidget](const QString &p1, const QString &p2, const QString &p3){
    if (roomWidget) {
        // Truyền đủ 3 tham số (p1, p2, p3) vào hàm cập nhật
        roomWidget->updateMembers(p1, p2, p3);
    }
});

    // Kết nối khi nhận được Room ID từ Server
    connect(&GameClient::instance(), &GameClient::roomJoined, dlg, [=](QString roomId){
        if (roomWidget) {
            roomWidget->setRoomID(roomId);
            // Cập nhật ngay mình là Host để hiện avatar slot 1
            roomWidget->setHostInfo(m_currentUsername); 
        }
    });

    // Gửi lệnh tạo phòng lên Server
    GameClient::instance().sendCreateRoomRequest();

    // Khi bấm nút Rời phòng -> Đóng Dialog
    connect(roomWidget, &FriendRoomWidget::leftRoom, dlg, &QDialog::accept);
    
    dlg->exec(); 
}
void HomeWidget::openInbox() {
    // Reset màu nút chuông
    btnInbox->setStyleSheet("background-color: transparent;"); 
    btnInbox->setObjectName("LogoutBtnIcon");

    if(m_notifyDialog) {
        m_notifyDialog->show(); // Hiển thị Dialog bền vững
        m_notifyDialog->raise();
        m_notifyDialog->activateWindow();
    }
}
void HomeWidget::playRanked() {
    QDialog dlg(this);
    dlg.setFixedSize(500, 500);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    MatchmakingWidget *matchWidget = new MatchmakingWidget(&dlg);
    layout->addWidget(matchWidget);

    QString foundRoomId = "";

    connect(matchWidget, &MatchmakingWidget::matchFound, [&dlg, &foundRoomId](QString roomId){
        foundRoomId = roomId;
        dlg.accept(); // Đóng Dialog Radar trước
    });

    matchWidget->startSearching();

    // CHỈ KHI DIALOG RADAR ĐÃ ĐÓNG THẬT SỰ
    if (dlg.exec() == QDialog::Accepted) {
        // Gọi hàm chuyển màn hình
        this->joinRankedRoom(foundRoomId);
    }
}