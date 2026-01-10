#include "HomeWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
// #include <GameButton>
#include "SettingsDialog.h"
#include "ProfileDialog.h"
#include "../room/MatchmakingWidget.h"
#include "../room/FriendRoomWidget.h"
#include "NotificationDialog.h"
#include <QMessageBox>
#include "../../utils/GameButton.h"

HomeWidget::HomeWidget(QWidget *parent) : QWidget(parent) {
    this->setObjectName("HomeScreen");
    setupUi();
}
void HomeWidget::setupUi() {
    // 1. Layout chính toàn màn hình
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 40);
    mainLayout->setSpacing(0);

    // Layout chứa 3 cột chính
    QHBoxLayout *columnsLayout = new QHBoxLayout();
    columnsLayout->setSpacing(60); 

    // --- STYLE CHUNG ---
    circleBtnStyle = 
        "QAbstractButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(50, 50, 80, 0.95), stop:1 rgba(20, 20, 35, 0.98));"
        "   border: 1px solid rgba(255, 255, 255, 0.15);"
        "   border-radius: 40px;" 
        "   outline: none;"
        "}"
        "QAbstractButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(70, 70, 100, 0.95), stop:1 rgba(40, 40, 60, 0.98)); border: 1px solid rgba(255, 255, 255, 0.3); border-radius: 40px; }"
        "QAbstractButton:pressed { background: rgba(10, 10, 20, 0.6); border-radius: 40px !important; margin-top: 2px; }";

    auto addShadow = [](QWidget* w) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(w);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(255, 255, 255, 250));
        shadow->setOffset(0, 0);
        w->setGraphicsEffect(shadow);
    };

    // =========================================================================
    // [CỘT 1 - TRÁI]: PROFILE + LEADERBOARD
    // =========================================================================
    QVBoxLayout *leftColumn = new QVBoxLayout();
    leftColumn->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    leftColumn->setSpacing(20);

    QFrame *profileFrame = new QFrame(this);
    profileFrame->setFixedSize(495, 120);
    profileFrame->setAttribute(Qt::WA_TranslucentBackground);
    profileFrame->setStyleSheet("background: transparent; border: none;");
    profileFrame->setContentsMargins(25, 0, 0, 0);


    
    QVBoxLayout *profileRootLayout = new QVBoxLayout(profileFrame);
    profileRootLayout->setContentsMargins(5, 5, 0, 0);

    QWidget *borderWidget = new QWidget(profileFrame);
    borderWidget->setFixedSize(450, 100); 
    borderWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255, 255, 255, 0), stop:0.5 rgba(255, 255, 255, 0.5), stop:1 rgba(255, 255, 255, 0)); border-radius: 20px;");

    QGraphicsDropShadowEffect *profileGlow = new QGraphicsDropShadowEffect(borderWidget);
    profileGlow->setBlurRadius(30);              // Độ nhòe (càng cao càng lan rộng)
    profileGlow->setColor(QColor(255, 255, 255, 150)); // Màu trắng sáng, độ trong suốt 150
    profileGlow->setOffset(0, 0);                // Tỏa đều xung quanh
    borderWidget->setGraphicsEffect(profileGlow);
    
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2);

    QWidget *glassPanel = new QWidget(borderWidget);
    glassPanel->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(50, 50, 80, 0.95), stop:1 rgba(20, 20, 35, 0.98)); border-radius: 18px;");
    borderLayout->addWidget(glassPanel);
    profileRootLayout->addWidget(borderWidget);

    // Neon Lines cho Profile
    auto createNeon = [&](QWidget* parent, int x) {
        QWidget* line = new QWidget(parent);
        line->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,0), stop:0.5 white, stop:1 rgba(255,255,255,0));");
        line->setGeometry(x, 18, 2, 60);
        QGraphicsDropShadowEffect* glow = new QGraphicsDropShadowEffect(line);
        glow->setBlurRadius(20); glow->setColor(QColor(200, 230, 255)); glow->setOffset(0,0);
        line->setGraphicsEffect(glow);
    };
    createNeon(glassPanel, 0);
    createNeon(glassPanel, 444);

    QHBoxLayout *contentLayout = new QHBoxLayout(glassPanel);
    contentLayout->setContentsMargins(20, 10, 20, 10);
    contentLayout->setSpacing(15);
    lblAvatar = new QLabel("A", this);
    lblAvatar->setFixedSize(60, 60);
    lblAvatar->setAlignment(Qt::AlignCenter);
    lblAvatar->setStyleSheet("background-color: #8e44ad; color: white; border-radius: 30px; font-weight: bold; font-size: 28px; border: 2px solid #a29bfe;");

    QVBoxLayout *infoTextLayout = new QVBoxLayout();
    infoTextLayout->setSpacing(0);
    // infoTextLayout->setContentsMargins(0, 12, 0, 12);
    lblUsername = new QLabel("Người Chơi", this);
    lblUsername->setStyleSheet("color: white; font-weight: 900; font-size: 22px; background: transparent; margin-top: 6px;");
    lblRank = new QLabel("Rank: Đồng I (0 điểm)", this);
    lblRank->setStyleSheet("color: #f1c40f; font-size: 15px; font-weight: 500; background: transparent; margin-bottom: 4px;");
    infoTextLayout->addWidget(lblUsername);
    infoTextLayout->addWidget(lblRank);

    contentLayout->addWidget(lblAvatar);
    // contentLayout->addSpacing(-5);
    contentLayout->addLayout(infoTextLayout);
    contentLayout->addStretch();
    
    leaderboardWidget = new LeaderboardWidget(this);
    leaderboardWidget->setFixedSize(510, 810);

    leftColumn->addWidget(profileFrame);
    leftColumn->addSpacing(-10);
    leftColumn->addWidget(leaderboardWidget);
    leftColumn->addStretch();

    // =========================================================================
    // [CỘT 2 - GIỮA]: BANNER + CÁC NÚT CHƠI
    // =========================================================================
    QWidget *centerWrapper = new QWidget(this);
    centerWrapper->setObjectName("CenterWrapper");
    centerWrapper->setFixedWidth(740);
    centerWrapper->setStyleSheet("#CenterWrapper { background: transparent; }");
    QVBoxLayout *centerColumn = new QVBoxLayout(centerWrapper);
    centerColumn->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    centerColumn->setSpacing(20);

    
    QPixmap bannerPix(":/banner.png");
    QPixmap scaledPix;
    if (!bannerPix.isNull()) {
        // Scale ảnh về 500x400 (như bạn muốn)
        scaledPix = bannerPix.scaled(600, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // 2. Tạo Container chứa hiệu ứng
    QWidget *glowContainer = new QWidget(this);
    glowContainer->setFixedSize(700, 400); // Kích thước khung chứa (phải to hơn ảnh để chứa ánh sáng)
    
    // [QUAN TRỌNG] Dòng này làm cho nền của Widget biến mất hoàn toàn, sửa lỗi "Hộp đen"
    glowContainer->setAttribute(Qt::WA_TranslucentBackground); 
    glowContainer->setStyleSheet("background: transparent;");

    // 3. Layout xếp chồng (Grid)
    QGridLayout *stackLayout = new QGridLayout(glowContainer);
    stackLayout->setContentsMargins(0, 0, 0, 0);
    stackLayout->setAlignment(Qt::AlignCenter);

    // 4. LỚP DƯỚI: Ánh sáng tím (Background Glow)
    QLabel *lblPurpleGlow = new QLabel(glowContainer);
    lblPurpleGlow->setPixmap(scaledPix);
    lblPurpleGlow->setAlignment(Qt::AlignCenter);
    lblPurpleGlow->setStyleSheet("background: transparent;"); // Đảm bảo trong suốt

    QGraphicsDropShadowEffect *purpleEffect = new QGraphicsDropShadowEffect(lblPurpleGlow);
    purpleEffect->setColor(QColor("#bd00ff")); // Tím neon
    purpleEffect->setBlurRadius(80);           // Nhòe rộng ra ngoài
    purpleEffect->setOffset(0, 0);
    lblPurpleGlow->setGraphicsEffect(purpleEffect);

    // 5. LỚP TRÊN: Ảnh chính + Ánh sáng xanh (Foreground)
    QLabel *lblMainImage = new QLabel(glowContainer); // Đặt tên mới để không trùng lặp
    lblMainImage->setPixmap(scaledPix);
    lblMainImage->setAlignment(Qt::AlignCenter);
    lblMainImage->setStyleSheet("background: transparent;");

    QGraphicsDropShadowEffect *cyanEffect = new QGraphicsDropShadowEffect(lblMainImage);
    cyanEffect->setColor(QColor("#00eaff"));   // Xanh Cyan
    cyanEffect->setBlurRadius(30);             // Nhòe ít hơn để nét ở tâm
    cyanEffect->setOffset(0, 0);
    lblMainImage->setGraphicsEffect(cyanEffect);

    // 6. Xếp chồng lên nhau (Cùng vị trí 0,0)
    // Add lớp tím trước (nằm dưới)
    stackLayout->addWidget(lblPurpleGlow, 0, 0, Qt::AlignCenter);
    // Add lớp ảnh chính sau (nằm trên)
    stackLayout->addWidget(lblMainImage, 0, 0, Qt::AlignCenter);

    

    // ======================================================
    // 1. KHỞI TẠO NÚT RANK (Dùng biến thành viên btnRank)
    // ======================================================
    btnRank = new GameButton(this); // Gán vào biến thành viên
    btnRank->setObjectName("BtnMode_Rank");
    btnRank->setFixedSize(500, 130);
    btnRank->setCursor(Qt::PointingHandCursor);
    QGraphicsDropShadowEffect *glowRank = new QGraphicsDropShadowEffect(btnRank);
    glowRank->setBlurRadius(50); // Độ tỏa lớn (Hào quang)
    glowRank->setColor(QColor(255, 140, 0, 255)); // Màu cam, Alpha 180
    glowRank->setOffset(0, 0); // Tỏa đều 4 phía (không lệch)
    btnRank->setGraphicsEffect(glowRank);
    
    // Style cho vỏ nút (Background Gradient)
    btnRank->setStyleSheet(
        "#BtnMode_Rank {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ac43, stop:1 #f5613d);"
        "   border: 2px solid #fbad6c;"
        "   border-radius: 15px;"
        "}"
        "#BtnMode_Rank:hover {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffbe7f, stop:1 #f39c12);"
        "}"
        "#BtnMode_Rank:pressed {"
        "   background-color: #d35400;"
        "}"
    );

    // --- TẠO LAYOUT BÊN TRONG NÚT ĐỂ CHỨA ICON VÀ TEXT RIÊNG BIỆT ---
    QHBoxLayout *rankLayout = new QHBoxLayout(btnRank);
    rankLayout->setContentsMargins(40, 20, 40, 25); // Căn chỉnh lề
    rankLayout->setSpacing(20);

    // A. Icon có bóng
    QLabel *lblRankIcon = new QLabel(btnRank);
    lblRankIcon->setPixmap(QPixmap(":/findRank.png").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    lblRankIcon->setStyleSheet("background: transparent; border: none;");
    
    QGraphicsDropShadowEffect *shadowIconRank = new QGraphicsDropShadowEffect(lblRankIcon);
    shadowIconRank->setBlurRadius(30);
    shadowIconRank->setColor(QColor(0, 0, 0, 140));
    shadowIconRank->setOffset(0, 5);
    lblRankIcon->setGraphicsEffect(shadowIconRank);

    // B. Text có bóng
    QLabel *lblRankText = new QLabel("TÌM TRẬN", btnRank);
    lblRankText->setStyleSheet("color: white; font-weight: 3000; font-size: 40px; background: transparent; border: none; font-family: 'Nunito', sans-serif;");
    lblRankText->setAlignment(Qt::AlignCenter);

    QGraphicsDropShadowEffect *shadowTextRank = new QGraphicsDropShadowEffect(lblRankText);
    shadowTextRank->setBlurRadius(30);
    shadowTextRank->setColor(QColor(0, 0, 0, 180)); // Bóng chữ đậm hơn
    shadowTextRank->setOffset(2, 2);
    lblRankText->setGraphicsEffect(shadowTextRank);

    rankLayout->addWidget(lblRankIcon);
    rankLayout->addWidget(lblRankText);
    rankLayout->addStretch(); // Đẩy nội dung sang trái


    // ======================================================
    // 2. KHỞI TẠO NÚT FRIEND (Dùng biến thành viên btnFriend)
    // ======================================================
    btnFriend = new GameButton(this);
    btnFriend->setObjectName("BtnMode_Friend");
    btnFriend->setFixedSize(500, 130);
    btnFriend->setCursor(Qt::PointingHandCursor);
    QGraphicsDropShadowEffect *glowFriend = new QGraphicsDropShadowEffect(btnFriend);
    glowFriend->setBlurRadius(50); // Độ tỏa lớn
    glowFriend->setColor(QColor(46, 204, 113, 250)); // Màu xanh, Alpha 180
    glowFriend->setOffset(0, 0); // Tỏa đều
    btnFriend->setGraphicsEffect(glowFriend);

    btnFriend->setStyleSheet(
        "#BtnMode_Friend {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ec182, stop:1 #0f6c53);"
        "   border: 2px solid #63c89f;"
        "   border-radius: 15px;"
        "}"
        "#BtnMode_Friend:hover {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #7aefa3, stop:1 #4cd137);"
        "}"
        "#BtnMode_Friend:pressed {"
        "   background-color: #219653;"
        "}"
    );

    QHBoxLayout *friendLayout = new QHBoxLayout(btnFriend);
    friendLayout->setContentsMargins(48, 20, 40, 25);
    friendLayout->setSpacing(23);

    // A. Icon
    QLabel *lblFriendIcon = new QLabel(btnFriend);
    lblFriendIcon->setPixmap(QPixmap(":/friends.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    lblFriendIcon->setStyleSheet("background: transparent; border: none;");

    QGraphicsDropShadowEffect *shadowIconFriend = new QGraphicsDropShadowEffect(lblFriendIcon);
    shadowIconFriend->setBlurRadius(30);
    shadowIconFriend->setColor(QColor(0, 0, 0, 140));
    shadowIconFriend->setOffset(0, 5);
    lblFriendIcon->setGraphicsEffect(shadowIconFriend);

    // B. Text
    QLabel *lblFriendText = new QLabel("TẠO PHÒNG", btnFriend);
    lblFriendText->setStyleSheet("color: white; font-weight: 3000; font-size: 40px; background: transparent; border: none; font-family: 'Nunito', sans-serif;");
    lblFriendText->setAlignment(Qt::AlignCenter);

    QGraphicsDropShadowEffect *shadowTextFriend = new QGraphicsDropShadowEffect(lblFriendText);
    shadowTextFriend->setBlurRadius(30);
    shadowTextFriend->setColor(QColor(0, 0, 0, 180));
    shadowTextFriend->setOffset(2, 2);
    lblFriendText->setGraphicsEffect(shadowTextFriend);

    // Hàm lambda để tạo hiệu ứng viền sáng trên/dưới cho bất kỳ nút nào
    auto addTopBottomGlow = [](QWidget* parentBtn, int w, int h) {
        // 1. STYLE CHUNG (Gradient NGANG)
        QString hStyle = 
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0," // Đổi thành NGANG (x tăng, y giữ)
            "   stop:0    rgba(255, 255, 255, 0),"   // Trái: Trong suốt
            "   stop:0.1  rgba(255, 255, 255, 0.1)," 
            "   stop:0.5  rgba(255, 255, 255, 1.0)," // Giữa: Trắng sáng rực
            "   stop:0.9  rgba(255, 255, 255, 0.1)," 
            "   stop:1    rgba(255, 255, 255, 0));"  // Phải: Trong suốt
            "border: none;";

        // 2. TẠO LINE TRÊN (Top Line)
        QWidget *topLine = new QWidget(parentBtn);
        topLine->setStyleSheet(hStyle);
        topLine->setAttribute(Qt::WA_TransparentForMouseEvents); // Quan trọng: để bấm xuyên qua được

        QGraphicsDropShadowEffect *glowT = new QGraphicsDropShadowEffect(topLine);
        glowT->setBlurRadius(50); // Tỏa nhẹ
        glowT->setColor(QColor(255, 255, 255, 255)); // Màu trắng sáng
        glowT->setOffset(0, 0);
        topLine->setGraphicsEffect(glowT);

        // Canh chỉnh vị trí Top
        // x=25 (thụt vào tránh góc bo), y=0 (sát mép trên)
        // width = 500 - 50 = 450, height = 3px (mảnh)
        topLine->setGeometry(25, 0, w - 50, 3); 


        // 3. TẠO LINE DƯỚI (Bottom Line)
        QWidget *botLine = new QWidget(parentBtn);
        botLine->setStyleSheet(hStyle);
        botLine->setAttribute(Qt::WA_TransparentForMouseEvents);

        QGraphicsDropShadowEffect *glowB = new QGraphicsDropShadowEffect(botLine);
        glowB->setBlurRadius(40);
        glowB->setColor(QColor(255, 255, 255,255)); // Mờ hơn xíu cho tự nhiên
        glowB->setOffset(0, 0);
        botLine->setGraphicsEffect(glowB);

        // Canh chỉnh vị trí Bottom
        // y = 150 (chiều cao nút) - 3 (chiều cao line) = 147
        botLine->setGeometry(25, h - 3, w - 50, 3);
    };

    // --- ÁP DỤNG CHO 2 NÚT ---
    addTopBottomGlow(btnRank, 500, 130);
    addTopBottomGlow(btnFriend, 500, 130);

    friendLayout->addWidget(lblFriendIcon);
    friendLayout->addWidget(lblFriendText);
    friendLayout->addStretch();

    centerColumn->addSpacing(30); 
    centerColumn->addWidget(glowContainer, 0, Qt::AlignCenter);
    centerColumn->addWidget(btnRank, 0, Qt::AlignCenter);
    centerColumn->addWidget(btnFriend, 0, Qt::AlignCenter);
    centerColumn->addStretch();

    // =========================================================================
    // [CỘT 3 - PHẢI]: NÚT TRÒN MENU + BẠN BÈ
    // =========================================================================
    QVBoxLayout *rightColumn = new QVBoxLayout();
    rightColumn->setAlignment(Qt::AlignTop | Qt::AlignRight);
    rightColumn->setContentsMargins(0, 16, 0, 0);
    

    QHBoxLayout *topButtonsLayout = new QHBoxLayout();
    topButtonsLayout->setSpacing(5);

    auto createMenuBtn = [&](QString iconPath, QString tooltip) {
        GameButton* btn = new GameButton(this);
        btn->setFixedSize(80, 80);
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(40, 40));
        btn->setToolTip(tooltip);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(circleBtnStyle);
        addShadow(btn);
        return btn;
    };

    
    GameButton* btnHistory = createMenuBtn(":/history.png", "Lịch sử đấu");
    GameButton* btnSettings = createMenuBtn(":/setting.png", "Cài đặt");
    btnInbox = new GameButton(this); // Cần GameButtoncho logic notify
    btnInbox->setFixedSize(80, 80);
    btnInbox->setIcon(QIcon(":/noti.png"));
    btnInbox->setIconSize(QSize(36, 36));
    btnInbox->setStyleSheet(circleBtnStyle);
    addShadow(btnInbox);
    GameButton* btnLogout = new GameButton(this);
    btnLogout->setFixedSize(80, 80);
    btnLogout->setIcon(QIcon(":/logout.png"));
    btnLogout->setIconSize(QSize(32, 32));
    btnLogout->setStyleSheet(circleBtnStyle);
    addShadow(btnLogout);

    topButtonsLayout->addWidget(btnHistory);
    topButtonsLayout->addWidget(btnSettings);
    topButtonsLayout->addWidget(btnInbox);
    topButtonsLayout->addWidget(btnLogout);

    socialWidget = new SocialWidget(this);
    socialWidget->setFixedSize(510, 810);
    rightColumn->addLayout(topButtonsLayout);
    rightColumn->addSpacing(-31);
    rightColumn->addWidget(socialWidget);
    rightColumn->addStretch();

    // HOÀN THIỆN
    columnsLayout->addSpacing(0);
    columnsLayout->addLayout(leftColumn);
    columnsLayout->addWidget(centerWrapper);
    columnsLayout->addLayout(rightColumn);
    columnsLayout->addSpacing(0);

    mainLayout->addLayout(columnsLayout);

    // KẾT NỐI
    connect(btnInbox, &GameButton::clicked, this, &HomeWidget::openInbox);
    connect(btnLogout, &GameButton::clicked, this, &HomeWidget::logout);
    connect(btnRank, &GameButton::clicked, this, &HomeWidget::playRanked);
    connect(btnFriend, &GameButton::clicked, this, &HomeWidget::playWithFriend);
    connect(btnSettings, &GameButton::clicked, this, &HomeWidget::openSettings);
    connect(btnHistory, &GameButton::clicked, this, &HomeWidget::openHistory);

    // Friend Notify logic
  connect(&GameClient::instance(), &GameClient::friendRequestReceived, [=](const QString &senderName){
        if(m_notifyDialog) m_notifyDialog->addFriendRequest(senderName);
        QApplication::beep();
    });

    connect(&GameClient::instance(), &GameClient::pendingListReceived, [=](const QStringList &users){
        if (!users.isEmpty() && m_notifyDialog) {
            for(const QString &u : users) m_notifyDialog->addFriendRequest(u);
        }
    });
    
    connect(&GameClient::instance(), &GameClient::userInfoReceived, this, &HomeWidget::setPlayerInfo);
}

void HomeWidget::setPlayerInfo(const QString& name, int score, const QString& rankName) {
    m_currentUsername = name;
    m_currentScore = score;
    m_currentRankName = rankName;

    if (m_notifyDialog) {
        delete m_notifyDialog; 
    }
    m_notifyDialog = new NotificationDialog(this);
   connect(m_notifyDialog, &NotificationDialog::notificationCountChanged, this, [=](int count){
        if (count > 0) {
            // Có thông báo: Đổi sang màu ĐỎ (Gradient đỏ cho đẹp)
            QString alertStyle = 
                "QAbstractButton { "
                "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); "
                "   border: 2px solid #ff6b6b; "
                "   border-radius: 40px; "
                "}"
                "QAbstractButton:hover { background: #ff6b6b; }";
            btnInbox->setStyleSheet(alertStyle);
            
            // Đổi icon sang chuông rung hoặc màu khác nếu muốn
            // btnInbox->setIcon(QIcon(":/noti_active.png")); 
        } else {
            // Hết thông báo: Reset về mặc định
            btnInbox->setStyleSheet(circleBtnStyle);
            
            // Reset icon
            // btnInbox->setIcon(QIcon(":/noti.png"));
        }
    });

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
    // btnInbox->setObjectName("LogoutBtnIcon");

    if(m_notifyDialog) {
        m_notifyDialog->show(); 
        m_notifyDialog->raise();
        m_notifyDialog->activateWindow();
    }
}
void HomeWidget::playRanked() {
    m_radarDialog = new QDialog(this);
    m_radarDialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    m_radarDialog->setFixedSize(400, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(m_radarDialog);
    MatchmakingWidget *matchWidget = new MatchmakingWidget(m_radarDialog);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(matchWidget);

    // CHỈ đóng radar khi có trận, KHÔNG gọi switchToGameScreen ở đây 
    // vì GameClient.cpp sẽ lo việc nhận MatchID thật từ Server
    connect(matchWidget, &MatchmakingWidget::matchFound, this, [this](QString roomId){
        if (m_radarDialog) m_radarDialog->accept(); 
    });

    connect(matchWidget, &MatchmakingWidget::cancelSearchSignal, m_radarDialog, &QDialog::reject);

    matchWidget->startSearching();
    m_radarDialog->exec();
}

void HomeWidget::switchToGameScreen(QString matchId, QString roomId) {
    
    qDebug() << "[TEST] San sang vao Game voi MatchID: " << matchId;
    
    emit signalStartGame(matchId, roomId);
}
void HomeWidget::setUserProfile(const QString &name, int points, const QString &rank) {
    // Gọi lại hàm setPlayerInfo có sẵn cho đỡ lặp code
    setPlayerInfo(name, points, rank);
}
void HomeWidget::updateLeaderboard(const QList<RankItem> &items) {
    if (leaderboardWidget) {
        leaderboardWidget->updateLeaderboard(items);
    }
}