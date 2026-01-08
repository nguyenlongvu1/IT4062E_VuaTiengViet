#include "SocialWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QLineEdit>
#include <../../utils/GameButton.h>
#include <QFrame>
#include <QDebug>
#include "../../network/GameClient.h"
#include <QGraphicsDropShadowEffect>

SocialWidget::SocialWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    this->setStyleSheet("background: transparent;");
    setupUi();
    reloadFriendList();
}

void SocialWidget::setupUi() {
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    // [SỬA LỖI TRÀN 1] Đưa margin về 0 để kích thước khớp chuẩn 450x750
    rootLayout->setContentsMargins(0, 0, 0, 0); 
    this->setAttribute(Qt::WA_TranslucentBackground);
    int glowMargin = 30;
    rootLayout->setContentsMargins(glowMargin, glowMargin, glowMargin, glowMargin);

    // =========================================================================
    // 1. KHUNG VIỀN PHÁT SÁNG (BORDER WIDGET) - GIỮ NGUYÊN
    // =========================================================================
    QWidget *borderWidget = new QWidget(this);
    borderWidget->setObjectName("borderWidget");
    borderWidget->setFixedSize(450, 750); 
    
    // Gradient nền viền (Outer Glow)
    borderWidget->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1," 
        "   stop:0    rgba(255, 255, 255, 0),"    
        "   stop:0.5  rgba(255, 255, 255, 0.5)," 
        "   stop:1    rgba(255, 255, 255, 0));"
        "border-radius: 20px;" 
    );

    // Layout viền
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2); 

    // =========================================================================
    // 2. KÍNH TỐI (GLASS PANEL) - GIỮ NGUYÊN
    // =========================================================================
    QWidget *glassPanel = new QWidget(borderWidget);
    glassPanel->setObjectName("glassPanel");
    glassPanel->setStyleSheet(
        "#glassPanel {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 rgba(50, 50, 80, 0.95),"    
        "       stop:1 rgba(20, 20, 35, 0.98));"  
        "   border-radius: 20px;" 
        "   border: none;"
        "}"
    );

    // Hiệu ứng bóng đổ (Shadow)
    QGraphicsDropShadowEffect *panelShadow = new QGraphicsDropShadowEffect(borderWidget);
    panelShadow->setBlurRadius(30);
    panelShadow->setColor(QColor(255, 255, 255, 180)); 
    panelShadow->setOffset(0, 0);
    borderWidget->setGraphicsEffect(panelShadow);

    borderLayout->addWidget(glassPanel);
    rootLayout->addWidget(borderWidget, 0, Qt::AlignCenter);

    // =========================================================================
    // 3. ĐƯỜNG KẺ NEON DỌC - GIỮ NGUYÊN
    // =========================================================================
    // --- TRÁI ---
    QWidget *leftLine = new QWidget(glassPanel);
    leftLine->setStyleSheet(
       "background: qlineargradient(x1:0, y1:0, x2:0, y2:1," 
        "    stop:0    rgba(255, 255, 255, 0),"    
        "    stop:0.2  rgba(255, 255, 255, 0.4)," 
        "    stop:0.5  rgba(255, 255, 255, 1.0)," 
        "    stop:0.8  rgba(255, 255, 255, 0.4)," 
        "    stop:1    rgba(255, 255, 255, 0));"  
        "border: none;"
    );
    QGraphicsDropShadowEffect *glowL = new QGraphicsDropShadowEffect(leftLine);
    glowL->setBlurRadius(80); 
    glowL->setColor(QColor(200, 230, 255, 255));
    glowL->setOffset(0,0);
    leftLine->setGraphicsEffect(glowL);
    leftLine->setGeometry(-1, 40, 2, 666); 

    // --- PHẢI ---
    QWidget *rightLine = new QWidget(glassPanel);
    rightLine->setStyleSheet(leftLine->styleSheet());
    QGraphicsDropShadowEffect *glowR = new QGraphicsDropShadowEffect(rightLine);
    glowR->setBlurRadius(80); glowR->setColor(QColor(200, 230, 255, 255)); glowR->setOffset(0,0);
    rightLine->setGraphicsEffect(glowR);
    rightLine->setGeometry(445, 40, 1, 666);


    // =========================================================================
    // 4. NỘI DUNG BÊN TRONG (LAYOUT)
    // =========================================================================
    QVBoxLayout *mainLayout = new QVBoxLayout(glassPanel);
    mainLayout->setContentsMargins(25, 25, 25, 25); 
    mainLayout->setSpacing(0);

    // [A] Tiêu đề
    QLabel *lblTitle = new QLabel("TÌM BẠN BÈ", glassPanel);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("color: white; font-size: 25px; font-weight: 900; letter-spacing: 1px; background: transparent;");
    mainLayout->addWidget(lblTitle);
    mainLayout->addSpacing(15);

    // Kẻ ngang 1
    QWidget *sep1 = new QWidget(glassPanel); 
    sep1->setFixedHeight(2);
    sep1->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 transparent, stop:0.5 rgba(255, 255, 255, 1), stop:1 transparent);");
    mainLayout->addWidget(sep1);
    mainLayout->addSpacing(15);

    // [B] Thanh tìm kiếm
    QWidget *searchContainer = new QWidget(glassPanel);
    searchContainer->setFixedHeight(45);
    searchContainer->setStyleSheet(
        "background-color: rgba(255, 255, 255, 0.22); border-radius: 10px; border: 1px solid rgba(255,255,255,0.1);"
    );
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(5, 0, 15, 0);
    searchLayout->setSpacing(0);
    
    GameButton *iconSearch = new GameButton(glassPanel);
    iconSearch->setIcon(QIcon(":/search.png")); 
    iconSearch->setFixedSize(35, 35);
    iconSearch->setIconSize(QSize(25, 25));
    iconSearch->setStyleSheet("border: none; background: transparent;");
    
    txtSearch = new QLineEdit(glassPanel);
    txtSearch->setPlaceholderText("Nhập tên...");
    txtSearch->setStyleSheet("border: none; background: transparent; color: rgba(255,255,255,0.6); font-size: 17px; margin-left:0px; padding-left:0px;");
    
    searchLayout->addWidget(iconSearch);
    searchLayout->addWidget(txtSearch);
    mainLayout->addWidget(searchContainer);
    
    // -------------------------------------------------------------------------
    // ĐÃ XÓA PHẦN TABS (Bạn Bè / Gần Đây) TẠI ĐÂY
    // -------------------------------------------------------------------------

    // Kẻ ngang 2 (Giữ lại để ngăn cách thanh tìm kiếm và danh sách)
    mainLayout->addSpacing(15);
    QWidget *sep2 = new QWidget(glassPanel); sep2->setFixedHeight(1);
    sep2->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 transparent, stop:0.5 rgba(255,255,255,0.3), stop:1 transparent);");
    mainLayout->addWidget(sep2);
    mainLayout->addSpacing(10); // Spacing nhỏ trước khi vào list

    // [C] Danh sách bạn bè (Không dùng StackedWidget nữa vì chỉ còn 1 list)
    listFriends = new QListWidget(glassPanel);
    listFriends->setStyleSheet("QListWidget { background: transparent; border: none; outline: none; }");
    listFriends->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listFriends->setFocusPolicy(Qt::NoFocus);

    // Thêm trực tiếp vào layout chính
    mainLayout->addWidget(listFriends);

    // --- LOGIC EVENTS ---
    connect(txtSearch, &QLineEdit::textChanged, [=](const QString &text){
       if (text.isEmpty()) reloadFriendList();
    });
    connect(txtSearch, &QLineEdit::returnPressed, this, &SocialWidget::onSearchFriend);
    connect(iconSearch, &GameButton::clicked, this, &SocialWidget::onSearchFriend);

    // Tín hiệu từ Server
    connect(&GameClient::instance(), &GameClient::searchResultReceived, 
            this, &SocialWidget::onSearchResultReceived);
    
    connect(&GameClient::instance(), &GameClient::friendListUpdated, 
            this, &SocialWidget::onFriendListUpdated);

    connect(&GameClient::instance(), &GameClient::friendListReceived, 
        this, [=](const QList<UserSearchResult>& friends){
        m_friendCache.clear();
        for(const auto& f : friends) m_friendCache.append(f.username);
        
        if (!txtSearch->text().isEmpty()) return;

        listFriends->clear();
        if (friends.isEmpty()) return;

        QStringList addedUsers; 
        for (const auto& f : friends) {
            if (!addedUsers.contains(f.username)) {
                addSearchResultItem(f); 
                addedUsers.append(f.username);
            }
        }
    });
}

