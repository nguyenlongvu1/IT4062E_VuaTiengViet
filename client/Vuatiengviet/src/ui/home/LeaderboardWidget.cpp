// src/ui/home/LeaderboardWidget.cpp

#include "LeaderboardWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QTimer>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
LeaderboardWidget::LeaderboardWidget(QWidget *parent) : QWidget(parent) {
    // [QUAN TRỌNG] Bật tính năng vẽ nền để hiển thị màu trong suốt
    setAttribute(Qt::WA_StyledBackground, true); 
    setAttribute(Qt::WA_TranslucentBackground, true);
    this->setStyleSheet("background: transparent;");
    
    
    setupUi();

    // 1. Kết nối với GameClient để nhận dữ liệu
    connect(&GameClient::instance(), &GameClient::leaderboardReceived, 
            this, &LeaderboardWidget::updateLeaderboard);

    // 2. Gửi yêu cầu lấy dữ liệu sau 500ms để UI kịp khởi tạo
    QTimer::singleShot(500, [=](){
        GameClient::instance().sendGetLeaderboardRequest();
    });
}

// src/ui/home/LeaderboardWidget.cpp

// ... (Includes giữ nguyên)

void LeaderboardWidget::setupUi() {
    // 1. ROOT LAYOUT
    // Cần margin 30px để chứa hiệu ứng tỏa sáng (Glow) không bị cắt
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(30, 30, 30, 30); 
    
    // [QUAN TRỌNG] Không set fixed size cho Widget tổng (this), để nó tự giãn theo margin
    // this->setFixedSize(450, 750); 

    // =========================================================================
    // LỚP 1: KHUNG VIỀN PHÁT SÁNG (BORDER WIDGET)
    // =========================================================================
    QWidget *borderWidget = new QWidget(this);
    borderWidget->setObjectName("borderWidget");
    borderWidget->setFixedSize(450, 750); // Kích thước cố định đặt ở đây

    // Style Gradient nền viền (Tạo hiệu ứng viền sáng mờ)
    borderWidget->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1," 
        "   stop:0    rgba(255, 255, 255, 0),"   
        "   stop:0.5  rgba(255, 255, 255, 0.5)," // Viền sáng nhẹ ở giữa
        "   stop:1    rgba(255, 255, 255, 0));"
        "border-radius: 20px;" 
    );

    // Hiệu ứng bóng đổ trắng/sáng (Outer Glow)
    QGraphicsDropShadowEffect *panelShadow = new QGraphicsDropShadowEffect(borderWidget);
    panelShadow->setBlurRadius(30);
    panelShadow->setColor(QColor(255, 255, 255, 180)); 
    panelShadow->setOffset(0, 0);
    borderWidget->setGraphicsEffect(panelShadow);

    // Layout của viền (Margin 2px để lộ viền sáng)
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2); 
    
    // Add borderWidget vào giữa màn hình
    rootLayout->addWidget(borderWidget, 0, Qt::AlignCenter);


    // =========================================================================
    // LỚP 2: KÍNH TỐI (GLASS PANEL) - NẰM BÊN TRONG BORDER
    // =========================================================================
    QWidget *glassPanel = new QWidget(borderWidget);
    glassPanel->setObjectName("glassPanel");
    
    // [STYLE DARK GLASS] Giống hệt SocialWidget
    glassPanel->setStyleSheet(
        "#glassPanel {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 rgba(13, 13, 82, 1),"   // Tối xanh ở trên
        "       stop:1 rgba(20, 20, 35, 0.75));"  // Đen tím ở dưới
        "   border-radius: 20px;"
        "   border: none;"
        "}"
    );
    
    // Thêm glassPanel vào borderLayout
    borderLayout->addWidget(glassPanel);


    // =========================================================================
    // PHẦN 3: CÁC HIỆU ỨNG TRANG TRÍ (ĐƯỜNG KẺ NEON)
    // =========================================================================
    
    // --- ĐƯỜNG KẺ DỌC TRÁI ---
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
    glowL->setOffset(0, 0);   
    leftLine->setGraphicsEffect(glowL);
    // Vị trí: x=0, y=40, rộng=2, cao=666
    leftLine->setGeometry(0, 40, 2, 666); 

    // --- ĐƯỜNG KẺ DỌC PHẢI ---
    QWidget *rightLine = new QWidget(glassPanel);
    rightLine->setStyleSheet(leftLine->styleSheet());
    QGraphicsDropShadowEffect *glowR = new QGraphicsDropShadowEffect(rightLine);
    glowR->setBlurRadius(80); 
    glowR->setColor(QColor(200, 230, 255, 255)); 
    glowR->setOffset(0, 0);
    rightLine->setGraphicsEffect(glowR);
    // Vị trí: x=448 (450-2), y=40
    rightLine->setGeometry(448, 40, 2, 666);


    // =========================================================================
    // PHẦN 4: NỘI DUNG CHÍNH (TIÊU ĐỀ & DANH SÁCH)
    // =========================================================================
    QVBoxLayout *mainLayout = new QVBoxLayout(glassPanel);
    mainLayout->setContentsMargins(25, 25, 25, 25); // Margin bên trong kính
    mainLayout->setSpacing(15);

    // [A] Tiêu đề
    QLabel *lblTitle = new QLabel("BẢNG XẾP HẠNG", glassPanel);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setFixedHeight(40); 
    lblTitle->setStyleSheet("font-weight: bold; font-size: 25px; color: white; letter-spacing: 1px; background: transparent;");
    mainLayout->addWidget(lblTitle);

    // [B] Kẻ ngang phân cách
    QWidget *separator = new QWidget(glassPanel);
    separator->setFixedHeight(2); 
    separator->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:0," 
        "    stop:0    rgba(255, 255, 255, 0),"   
        "    stop:0.2  rgba(255, 255, 255, 0.1)," 
        "    stop:0.5  rgba(255, 255, 255, 1.0)," 
        "    stop:0.8  rgba(255, 255, 255, 0.1)," 
        "    stop:1    rgba(255, 255, 255, 0));"  
    );
    mainLayout->addWidget(separator);

    // [C] Danh sách xếp hạng
    listRank = new QListWidget(glassPanel);
    listRank->setStyleSheet(
        "QListWidget, QListWidget::viewport { background: transparent; border: none; }"
    );
    listRank->setFocusPolicy(Qt::NoFocus);
    listRank->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
    listRank->setSelectionMode(QAbstractItemView::NoSelection);

    mainLayout->addWidget(listRank);
}
QWidget* LeaderboardWidget::createRankItemWidget(const RankItem &data, int index) {
    QWidget *itemContainer = new QWidget();
    itemContainer->setFixedHeight(70);
    itemContainer->setStyleSheet(
    "background-color: transparent; border: none;"
);


    QWidget *contentWidget = new QWidget(itemContainer);
    QHBoxLayout *outerLayout = new QHBoxLayout(itemContainer);
    outerLayout->setContentsMargins(0, 5, 5, 5);
    outerLayout->addWidget(contentWidget);


    QHBoxLayout *layout = new QHBoxLayout(contentWidget);
    layout->setContentsMargins(5, 0, 15, 0);
    layout->setSpacing(10);

    int rankNum = index + 1;

    // --- 1. CỘT HẠNG (Giữ nguyên) ---
    QLabel *lblRank = new QLabel();
    lblRank->setFixedSize(50, 50);
    lblRank->setAlignment(Qt::AlignCenter);
    // ... (Code xử lý icon/số hạng như cũ) ...
    if (index < 3) {
        QString imgPath = (index == 0) ? ":/top1.png" : ((index == 1) ? ":/top2.png" : ":/top3.png");
        QPixmap pix(imgPath);
        if (!pix.isNull()) {
            lblRank->setPixmap(pix.scaled(45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            lblRank->setText(QString::number(rankNum)+".");
            QString color = (index == 0) ? "#f1c40f" : ((index == 1) ? "white" : "#e67e22");
            lblRank->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 24px;").arg(color));
        }
    } else {
        lblRank->setText(QString::number(rankNum) + ".");
        lblRank->setStyleSheet("color: white; font-weight: bold; font-size: 18px;");
    }
    if(!lblRank->styleSheet().contains("border: none")) 
        lblRank->setStyleSheet(lblRank->styleSheet() + "background: transparent; border: none;");


    // =================================================================
    // [SỬA] CỘT AVATAR - TẤT CẢ ĐỀU PHÁT SÁNG
    // =================================================================
    QLabel *lblAvatar = new QLabel();
    lblAvatar->setFixedSize(42, 42);
    QString bgCol = (data.name.length() % 2 == 0) ? "#9b59b6" : "#3498db";
    
    // [SỬA 1] Tất cả avatar đều có viền vàng (không chỉ Top 1)
    QString borderStyle = "border: 2px solid #f1c40f;"; 
    
    lblAvatar->setText(data.name.left(1).toUpper());
    lblAvatar->setAlignment(Qt::AlignCenter);
    lblAvatar->setStyleSheet(QString(
        "background-color: %1; color: white; border-radius: 21px; "
        "font-weight: bold; font-size: 18px; %2"
    ).arg(bgCol).arg(borderStyle));

    // [SỬA 2] Thêm hiệu ứng phát sáng (Glow) cho TẤT CẢ avatar
    QGraphicsDropShadowEffect *avatarGlow = new QGraphicsDropShadowEffect;
    avatarGlow->setBlurRadius(15); // Độ nhòe
    avatarGlow->setColor(QColor(255, 215, 0, 180)); // Màu vàng sáng
    avatarGlow->setOffset(0, 0); // Tỏa đều
    lblAvatar->setGraphicsEffect(avatarGlow);
    // =================================================================


    // --- 3. CỘT THÔNG TIN (Giữ nguyên) ---
    QWidget *infoWidget = new QWidget();
    infoWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(5, 0, 0, 0);
    infoLayout->setSpacing(0);
    infoLayout->setAlignment(Qt::AlignVCenter);
    // ... (Code tên và rank title như cũ) ...
    QLabel *lblName = new QLabel(data.name);
    if (index == 0) lblName->setStyleSheet("color: #f1c40f; font-weight: 900; font-size: 21px; border: none;");
    else lblName->setStyleSheet("color: white; font-weight: bold; font-size: 20px; border: none;");
    QLabel *lblRankTitle = new QLabel(data.rank);
    lblRankTitle->setStyleSheet("color: #bdc3c7; font-size: 18px; border: none;");
    infoLayout->addWidget(lblName);
    infoLayout->addWidget(lblRankTitle);

    // --- 4. CỘT ĐIỂM SỐ (Giữ nguyên) ---
    QLabel *lblScore = new QLabel(QString("%1").arg(data.score));
    lblScore->setFixedWidth(80);
    lblScore->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QString scoreColor = (index == 0) ? "#f1c40f" : "#ffffffff";
    lblScore->setStyleSheet(QString("color: %1; font-weight: 900; font-size: 25px; background: transparent; border: none;").arg(scoreColor));

    // --- LẮP RÁP ---
    layout->addWidget(lblRank);
    layout->addWidget(lblAvatar);
    layout->addWidget(infoWidget, 1);
    layout->addWidget(lblScore);

    // --- HIỆU ỨNG NỀN CHO DÒNG (Giữ nguyên) ---
    if (index == 0) {
        contentWidget->setStyleSheet(
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 215, 0, 0.25), stop:1 rgba(241, 196, 15, 0.1)); "
            "border: 2px solid rgba(255, 215, 0, 0.8); border-radius: 15px;"
        );
        QGraphicsDropShadowEffect *glowEffect = new QGraphicsDropShadowEffect;
        glowEffect->setBlurRadius(30); glowEffect->setColor(QColor(255, 215, 0, 150)); glowEffect->setOffset(0, 0);
        contentWidget->setGraphicsEffect(glowEffect);
        QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect;
        shadowEffect->setBlurRadius(15); shadowEffect->setColor(QColor(0, 0, 0, 80)); shadowEffect->setOffset(0, 5);
        itemContainer->setGraphicsEffect(shadowEffect);
    }
    else if (index == 1) {
        contentWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,0.15), stop:1 rgba(255,255,255,0.05)); border: 1px solid rgba(255,255,255,0.3); border-radius: 15px;");
    }
    else if (index == 2) {
        contentWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(230,126,34,0.15), stop:1 rgba(230,126,34,0.05)); border: 1px solid rgba(230,126,34,0.3); border-radius: 15px;");
    }
    else {
        contentWidget->setStyleSheet("background: transparent; border: none;");
    }

    return itemContainer;
}

void LeaderboardWidget::updateLeaderboard(const QList<RankItem> &items) {
    listRank->clear();

    for(int i = 0; i < items.size(); ++i) {
        QListWidgetItem *itemPlaceholder = new QListWidgetItem(listRank);
        QWidget *customWidget = createRankItemWidget(items[i], i);
        
        // [CỰC KỲ QUAN TRỌNG] 
        // 64px * 10 dòng = 640px. 
        // Tổng chiều cao widget 750px - 50px (Tiêu đề) - 40px (Margin) = 660px còn trống.
        // Set 64px là vừa đẹp.
        itemPlaceholder->setSizeHint(QSize(0, 60)); 

        listRank->setItemWidget(itemPlaceholder, customWidget);
    }
}