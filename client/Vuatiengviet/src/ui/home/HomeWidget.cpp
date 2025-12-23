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
    this->setObjectName("HomeScreen");
    setupUi();
}

void HomeWidget::setupUi() {
    // Layout chính toàn màn hình
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 40);
    mainLayout->setSpacing(20);

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

    QIcon logoutIcon(":/logout.png");
    if (!logoutIcon.isNull()) {
        btnLogout->setIcon(logoutIcon);
        btnLogout->setIconSize(QSize(30, 30)); 
    }

    topLayout->addWidget(profileFrame, 0, Qt::AlignTop);
    topLayout->addStretch();
    topLayout->addWidget(btnHistory, 0, Qt::AlignTop);
    topLayout->addSpacing(5);                        
    topLayout->addWidget(btnSettings, 0, Qt::AlignTop);
    topLayout->addSpacing(5);                       
    topLayout->addWidget(btnInbox, 0, Qt::AlignTop);  
    topLayout->addSpacing(5);                        
    topLayout->addWidget(btnLogout, 0, Qt::AlignTop);

  
    QHBoxLayout *midLayout = new QHBoxLayout();
    midLayout->setSpacing(20);

    leaderboardWidget = new LeaderboardWidget(this);
    // leaderboardWidget->setFixedWidth(250); 

    QVBoxLayout *centerContainer = new QVBoxLayout();
    centerContainer->setAlignment(Qt::AlignCenter);
    centerContainer->setSpacing(20);

   
    QHBoxLayout *playBtnLayout = new QHBoxLayout();
    playBtnLayout->setSpacing(20);
    
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
        btnFriend->setIconSize(QSize(70, 70)); 
    }

    playBtnLayout->addWidget(btnRank);
    playBtnLayout->addWidget(btnFriend);
    
    // centerContainer->addWidget(lblLogo);
    centerContainer->addLayout(playBtnLayout);

    socialWidget = new SocialWidget(this);
    socialWidget->setFixedWidth(350);

    midLayout->addWidget(leaderboardWidget);      
    midLayout->addLayout(centerContainer, 1);     
    midLayout->addWidget(socialWidget);           

    // 3. THANH DƯỚI (MENU PHỤ)
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(20);
    bottomLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(midLayout, 1);
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
connect(&GameClient::instance(), &GameClient::userInfoReceived, 
            this, &HomeWidget::setPlayerInfo);
}

void HomeWidget::setPlayerInfo(const QString& name, int score, const QString& rankName) {
    m_currentUsername = name;
    m_currentScore = score;
    m_currentRankName = rankName;
    m_notifyDialog = new NotificationDialog(this);
    m_notifyDialog->hide();
    // 1. Cập nhật Tên
    lblUsername->setText(name);

    // 2. Cập nhật Avatar (Chữ cái đầu in hoa)
    if (!name.isEmpty()) {
        lblAvatar->setText(name.left(1).toUpper());
    }
    
    // 3. Tính tên Rank từ điểm số
    // QString rankName = getRankName(score);

    // 4. Hiển thị ra Label
    // Ví dụ: Rank: Thủ khoa khối D (950 điểm)
    lblRank->setText(QString("Rank: %1 (%2 điểm)").arg(rankName).arg(score));
}
void HomeWidget::openSettings() {
    SettingsDialog dlg(this);
    dlg.exec(); 
}
void HomeWidget::openHistory() {
   ProfileDialog dlg(m_currentUsername, m_currentScore, m_currentRankName, this);
    dlg.exec();
}

