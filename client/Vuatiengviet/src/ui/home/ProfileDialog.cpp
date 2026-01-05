#include "ProfileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QFrame>


ProfileDialog::ProfileDialog(const QString &username, int score, const QString &rankName, QWidget *parent) 
    : QDialog(parent) 
{
    // 1. Cấu hình cửa sổ không viền
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(560, 680); // Tăng chiều cao xíu cho bảng thoáng

    // 2. Style Sheet chung
    setStyleSheet(
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); "
        "   border-radius: 10px; padding: 10px; font-weight: bold; color: white; border: 1px solid #c0392b;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6b6b, stop:1 #e74c3c); }"
    );
    
    setupUi(username, score, rankName);
}

void ProfileDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Gradient Xanh Than
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor("#0f2027"));
    gradient.setColorAt(0.5, QColor("#203a43"));
    gradient.setColorAt(1.0, QColor("#2c5364"));

    painter.setBrush(gradient);
    painter.setPen(QPen(QColor("#f1c40f"), 3));
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 15, 15);
}

void ProfileDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void ProfileDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void ProfileDialog::setupUi(const QString &username, int score, const QString &rankName) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(25, 30, 25, 30);
    layout->setSpacing(15);

    // Tiêu đề
    QLabel *lblTitleWindow = new QLabel("HỒ SƠ NGƯỜI CHƠI", this);
    lblTitleWindow->setAlignment(Qt::AlignCenter);
    lblTitleWindow->setStyleSheet("font-size: 24px; font-weight: 900; color: #f1c40f; background: transparent; margin-bottom: 5px;");
    layout->addWidget(lblTitleWindow);

    // --- PHẦN 1: PROFILE NEON FRAME (Giữ nguyên) ---
    QFrame *profileFrame = new QFrame(this);
    profileFrame->setFixedSize(500, 120);
    profileFrame->setAttribute(Qt::WA_TranslucentBackground);
    profileFrame->setStyleSheet("background: transparent; border: none;");

    QVBoxLayout *profileRootLayout = new QVBoxLayout(profileFrame);
    profileRootLayout->setContentsMargins(5, 5, 0, 0);
    profileRootLayout->setAlignment(Qt::AlignCenter);

    QWidget *borderWidget = new QWidget(profileFrame);
    borderWidget->setFixedSize(450, 100);
    borderWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255, 255, 255, 0), stop:0.5 rgba(255, 255, 255, 0.5), stop:1 rgba(255, 255, 255, 0)); border-radius: 20px;");

    QGraphicsDropShadowEffect *profileGlow = new QGraphicsDropShadowEffect(borderWidget);
    profileGlow->setBlurRadius(30);
    profileGlow->setColor(QColor(255, 255, 255, 150));
    profileGlow->setOffset(0, 0);
    borderWidget->setGraphicsEffect(profileGlow);
    
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2);

    QWidget *glassPanel = new QWidget(borderWidget);
    glassPanel->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(50, 50, 80, 0.95), stop:1 rgba(20, 20, 35, 0.98)); border-radius: 18px;");
    borderLayout->addWidget(glassPanel);
    profileRootLayout->addWidget(borderWidget);

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

    QLabel *lblAvatar = new QLabel(this);
    lblAvatar->setText(username.left(1).toUpper());
    lblAvatar->setFixedSize(60, 60);
    lblAvatar->setAlignment(Qt::AlignCenter);
    lblAvatar->setStyleSheet("background-color: #8e44ad; color: white; border-radius: 30px; font-weight: bold; font-size: 28px; border: 2px solid #a29bfe;");

    QVBoxLayout *infoTextLayout = new QVBoxLayout();
    infoTextLayout->setSpacing(2);
    infoTextLayout->setAlignment(Qt::AlignVCenter);

    QLabel *lblUsername = new QLabel(username, this);
    lblUsername->setStyleSheet("color: white; font-weight: 900; font-size: 20px; background: transparent;");

    QLabel *lblRank = new QLabel(QString("Rank: %1").arg(rankName), this);
    lblRank->setStyleSheet("color: #f1c40f; font-size: 14px; font-weight: bold; background: transparent;");

    int wins = 15;
    int losses = 5;
    QLabel *lblStats = new QLabel(QString("Điểm: %1   |   Thắng: %2   |   Thua: %3").arg(score).arg(wins).arg(losses), this);
    lblStats->setStyleSheet("color: #bdc3c7; font-size: 13px; font-weight: 500; background: transparent;");

    infoTextLayout->addWidget(lblUsername);
    infoTextLayout->addWidget(lblRank);
    infoTextLayout->addWidget(lblStats);

    contentLayout->addWidget(lblAvatar);
    contentLayout->addLayout(infoTextLayout);
    contentLayout->addStretch();

    layout->addWidget(profileFrame, 0, Qt::AlignCenter);

    // --- PHẦN 2: LỊCH SỬ ĐẤU (ĐÃ NÂNG CẤP) ---
    QLabel *lblHistoryTitle = new QLabel("Lịch sử đấu gần đây", this);
    lblHistoryTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #f1c40f; background: transparent; margin-top: 5px;");
    layout->addWidget(lblHistoryTitle);

    QTableWidget *table = new QTableWidget(10, 3);
    table->setHorizontalHeaderLabels({"Thời gian", "Đối thủ", "Kết quả"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setShowGrid(false); // Tắt lưới mặc định để dùng border css
    table->setAlternatingRowColors(false); 
    
    // Tăng chiều cao dòng cho thoáng
    table->verticalHeader()->setDefaultSectionSize(45); 

    // --- Style Bảng "Sạch & Đẹp" ---
    table->setStyleSheet(
        "QTableWidget { "
        "   background-color: white; "      
        "   border-radius: 12px; "          
        "   color: #34495e; "               // Chữ màu xám đậm (dễ đọc)
        "   border: none;"
        "   outline: 0;"
        "}"
        "QHeaderView::section { "
        "   background-color: #ecf0f1; "    // Header màu xám khói sang trọng
        "   color: #7f8c8d; "               // Chữ header màu ghi
        "   font-weight: bold; "
        "   font-size: 13px; "
        "   border: none; "
        "   padding: 8px;"
        "}"
        "QTableWidget::item { "
        "   border-bottom: 1px solid #f5f6fa; " // Đường kẻ mờ ngăn cách các dòng
        "   padding-left: 10px;"
        "}"
        // Style thanh cuộn (Scrollbar) nhỏ gọn
        "QScrollBar:vertical { background: #f1f2f6; width: 8px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #bdc3c7; min-height: 20px; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    for(int i=0; i<10; i++) {
        table->setItem(i, 0, new QTableWidgetItem("20/12 10:30"));
        table->setItem(i, 1, new QTableWidgetItem(QString("DoiThu_%1").arg(i+1)));
        
        // Logic màu chữ Thắng/Thua
        bool isWin = (i % 2 == 0);
        QTableWidgetItem *resItem = new QTableWidgetItem(isWin ? "THẮNG (+25)" : "THUA (-15)");
        
        // Màu chữ Xanh/Đỏ tươi hơn một chút
        resItem->setForeground(isWin ? QColor("#2ecc71") : QColor("#e74c3c"));
        resItem->setFont(QFont("Segoe UI", 10, QFont::Bold)); // Font hiện đại hơn
        resItem->setTextAlignment(Qt::AlignCenter);
        
        table->setItem(i, 2, resItem);
    }
    layout->addWidget(table);

    // Nút đóng
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnClose = new QPushButton("Đóng", this);
    btnClose->setFixedWidth(120);
    btnClose->setCursor(Qt::PointingHandCursor);
    connect(btnClose, &QPushButton::clicked, this, &ProfileDialog::accept);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
}