// ... (Các hàm logic bên dưới GIỮ NGUYÊN HOÀN TOÀN) ...

void SocialWidget::onSearchFriend() {
    QString keyword = txtSearch->text().trimmed();
    if (keyword.isEmpty()) {
        reloadFriendList(); 
        return;
    }
    listFriends->clear();
    GameClient::instance().sendSearchRequest(keyword);
}

void SocialWidget::onSearchResultReceived(const QList<UserSearchResult>& results) {
    if (txtSearch->text().isEmpty()) return;
    
    listFriends->clear();

    for (UserSearchResult user : results) {
        if (m_friendCache.contains(user.username)) {
            user.isFriend = true; 
        }

        addSearchResultItem(user);
    }
}

void SocialWidget::onFriendListUpdated() {
    reloadFriendList();
}

void SocialWidget::reloadFriendList() {
    if (!txtSearch->text().isEmpty()) return;
    listFriends->clear();
    GameClient::instance().sendGetFriendList(); 
}

void SocialWidget::addSearchResultItem(const UserSearchResult& user) {
    QListWidgetItem *item = new QListWidgetItem(listFriends);
    item->setSizeHint(QSize(0, 70)); 
    item->setData(Qt::UserRole, user.username);

    // Widget tổng
    QWidget *wid = new QWidget();
    wid->setStyleSheet("background: transparent;");

    QVBoxLayout *mainVLayout = new QVBoxLayout(wid);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setSpacing(0);

    // =========================================================================
    // PHẦN 1: NỘI DUNG CHÍNH (ITEM) - GIỮ NGUYÊN
    // =========================================================================
    QWidget *topContent = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(topContent);
    contentLayout->setContentsMargins(10, 0, 15, 5);
    contentLayout->setSpacing(10);

    // --- LOGIC MÀU SẮC ---
    QString statusColor;
    bool isGlowing = false;
    
    if (user.isFriend) {
        if (user.status == "Online") { statusColor = "#2ecc71"; isGlowing = true; } 
        else if (user.status == "Busy" || user.status == "InGame") { statusColor = "#e74c3c"; isGlowing = true; } 
        else { statusColor = "#95a5a6"; isGlowing = false; }
    } else {
        statusColor = "#bdc3c7"; 
        isGlowing = false;
    }

    // --- A. AVATAR WRAPPER ---
    QWidget *avatarWrapper = new QWidget();
    avatarWrapper->setFixedSize(70, 70); 

    QLabel *lblAvatar = new QLabel(avatarWrapper);
    lblAvatar->setObjectName("lblAvatar");
    lblAvatar->setFixedSize(54, 54);
    lblAvatar->move(8, 8); 
    
    QStringList bgColors = {"#8e44ad", "#2c3e50", "#d35400", "#16a085", "#c0392b"};
    int hash = 0; for(QChar c : user.username) hash += c.unicode();
    QString myBg = bgColors[hash % bgColors.size()];

    lblAvatar->setText(user.username.left(1).toUpper());
    lblAvatar->setAlignment(Qt::AlignCenter);
    lblAvatar->setStyleSheet(QString(
        "background-color: %1; color: white; border-radius: 27px; "
        "font-weight: bold; font-size: 24px; border: 2px solid %2;"
    ).arg(myBg).arg(statusColor));

    if (isGlowing) {
        QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect(lblAvatar);
        glow->setBlurRadius(15); glow->setColor(QColor(statusColor)); glow->setOffset(0, 0);
        lblAvatar->setGraphicsEffect(glow);
    }

    if (user.isFriend) {
        QLabel *lblSmallDot = new QLabel(avatarWrapper);
        lblSmallDot->setObjectName("lblSmallDot");
        lblSmallDot->setFixedSize(16, 16);
        lblSmallDot->move(48, 48); 
        lblSmallDot->setStyleSheet(QString(
            "background-color: %1; border-radius: 8px; border: 2px solid #282838;"
        ).arg(statusColor));
    }

    // --- B. TEXT CONTAINER ---
    QWidget *textContainer = new QWidget();
    QVBoxLayout *textLayout = new QVBoxLayout(textContainer);
    textLayout->setAlignment(Qt::AlignVCenter);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(3);

    QLabel *lblName = new QLabel(user.username);
    lblName->setStyleSheet("color: white; font-weight: bold; font-size: 17px; background: transparent;");

    QString statusText = user.isFriend ? user.status : "Người chơi";
    
    QLabel *lblStatusText = new QLabel(statusText);
    lblStatusText->setObjectName("lblStatusText");
    lblStatusText->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 13px; background: transparent;").arg(statusColor));

    textLayout->addWidget(lblName);
    textLayout->addWidget(lblStatusText);

    contentLayout->addWidget(avatarWrapper, 0, Qt::AlignVCenter);
    contentLayout->addWidget(textContainer, 1, Qt::AlignVCenter);

    // --- C. XỬ LÝ NÚT KẾT BẠN vs CHẤM TRÒN ---
    if (!user.isFriend) {
        GameButton *btnAdd = new GameButton("+");
        btnAdd->setFixedSize(36, 36);
        btnAdd->setCursor(Qt::PointingHandCursor);
        
        btnAdd->setStyleSheet(
            "GameButton {"
            "   background-color: rgba(46, 204, 113, 0.15);" 
            "   color: #2ecc71;"
            "   border: 1px solid #2ecc71;"
            "   border-radius: 18px;"
            "   font-weight: 900;"
            "   font-size: 22px;"
            "   padding-bottom: 4px;"
            "}"
            "GameButton:hover {"
            "   background-color: #2ecc71;"
            "   color: white;"
            "}"
            "GameButton:pressed {"
            "   background-color: #27ae60;"
            "}"
        );

        connect(btnAdd, &GameButton::clicked, [=]() {
            GameClient::instance().sendAddFriendRequest(user.username);
            btnAdd->setText("✓"); 
            btnAdd->setEnabled(false); 
            btnAdd->setStyleSheet(
                "background-color: transparent; color: #bdc3c7; border: 1px solid #7f8c8d; border-radius: 18px; font-size: 18px;"
            );
        });

        contentLayout->addWidget(btnAdd, 0, Qt::AlignVCenter);

    } else {
        QLabel *lblRightDot = new QLabel();
        lblRightDot->setObjectName("lblRightDot");
        lblRightDot->setFixedSize(12, 12);
        lblRightDot->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(statusColor));

        if (isGlowing) {
            QGraphicsDropShadowEffect *glowRight = new QGraphicsDropShadowEffect(lblRightDot);
            glowRight->setBlurRadius(15); glowRight->setColor(QColor(statusColor)); glowRight->setOffset(0,0);
            lblRightDot->setGraphicsEffect(glowRight);
        }
        contentLayout->addWidget(lblRightDot, 0, Qt::AlignVCenter);
    }

    // =========================================================================
    // PHẦN 2: ĐƯỜNG KẺ NGANG
    // =========================================================================
    QWidget *lineSeparator = new QWidget();
    lineSeparator->setFixedHeight(1);
    lineSeparator->setStyleSheet("background-color: rgba(255, 255, 255, 0.1);"); 

    mainVLayout->addWidget(topContent, 1);
    mainVLayout->addWidget(lineSeparator);

    listFriends->setItemWidget(item, wid);
}