void HomeWidget::joinRankedRoom(const QString& roomId) {
    Q_UNUSED(roomId);

    // 1. Kiểm tra dọn dẹp để tránh chồng chéo
    QList<FriendRoomWidget*> oldWidgets = this->findChildren<FriendRoomWidget*>();
    for (auto old : oldWidgets) { old->deleteLater(); }

    QString myName = GameClient::instance().getCurrentUserID();

    // 2. Tạo Widget phòng chơi
    // Đặt parent là nullptr để nó là một cửa sổ độc lập, không bị phụ thuộc vào Home
    FriendRoomWidget *roomWidget = new FriendRoomWidget(myName, true, nullptr); 
    
    // 3. Thiết lập thuộc tính hiển thị
    roomWidget->setAttribute(Qt::WA_DeleteOnClose);
    roomWidget->setMinimumSize(1280, 720); // Kích thước chuẩn

    // 4. Kết nối quay lại màn hình Home khi thoát phòng
    connect(roomWidget, &FriendRoomWidget::leftRoom, this, [this, roomWidget](){
        this->show();           
        roomWidget->close();        });

    // 5. Cập nhật thông tin người chơi
    connect(&GameClient::instance(), &GameClient::roomInfoReceived, 
            roomWidget, &FriendRoomWidget::updateMembers, Qt::UniqueConnection);

    // 6. THỨ TỰ HIỂN THỊ QUAN TRỌNG:
    roomWidget->show();         // Hiện phòng trước
    roomWidget->raise();        // Đưa lên lớp trên cùng
    roomWidget->activateWindow(); // Tập trung chuột/phím vào đây
    
    this->hide();             

    // 7. Lấy dữ liệu
    GameClient::instance().sendGetRoomInfo(); 
    connect(&GameClient::instance(), &GameClient::matchStartedDirectly, 
            this, [this](QString matchId, QString roomId) {
        
        // 1. Đóng Dialog Radar tìm kiếm (nếu đang mở)
        if (m_radarDialog) m_radarDialog->accept();

        // 2. Khởi tạo Widget Game và chuyển màn hình
        // Dữ liệu lúc này đã được lưu vào DB (Match, MatchPlayers, MatchQuestions)
        this->switchToGameScreen(matchId, roomId);
    });
    }

void HomeWidget::playWithFriend() {
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Phòng Chờ");
    dlg->resize(1024, 768);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(0,0,0,0);
    
    FriendRoomWidget *roomWidget = new FriendRoomWidget(m_currentUsername, true, dlg);
    layout->addWidget(roomWidget);

    connect(&GameClient::instance(), &GameClient::roomInfoReceived, 
        dlg, [roomWidget](const QString &p1, const QString &p2, const QString &p3){
        if (roomWidget) {
            roomWidget->updateMembers(p1, p2, p3);
        }
    });
    connect(&GameClient::instance(), &GameClient::roomJoined, dlg, [=](QString roomId){
        if (roomWidget) {
            roomWidget->setRoomID(roomId);
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
    btnInbox->setStyleSheet("background-color: transparent;"); 
    btnInbox->setObjectName("LogoutBtnIcon");

    if(m_notifyDialog) {
        m_notifyDialog->show(); 
        m_notifyDialog->raise();
        m_notifyDialog->activateWindow();
    }
}
void HomeWidget::playRanked() {
    m_radarDialog = new QDialog(this);
    m_radarDialog->setWindowTitle("Đang tìm đối thủ...");
    m_radarDialog->setFixedSize(400, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(m_radarDialog);
    MatchmakingWidget *matchWidget = new MatchmakingWidget(m_radarDialog);
    layout->addWidget(matchWidget);

    // Kết nối Signal: Khi Server gửi START_MATCH về
    connect(&GameClient::instance(), &GameClient::matchStartedDirectly, 
            this, [this](QString matchId, QString roomId) {
        if (m_radarDialog) m_radarDialog->accept();
        this->switchToGameScreen(matchId, roomId); 
    });

    // Kích hoạt gửi lệnh FIND_MATCH lên Server
    matchWidget->startSearching(); 

    m_radarDialog->exec();
}

void HomeWidget::switchToGameScreen(QString matchId, QString roomId) {
    QMessageBox::information(this, "Ghép trận thành công", 
        QString("Đã lưu vào DB!\nMatch ID: %1\nRoom ID: %2\n\nHệ thống đã bốc sẵn 30 câu hỏi.").arg(matchId).arg(roomId));
    
    qDebug() << "[TEST] San sang vao Game voi MatchID: " << matchId;
}