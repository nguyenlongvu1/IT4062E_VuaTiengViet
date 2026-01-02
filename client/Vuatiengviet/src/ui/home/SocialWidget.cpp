#include "SocialWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QLineEdit>
#include <../../utils/GameButton.h>
#include <QStackedWidget>
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
    // [SỬA LỖI TRÀN 2] Set kích thước cứng cho chính Widget này luôn
    // this->setFixedSize(450, 750);

    // =========================================================================
    // 1. KHUNG VIỀN PHÁT SÁNG (BORDER WIDGET)
    // =========================================================================
    QWidget *borderWidget = new QWidget(this);
    borderWidget->setObjectName("borderWidget");
    borderWidget->setFixedSize(450, 750); // Khớp hoàn toàn với kích thước widget
    
    // Gradient nền viền (Outer Glow)
    borderWidget->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1," 
        "   stop:0    rgba(255, 255, 255, 0),"   
        "   stop:0.5  rgba(255, 255, 255, 0.5)," // Giảm độ đậm chút để đỡ chói
        "   stop:1    rgba(255, 255, 255, 0));"
        "border-radius: 20px;" 
    );

    // Layout viền (Margin 2px = Độ dày viền sáng)
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2); 

    // =========================================================================
    // 2. KÍNH TỐI (GLASS PANEL)
    // =========================================================================
    QWidget *glassPanel = new QWidget(borderWidget);
    glassPanel->setObjectName("glassPanel");
    glassPanel->setStyleSheet(
        "#glassPanel {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 rgba(50, 50, 80, 0.95),"   
        "       stop:1 rgba(20, 20, 35, 0.98));"  
        "   border-radius: 20px;" // 38px (border) - 2px (margin) = 36px (khớp góc)
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
    // 3. ĐƯỜNG KẺ NEON DỌC (TÍNH TOÁN LẠI TỌA ĐỘ)
    // =========================================================================
    // Kích thước thực tế glassPanel: Rộng 446px (450 - 2 trái - 2 phải)
    // Chiều cao thực tế glassPanel: Cao 746px (750 - 2 trên - 2 dưới)

    // --- TRÁI ---
    QWidget *leftLine = new QWidget(glassPanel);
    leftLine->setStyleSheet(
       "background: qlineargradient(x1:0, y1:0, x2:0, y2:1," // Hướng DỌC (x giữ nguyên, y tăng)
        "    stop:0    rgba(255, 255, 255, 0),"   // Trên cùng: Trong suốt
        "    stop:0.2  rgba(255, 255, 255, 0.4)," // Hiện mờ
        "    stop:0.5  rgba(255, 255, 255, 1.0)," // GIỮA: Trắng sáng nhất (100%)
        "    stop:0.8  rgba(255, 255, 255, 0.4)," // Mờ dần
        "    stop:1    rgba(255, 255, 255, 0));"  // Dưới cùng: Trong suốt
        "border: none;"
    );
    QGraphicsDropShadowEffect *glowL = new QGraphicsDropShadowEffect(leftLine);
    glowL->setBlurRadius(80); 
    glowL->setColor(QColor(200, 230, 255, 255));
     glowL->setOffset(0,0);
    leftLine->setGraphicsEffect(glowL);
    
    // SetGeometry: x=0, y=40, rộng=3, cao=660 (thụt vào 40px mỗi đầu để tránh góc bo)
    leftLine->setGeometry(-1, 40, 2, 666); 

    // --- PHẢI ---
    QWidget *rightLine = new QWidget(glassPanel);
    rightLine->setStyleSheet(leftLine->styleSheet());
    QGraphicsDropShadowEffect *glowR = new QGraphicsDropShadowEffect(rightLine);
    glowR->setBlurRadius(80); glowR->setColor(QColor(200, 230, 255, 255)); glowR->setOffset(0,0);
    rightLine->setGraphicsEffect(glowR);
    
    // SetGeometry: x = 446 (width) - 3 (line width) = 443
    rightLine->setGeometry(445, 40, 1, 666);


    // =========================================================================
    // 4. NỘI DUNG BÊN TRONG (LAYOUT)
    // =========================================================================
    QVBoxLayout *mainLayout = new QVBoxLayout(glassPanel);
    // Tăng margin bên trong lên để nội dung không dính sát vào đường kẻ neon
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
    mainLayout->addSpacing(15);

    // [C] Tabs
    QWidget *tabContainer = new QWidget(glassPanel);
    QHBoxLayout *tabLayout = new QHBoxLayout(tabContainer);
    tabContainer->setStyleSheet(" border-radius: 10px; background-color: rgba(255, 255, 255, 0.22);");
    tabLayout->setContentsMargins(3, 3, 3, 3); tabLayout->setSpacing(10);

    btnTabFriend = new GameButton("Bạn Bè", glassPanel);
    btnTabRecent = new GameButton("Gần Đây", glassPanel);
    btnTabFriend->setFixedHeight(38);
    btnTabRecent->setFixedHeight(38);
    btnTabFriend->setCursor(Qt::PointingHandCursor);

    QString tabActive = 
        "GameButton {"
        "   background: qradialgradient("
        "       cx:0.5, cy:0.5, radius: 0.8, fx:0.5, fy:0.5,"
        "       stop:0    rgba(255, 255, 255, 0)," 
        
        "       stop:0.6  rgba(255, 255, 255, 0.15),"
        "       stop:1    rgba(255, 255, 255, 0.4));" 
        
        "   color: white;"
        "   border-radius: 10px;" // Bo tròn 19px (bằng một nửa chiều cao 38px)
        
        // Viền sáng bao quanh để làm nổi bật hiệu ứng glow
        "   border: 1px solid rgba(255, 255, 255, 0.5);"
        "   font-weight: bold;"
        "   font-size: 20px;"
        "}";

    QString tabInactive = 
        "GameButton {"
        "   background: transparent;"
        "   color: #bdc3c7;"
        "   border: none;"
        "   font-weight: 500;"
        "   font-size: 18px;"
        "}"
        "GameButton:hover { color: white; background-color: rgba(255,255,255,0.05); border-radius: 19px; }";

    btnTabFriend->setStyleSheet(tabActive);
    btnTabRecent->setStyleSheet(tabInactive);

    tabLayout->addWidget(btnTabFriend, 1);
    tabLayout->addWidget(btnTabRecent, 1);
    mainLayout->addWidget(tabContainer);

    // Kẻ ngang 2
    QWidget *sep2 = new QWidget(glassPanel); sep2->setFixedHeight(1);
    sep2->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 transparent, stop:0.5 rgba(255,255,255,0.3), stop:1 transparent);");
    mainLayout->addSpacing(15);
    mainLayout->addWidget(sep2);
    mainLayout->addSpacing(0);

    // [D] Danh sách (Stacked Widget)
    QStackedWidget *stackedWidget = new QStackedWidget(glassPanel);
    stackedWidget->setStyleSheet("background: transparent;");
    stackedWidget->setContentsMargins(0, 0, 0, 0);

    listFriends = new QListWidget();
    listFriends->setStyleSheet("QListWidget { background: transparent; border: none; outline: none; }");
    listFriends->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listFriends->setFocusPolicy(Qt::NoFocus);

    listRecent = new QListWidget();
    listRecent->setStyleSheet(listFriends->styleSheet());
    listRecent->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listRecent->setFocusPolicy(Qt::NoFocus);

    stackedWidget->addWidget(listFriends);
    stackedWidget->addWidget(listRecent);
    mainLayout->addWidget(stackedWidget);

    // --- LOGIC EVENTS (Giữ nguyên) ---
    connect(btnTabFriend, &GameButton::clicked, [=](){
        stackedWidget->setCurrentIndex(0);
        btnTabFriend->setStyleSheet(tabActive);
        btnTabRecent->setStyleSheet(tabInactive);
    });

    connect(btnTabRecent, &GameButton::clicked, [=](){
        stackedWidget->setCurrentIndex(1);
        btnTabFriend->setStyleSheet(tabInactive);
        btnTabRecent->setStyleSheet(tabActive);
    });
    
    // ... (Giữ các phần connect còn lại y nguyên) ...
    // Copy lại các đoạn connect search và server ở code cũ của bạn vào đây
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
// ... (Các hàm logic onSearchFriend, onSearchResultReceived giữ nguyên như cũ) ...

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
    // Nếu ô tìm kiếm rỗng thì không hiển thị kết quả tìm kiếm
    if (txtSearch->text().isEmpty()) return;
    
    listFriends->clear();

    // Duyệt qua danh sách kết quả trả về
    // [LƯU Ý] Dùng "UserSearchResult user" (copy) thay vì "const auto& user" 
    // để ta có thể sửa đổi thuộc tính isFriend của nó.
    for (UserSearchResult user : results) {
        
        // KIỂM TRA: Nếu tên người này nằm trong danh sách bạn bè đã cache
        if (m_friendCache.contains(user.username)) {
            user.isFriend = true; // -> Đánh dấu là bạn bè để hiện dấu chấm
        }

        // Loại bỏ chính mình (nếu Server lỡ gửi về)
        // Bạn cần đảm bảo GameClient có hàm lấy username hiện tại, hoặc lọc từ server
        // if (user.username == "tuphan2510") continue; 

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
    // PHẦN 1: NỘI DUNG CHÍNH
    // =========================================================================
    QWidget *topContent = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(topContent);
    contentLayout->setContentsMargins(10, 0, 15, 5); // Margin phải 15px để nút không sát lề
    contentLayout->setSpacing(10);

    // --- LOGIC MÀU SẮC ---
    QString statusColor;
    bool isGlowing = false;
    
    // Nếu là bạn bè thì mới tô màu theo trạng thái, còn người lạ thì mặc định xám
    if (user.isFriend) {
        if (user.status == "Online") { statusColor = "#2ecc71"; isGlowing = true; } 
        else if (user.status == "Busy" || user.status == "InGame") { statusColor = "#e74c3c"; isGlowing = true; } 
        else { statusColor = "#95a5a6"; isGlowing = false; }
    } else {
        // Người lạ: Avatar màu mặc định, không phát sáng
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

    // Chỉ hiện chấm nhỏ ở avatar nếu là bạn bè
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

    // Nếu là bạn bè thì hiện Status (Online/Offline), người lạ thì hiện "Người lạ" hoặc ẩn
    QString statusText = user.isFriend ? user.status : "Người chơi";
    
    QLabel *lblStatusText = new QLabel(statusText);
    lblStatusText->setObjectName("lblStatusText");
    lblStatusText->setStyleSheet(QString("color: %1; font-weight: 500; font-size: 13px; background: transparent;").arg(statusColor));

    textLayout->addWidget(lblName);
    textLayout->addWidget(lblStatusText);

    // Lắp Avatar và Text vào Layout
    contentLayout->addWidget(avatarWrapper, 0, Qt::AlignVCenter);
    contentLayout->addWidget(textContainer, 1, Qt::AlignVCenter);

    // =========================================================================
    // [QUAN TRỌNG] --- C. XỬ LÝ NÚT KẾT BẠN vs CHẤM TRÒN ---
    // =========================================================================
    
    if (!user.isFriend) {
        // --- TRƯỜNG HỢP 1: NGƯỜI LẠ -> HIỆN NÚT THÊM BẠN ---
        GameButton *btnAdd = new GameButton("+");
        btnAdd->setFixedSize(36, 36);
        btnAdd->setCursor(Qt::PointingHandCursor);
        
        // Style Neon Xanh lá
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

        // Logic gửi lời mời
        connect(btnAdd, &GameButton::clicked, [=]() {
            GameClient::instance().sendAddFriendRequest(user.username);
            btnAdd->setText("✓"); // Đổi thành dấu tích
            btnAdd->setEnabled(false); // Khóa nút
            btnAdd->setStyleSheet(
                "background-color: transparent; color: #bdc3c7; border: 1px solid #7f8c8d; border-radius: 18px; font-size: 18px;"
            );
        });

        contentLayout->addWidget(btnAdd, 0, Qt::AlignVCenter);

    } else {
        // --- TRƯỜNG HỢP 2: BẠN BÈ -> HIỆN CHẤM TRÒN ONLINE/OFFLINE ---
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