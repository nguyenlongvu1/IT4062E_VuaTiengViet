#include "NotificationDialog.h"
#include "../../network/GameClient.h" // [QUAN TRỌNG] Include để gọi lệnh gửi đi
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

NotificationDialog::NotificationDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Hộp Thư Thông Báo");
    setFixedSize(400, 500);
    setStyleSheet("QDialog { background-color: #2c3e50; color: white; }"
                  "QListWidget { background: transparent; border: none; outline: none; }");
    setupUi();
    // loadFakeData(); // XÓA: Không load giả nữa
}

void NotificationDialog::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *lblTitle = new QLabel("🔔 Thông báo mới", this);
    lblTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #f1c40f; margin-bottom: 10px;");
    lblTitle->setAlignment(Qt::AlignCenter);

    listNoti = new QListWidget(this);

    QPushButton *btnClose = new QPushButton("Đóng", this);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 8px; border-radius: 5px; font-weight: bold; } QPushButton:hover { background-color: #7f8c8d; }");
    
    layout->addWidget(lblTitle);
    layout->addWidget(listNoti);
    layout->addWidget(btnClose);

    connect(btnClose, &QPushButton::clicked, this, &NotificationDialog::hide); // Sửa close -> hide để không bị hủy
}

// [LOGIC MỚI] Hàm thêm lời mời thật
void NotificationDialog::addFriendRequest(const QString &senderName) {
    // 1. KIỂM TRA TRÙNG LẶP (QUAN TRỌNG)
    // Duyệt qua danh sách hiện tại, nếu thấy tên này rồi thì thôi, không thêm nữa
    for(int i = 0; i < listNoti->count(); ++i) {
        QListWidgetItem* existingItem = listNoti->item(i);
        // Lấy widget con gắn vào item để check tên (hoặc lưu tên vào UserRole)
        // Cách đơn giản nhất: Lưu senderName vào UserRole lúc tạo item
        if (existingItem->data(Qt::UserRole).toString() == senderName) {
            return; // Đã có rồi -> Thoát luôn
        }
    }

    // 2. NẾU CHƯA CÓ THÌ TẠO MỚI (Code cũ của bạn)
    QListWidgetItem *listWItem = new QListWidgetItem(listNoti);
    listWItem->setSizeHint(QSize(listNoti->width() - 30, 90));
    
    // --- LƯU TÊN VÀO DATA ĐỂ CHECK TRÙNG LẦN SAU ---
    listWItem->setData(Qt::UserRole, senderName); 
    // -----------------------------------------------

    QWidget *wid = new QWidget();
    wid->setStyleSheet("background-color: rgba(0,0,0,0.3); border-radius: 8px; margin-bottom: 5px;");
    
    QVBoxLayout *vLayout = new QVBoxLayout(wid);
    
    QLabel *lblMsg = new QLabel(QString("<b>%1</b> muốn kết bạn với bạn.").arg(senderName));
    lblMsg->setStyleSheet("color: white; font-size: 13px;");
    lblMsg->setWordWrap(true);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAccept = new QPushButton("Đồng ý");
    btnAccept->setCursor(Qt::PointingHandCursor);
    btnAccept->setStyleSheet("background-color: #2ecc71; border: none; padding: 5px; border-radius: 4px; color: white; font-weight: bold;");
    
    QPushButton *btnDecline = new QPushButton("Từ chối");
    btnDecline->setCursor(Qt::PointingHandCursor);
    btnDecline->setStyleSheet("background-color: #e74c3c; border: none; padding: 5px; border-radius: 4px; color: white; font-weight: bold;");

    btnLayout->addWidget(btnAccept);
    btnLayout->addWidget(btnDecline);

    vLayout->addWidget(lblMsg);
    vLayout->addLayout(btnLayout);

    listNoti->setItemWidget(listWItem, wid);

    // --- LOGIC NÚT BẤM ---
    connect(btnAccept, &QPushButton::clicked, [=](){
        btnAccept->setText("Đang xử lý...");
        btnAccept->setEnabled(false);
        GameClient::instance().sendAcceptFriend(senderName);
        delete listNoti->takeItem(listNoti->row(listWItem));
    });

    connect(btnDecline, &QPushButton::clicked, [=](){
        delete listNoti->takeItem(listNoti->row(listWItem));
    });
}