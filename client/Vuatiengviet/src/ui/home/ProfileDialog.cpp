#include "ProfileDialog.h"
#include "../network/GameClient.h" // Nhớ include
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QScrollArea>
#include <QHeaderView>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>

ProfileDialog::ProfileDialog(const QString &username, int score, const QString &rankName, QWidget *parent) 
    : QDialog(parent) 
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(600, 700); // Tăng chiều rộng chút để đủ 4 cột

    setStyleSheet(
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); "
        "   border-radius: 10px; padding: 10px; font-weight: bold; color: white; border: 1px solid #c0392b;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6b6b, stop:1 #e74c3c); }"
    );
    
    setupUi(username, score, rankName);

    // 3. KẾT NỐI SERVER ĐỂ LẤY DỮ LIỆU THẬT
    connect(&GameClient::instance(), &GameClient::historyReceived, this, &ProfileDialog::onHistoryReceived);
    connect(&GameClient::instance(), &GameClient::matchLogReceived, this, &ProfileDialog::onMatchLogReceived);
    // Gửi yêu cầu ngay khi mở Dialog
    GameClient::instance().sendGetHistory();
}

// ... (Giữ nguyên paintEvent, mousePressEvent, mouseMoveEvent) ...
void ProfileDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
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

    // --- PHẦN 1: PROFILE NEON FRAME (Giữ nguyên code cũ của bạn) ---
    QFrame *profileFrame = new QFrame(this);
    profileFrame->setFixedSize(540, 120); // Tăng size frame
    profileFrame->setAttribute(Qt::WA_TranslucentBackground);
    profileFrame->setStyleSheet("background: transparent; border: none;");

    QVBoxLayout *profileRootLayout = new QVBoxLayout(profileFrame);
    profileRootLayout->setContentsMargins(5, 5, 0, 0);
    profileRootLayout->setAlignment(Qt::AlignCenter);

    QWidget *borderWidget = new QWidget(profileFrame);
    borderWidget->setFixedSize(500, 100);
    borderWidget->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255, 255, 255, 0), stop:0.5 rgba(255, 255, 255, 0.5), stop:1 rgba(255, 255, 255, 0)); border-radius: 20px;");

    QGraphicsDropShadowEffect *profileGlow = new QGraphicsDropShadowEffect(borderWidget);
    profileGlow->setBlurRadius(30); profileGlow->setColor(QColor(255, 255, 255, 150)); profileGlow->setOffset(0, 0);
    borderWidget->setGraphicsEffect(profileGlow);
    
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2);

    QWidget *glassPanel = new QWidget(borderWidget);
    glassPanel->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(50, 50, 80, 0.95), stop:1 rgba(20, 20, 35, 0.98)); border-radius: 18px;");
    borderLayout->addWidget(glassPanel);
    profileRootLayout->addWidget(borderWidget);

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
    QLabel *lblStats = new QLabel(QString("Tổng điểm tích lũy: %1").arg(score), this);
    lblStats->setStyleSheet("color: #bdc3c7; font-size: 13px; font-weight: 500; background: transparent;");

    infoTextLayout->addWidget(lblUsername);
    infoTextLayout->addWidget(lblRank);
    infoTextLayout->addWidget(lblStats);

    contentLayout->addWidget(lblAvatar);
    contentLayout->addLayout(infoTextLayout);
    contentLayout->addStretch();
    layout->addWidget(profileFrame, 0, Qt::AlignCenter);

    // --- PHẦN 2: LỊCH SỬ ĐẤU (TABLE) ---
    QLabel *lblHistoryTitle = new QLabel("Lịch sử lượt chơi (50 lượt gần nhất)", this);
    lblHistoryTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #f1c40f; background: transparent; margin-top: 5px;");
    layout->addWidget(lblHistoryTitle);

    // [THAY ĐỔI] Tăng số cột lên 4 để chứa nút Replay
    m_historyTable = new QTableWidget(0, 4, this); 
    m_historyTable->setHorizontalHeaderLabels({"ID Trận", "Biến động", "Chi tiết", "Xoá"});
    
    // Cấu hình Header
    QHeaderView *header = m_historyTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Vòng // Điểm
    header->setSectionResizeMode(1, QHeaderView::Stretch);          // Đáp án (Giãn hết cỡ)
    header->setSectionResizeMode(2, QHeaderView::Fixed);            // Nút Replay
    header->setSectionResizeMode(3, QHeaderView::Fixed);            // Nút Xoá
    m_historyTable->setColumnWidth(2, 80);

    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_historyTable->setFocusPolicy(Qt::NoFocus);
    m_historyTable->setShowGrid(false);
    m_historyTable->setAlternatingRowColors(false); 
    m_historyTable->verticalHeader()->setDefaultSectionSize(50); // Chiều cao dòng

    m_historyTable->setStyleSheet(
        "QTableWidget { background-color: white; border-radius: 12px; color: #34495e; border: none; outline: 0; }"
        "QHeaderView::section { background-color: #ecf0f1; color: #7f8c8d; font-weight: bold; font-size: 13px; border: none; padding: 8px; }"
        "QTableWidget::item { border-bottom: 1px solid #f5f6fa; padding-left: 10px; }"
        "QScrollBar:vertical { background: #f1f2f6; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #bdc3c7; min-height: 20px; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    layout->addWidget(m_historyTable);

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

void ProfileDialog::onHistoryReceived(const QString &data) {
    m_historyTable->setRowCount(0); 

    if (data.isEmpty() || data == "EMPTY") return;

    QStringList matches = data.split("\n", Qt::SkipEmptyParts);

    for (const QString &matchStr : matches) {
        QStringList parts = matchStr.split("|"); 
        if (parts.size() < 5) continue; 

        QString matchId = parts[0];
        bool exists = false;
        for (int i = 0; i < m_historyTable->rowCount(); ++i) {
            if (m_historyTable->item(i, 0)->text() ==  matchId) {
                exists = true;
                break;
            }
        }
        if (exists) continue;
        int scoreVal = parts[4].toInt();

        // CHÌA KHÓA: insertRow(0) để đẩy bản ghi mới nhất lên trên cùng
       int currentRow = m_historyTable->rowCount();
        m_historyTable->insertRow(currentRow);
        // int row = 0; 

        // CỘT 1: ID TRẬN
        QTableWidgetItem *itemId = new QTableWidgetItem("#" + matchId);
        itemId->setTextAlignment(Qt::AlignCenter);
        itemId->setForeground(QColor("#7f8c8d"));
        m_historyTable->setItem(currentRow, 0, itemId);

        // CỘT 2: BIẾN ĐỘNG
        QString scorePrefix = (scoreVal > 0 ? "+" : "");
        QTableWidgetItem *itemScore = new QTableWidgetItem(scorePrefix + QString::number(scoreVal) + " Điểm");
        itemScore->setTextAlignment(Qt::AlignCenter);
        itemScore->setFont(QFont("Segoe UI", 10, QFont::Bold));
        
        if (scoreVal > 0) itemScore->setForeground(QColor("#2ecc71"));
        else if (scoreVal < 0) itemScore->setForeground(QColor("#e74c3c"));
        else itemScore->setForeground(QColor("#95a5a6"));
        m_historyTable->setItem(currentRow, 1, itemScore);

        // CỘT 3: NÚT CHI TIẾT
        QPushButton* btnDetail = new QPushButton("Xem lại");
        btnDetail->setCursor(Qt::PointingHandCursor);
        btnDetail->setStyleSheet(
            "QPushButton { background: #34495e; color: white; border-radius: 5px; font-size: 10px; padding: 4px; }"
            "QPushButton:hover { background: #2c3e50; }"
        );
        m_historyTable->setCellWidget(currentRow, 2, btnDetail);
        connect(btnDetail, &QPushButton::clicked, [=]() {
            GameClient::instance().sendGetMatchLog(matchId.toInt());
        });

        // CỘT 4: NÚT XÓA (MỚI)
        QPushButton* btnDelete = new QPushButton("XÓA");
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet(
            "QPushButton { background: #c0392b; color: white; border-radius: 5px; font-size: 10px; font-weight: bold; }"
            "QPushButton:hover { background: #e74c3c; }"
        );
        m_historyTable->setCellWidget(currentRow, 3, btnDelete);
        
        // Logic khi bấm xóa
        connect(btnDelete, &QPushButton::clicked, [=]() {
            auto reply = QMessageBox::question(this, "Xác nhận", "Bạn có chắc muốn xóa lịch sử trận này?", 
                                               QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                // 1. Gửi lệnh xóa lên Server (Bạn cần định nghĩa hàm này trong GameClient)
                // GameClient::instance().sendDeleteHistory(matchId.toInt());
                
                // 2. Xóa dòng đó trên giao diện ngay lập tức
                // Lưu ý: Row có thể thay đổi nên ta tìm lại row hiện tại của widget
                int currentRow = -1;
                for(int i=0; i < m_historyTable->rowCount(); ++i) {
                    if(m_historyTable->cellWidget(i, 3) == btnDelete) {
                        currentRow = i;
                        break;
                    }
                }
                if(currentRow != -1) m_historyTable->removeRow(currentRow);
            }
        });
    }
}

void ProfileDialog::onMatchLogReceived(const QString &data) {
    if (data.isEmpty() || data == "EMPTY") {
        QMessageBox::information(this, "Thông báo", "Không có dữ liệu.");
        return;
    }

    QStringList rows = data.split("\n", Qt::SkipEmptyParts);
    
    // Tạo bảng HTML
    QString html = "<h3>CHI TIẾT TRẬN ĐẤU</h3>";
    html += "<table width='100%' border='1' style='border-collapse: collapse; text-align: center;'>";
    html += "<tr style='background-color: #652b2b;'>"
            "<th>Vòng</th><th>Câu trả lời</th><th>Kết quả</th><th>Điểm</th>"
            "</tr>";

    for (const QString &rowStr : rows) {
        QStringList cols = rowStr.split("|");
        if (cols.size() < 4) continue;

        QString resultColor = (cols[2] == "ĐÚNG") ? "#27ae60" : "#e74c3c";
        
        html += QString("<tr>"
                        "<td>%1</td>"
                        "<td>%2</td>"
                        "<td style='color: %3; font-weight: bold;'>%4</td>"
                        "<td>%5</td>"
                        "</tr>")
                .arg(cols[0])  // Round
                .arg(cols[1])  // Answer
                .arg(resultColor) 
                .arg(cols[2])  // Đúng/Sai
                .arg(cols[3]); // Points
    }
    html += "</table>";

    // Hiển thị nhanh bằng một Dialog nhỏ
    QDialog *detailDlg = new QDialog(this);
    detailDlg->setWindowTitle("Diễn biến");
    detailDlg->setMinimumWidth(400);
    
    QVBoxLayout *layout = new QVBoxLayout(detailDlg);
    QLabel *lbl = new QLabel(html);
    lbl->setWordWrap(true);
    
    // Dùng ScrollArea đề phòng quá nhiều dòng (như 60 dòng của bạn)
    QScrollArea *sa = new QScrollArea;
    sa->setWidget(lbl);
    sa->setWidgetResizable(true);
    
    layout->addWidget(sa);
    QPushButton *btn = new QPushButton("Đóng");
    connect(btn, &QPushButton::clicked, detailDlg, &QDialog::accept);
    layout->addWidget(btn);
    
    detailDlg->exec();
}