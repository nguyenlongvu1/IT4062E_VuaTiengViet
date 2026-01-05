#include "NotificationDialog.h"
#include "../../network/GameClient.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QPainter>      // Cần thiết để vẽ nền
#include <QMouseEvent>   // Cần thiết để xử lý chuột
#include <QScrollBar>    // Cần thiết để style thanh cuộn

NotificationDialog::NotificationDialog(QWidget *parent) : QDialog(parent) {
    // 1. Cấu hình cửa sổ không viền, nền trong suốt
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(400, 550); // Tăng chiều cao chút cho đẹp
    
    setupUi();
}

// --- PHẦN VẼ GIAO DIỆN & KÉO THẢ (GIỐNG PROFILE DIALOG) ---
void NotificationDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Gradient nền (Tông Tím Than - Xanh Đậm)
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor("#141E30"));
    gradient.setColorAt(1.0, QColor("#243B55"));

    painter.setBrush(gradient);
    // Viền màu Vàng Gold (#f1c40f) để nhấn mạnh đây là Thông Báo
    painter.setPen(QPen(QColor("#f1c40f"), 2)); 
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 15, 15);
}

void NotificationDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void NotificationDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
// ---------------------------------------------------------
void NotificationDialog::checkCount() {
    emit notificationCountChanged(listNoti->count());
}
void NotificationDialog::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    
    // Header
    QLabel *lblTitle = new QLabel("🔔 HỘP THƯ THÔNG BÁO", this);
    lblTitle->setStyleSheet("background: transparent; font-size: 18px; font-weight: 900; color: #f1c40f; letter-spacing: 1px;");
    lblTitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(lblTitle);

    // List Widget
    listNoti = new QListWidget(this);
    // Style cho ListWidget và Thanh cuộn (Scrollbar)
    listNoti->setStyleSheet(
        "QListWidget { background: rgba(0,0,0,0.2); border-radius: 10px; border: 1px solid rgba(255,255,255,0.1); outline: none; padding: 5px; }"
        // Style thanh cuộn dọc
        "QScrollBar:vertical { background: #1e272e; width: 8px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #7f8c8d; min-height: 20px; border-radius: 4px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );
    listNoti->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    layout->addWidget(listNoti);

    // Nút Đóng (Style đỏ gradient giống các dialog khác)
    QPushButton *btnClose = new QPushButton("Đóng", this);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setFixedHeight(40);
    btnClose->setStyleSheet(
        "QPushButton { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); "
        "   border-radius: 8px; font-weight: bold; color: white; border: 1px solid #c0392b;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6b6b, stop:1 #e74c3c); }"
    );
    
    layout->addWidget(btnClose);
    connect(btnClose, &QPushButton::clicked, this, &NotificationDialog::hide); 
}

void NotificationDialog::addFriendRequest(const QString &senderName) {
    // KIỂM TRA TRÙNG LẶP
    for(int i = 0; i < listNoti->count(); ++i) {
        QListWidgetItem* existingItem = listNoti->item(i);
        if (existingItem->data(Qt::UserRole).toString() == senderName) {
            return; 
        }
    }

    QListWidgetItem *listWItem = new QListWidgetItem(listNoti);
    listWItem->setSizeHint(QSize(listNoti->width() - 40, 100)); // Tăng chiều cao item chút
    listWItem->setData(Qt::UserRole, senderName); 

    QWidget *wid = new QWidget();
    // Style cho từng item (Card effect)
    wid->setStyleSheet(
        "QWidget { "
        "   background-color: rgba(255, 255, 255, 0.05); " // Trong suốt nhẹ
        "   border-radius: 8px; "
        "   border: 1px solid rgba(255, 255, 255, 0.1);"
        "}"
    );
    
    QVBoxLayout *vLayout = new QVBoxLayout(wid);
    vLayout->setContentsMargins(10, 10, 10, 10);
    
    // Label nội dung
    QLabel *lblMsg = new QLabel(QString("<span style='color:#3498db; font-weight:bold;'>%1</span> muốn kết bạn với bạn.").arg(senderName));
    lblMsg->setStyleSheet("background: transparent; border: none; color: #ecf0f1; font-size: 14px;"); // Reset border cho label
    lblMsg->setWordWrap(true);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    // Nút Đồng ý (Xanh lá)
    QPushButton *btnAccept = new QPushButton("Đồng ý");
    btnAccept->setCursor(Qt::PointingHandCursor);
    btnAccept->setStyleSheet(
        "QPushButton { background-color: #2ecc71; border: none; padding: 6px; border-radius: 5px; color: white; font-weight: bold; }"
        "QPushButton:hover { background-color: #27ae60; }"
    );
    
    // Nút Từ chối (Xám hoặc Đỏ nhạt)
    QPushButton *btnDecline = new QPushButton("Từ chối");
    btnDecline->setCursor(Qt::PointingHandCursor);
    btnDecline->setStyleSheet(
        "QPushButton { background-color: rgba(231, 76, 60, 0.8); border: none; padding: 6px; border-radius: 5px; color: white; font-weight: bold; }"
        "QPushButton:hover { background-color: #c0392b; }"
    );

    btnLayout->addWidget(btnAccept);
    btnLayout->addWidget(btnDecline);

    vLayout->addWidget(lblMsg);
    vLayout->addLayout(btnLayout);

    listNoti->setItemWidget(listWItem, wid);

    // Logic xử lý nút
    connect(btnAccept, &QPushButton::clicked, [=](){
        btnAccept->setText("Đã đồng ý");
        btnAccept->setStyleSheet("background-color: #27ae60; color: white; border: none; padding: 6px; border-radius: 5px;");
        btnAccept->setEnabled(false);
        btnDecline->hide();
        
        GameClient::instance().sendAcceptFriend(senderName);
        
        QTimer::singleShot(1000, [=](){
             if(listNoti->row(listWItem) >= 0) {
                 delete listNoti->takeItem(listNoti->row(listWItem));
                 checkCount(); // <--- BÁO CHO PARENT BIẾT ĐÃ GIẢM SỐ LƯỢNG
             }
        });
    });

    connect(btnDecline, &QPushButton::clicked, [=](){
        if(listNoti->row(listWItem) >= 0) {
            delete listNoti->takeItem(listNoti->row(listWItem));
            checkCount(); // <--- BÁO CHO PARENT BIẾT ĐÃ GIẢM SỐ LƯỢNG
        }
    });
    checkCount();
}