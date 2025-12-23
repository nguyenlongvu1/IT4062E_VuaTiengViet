#include "FriendRoomWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include "../../network/GameClient.h"

FriendRoomWidget::FriendRoomWidget(QString myUsername,bool isHost, QWidget *parent) 
    : QWidget(parent), m_isHost(isHost) , m_myUsername(myUsername)
{
    // Khởi tạo con trỏ bằng NULL để tránh lỗi rác bộ nhớ
    lblUser1 = nullptr;
    lblUser2 = nullptr; 
    frameSlot1 = nullptr;
    frameSlot2 = nullptr;

    setupUi();

}

void FriendRoomWidget::setupUi() {
    this->setObjectName("FriendRoomScreen"); 

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // --- HEADER: ROOM ID ---
    lblRoomID = new QLabel("Đang tạo phòng...", this);
    lblRoomID->setStyleSheet("font-size: 24px; font-weight: bold; color: #f1c40f;");
    lblRoomID->setAlignment(Qt::AlignCenter);

    // --- BODY: SLOTS AREA ---
    QHBoxLayout *slotsLayout = new QHBoxLayout();
    slotsLayout->setSpacing(30);
    slotsLayout->setAlignment(Qt::AlignCenter);

    // === SLOT 1: HOST (Khởi tạo trực tiếp biến thành viên) ===
    frameSlot1 = new QFrame(this);
    frameSlot1->setFixedSize(200, 300);
    frameSlot1->setStyleSheet("border: 2px solid #2ecc71; background-color: rgba(46, 204, 113, 0.1); border-radius: 10px;");
    
    QVBoxLayout *layout1 = new QVBoxLayout(frameSlot1);
    
    // QLabel *lblAvatar1 = new QLabel("H", frameSlot1); 
    lblAvatar1 = new QLabel("H", frameSlot1);
    lblAvatar1->setFixedSize(80, 80);
    lblAvatar1->setAlignment(Qt::AlignCenter);
    lblAvatar1->setStyleSheet("background-color: #3498db; color: white; border-radius: 40px; font-weight: bold; font-size: 24px;");

    lblUser1 = new QLabel("Đang tải...", frameSlot1); 
    lblUser1->setAlignment(Qt::AlignCenter);
    lblUser1->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");

    lblStatus1 = new QLabel("ĐÃ SẴN SÀNG", frameSlot1);
    lblStatus1->setAlignment(Qt::AlignCenter);
    lblStatus1->setStyleSheet("color: #2ecc71; font-style: italic;");

    layout1->addStretch();
    layout1->addWidget(lblAvatar1, 0, Qt::AlignCenter);
    layout1->addWidget(lblUser1);
    layout1->addWidget(lblStatus1);
    layout1->addStretch();

    // === SLOT 2: GUEST ===
    frameSlot2 = new QFrame(this);
    frameSlot2->setFixedSize(200, 300);
    frameSlot2->setStyleSheet("border: 1px solid #555; background-color: rgba(0,0,0,0.2); border-radius: 10px;"); 

    QVBoxLayout *layout2 = new QVBoxLayout(frameSlot2);

    // Avatar slot 2
    // QLabel *lblAvatar2 = new QLabel("?", frameSlot2);
    lblAvatar2 = new QLabel("?", frameSlot2);
    lblAvatar2->setFixedSize(80, 80);
    lblAvatar2->setAlignment(Qt::AlignCenter);
    lblAvatar2->setStyleSheet("background-color: #7f8c8d; color: white; border-radius: 40px; font-weight: bold; font-size: 24px;");

    lblUser2 = new QLabel("Trống", frameSlot2); 
    lblUser2->setAlignment(Qt::AlignCenter);
    lblUser2->setStyleSheet("font-size: 16px; color: #bdc3c7;");

    lblStatus2 = new QLabel("Đang chờ...", frameSlot2);
    lblStatus2->setAlignment(Qt::AlignCenter);
    lblStatus2->setStyleSheet("color: #7f8c8d; font-style: italic;");

    layout2->addStretch();
    layout2->addWidget(lblAvatar2, 0, Qt::AlignCenter);
    layout2->addWidget(lblUser2);
    layout2->addWidget(lblStatus2);
    layout2->addStretch();

    //SLot 3
    frameSlot3 = new QFrame(this);
    frameSlot3->setFixedSize(200, 300);
    frameSlot3->setStyleSheet("border: 2px solid #2ecc71; background-color: rgba(46, 204, 113, 0.1); border-radius: 10px;");
    
    QVBoxLayout *layout3 = new QVBoxLayout(frameSlot3);
    
    // QLabel *lblAvatar1 = new QLabel("H", frameSlot1);
    lblAvatar3 = new QLabel("H", frameSlot3);
    lblAvatar3->setFixedSize(80, 80);
    lblAvatar3->setAlignment(Qt::AlignCenter);
    lblAvatar3->setStyleSheet("background-color: #3498db; color: white; border-radius: 40px; font-weight: bold; font-size: 24px;");

    lblUser3 = new QLabel("Đang tải...", frameSlot3); 
    lblUser3->setAlignment(Qt::AlignCenter);
    lblUser3->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
    lblStatus3 = new QLabel("ĐÃ SẴN SÀNG", frameSlot3);
    lblStatus3->setAlignment(Qt::AlignCenter);
    lblStatus3->setStyleSheet("color: #2ecc71; font-style: italic;");

    layout3->addStretch();
    layout3->addWidget(lblAvatar3, 0, Qt::AlignCenter);
    layout3->addWidget(lblUser3);
    layout3->addWidget(lblStatus3);
    layout3->addStretch();

    // Thêm 2 slot vào layout ngang
    slotsLayout->addWidget(frameSlot1);
    slotsLayout->addWidget(frameSlot2);
    slotsLayout->addWidget(frameSlot3);

    // --- FOOTER: BUTTONS ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    // Nút tham gia (Cho khách)
    QPushButton *btnJoinRoom = new QPushButton("Nhập ID Phòng", this);
    btnJoinRoom->setFixedSize(150, 50);
    
    // Nút Bắt đầu (Cho chủ phòng)
    btnAction = new QPushButton(m_isHost ? "BẮT ĐẦU" : "SẴN SÀNG", this);
    btnAction->setFixedSize(200, 60);
    btnAction->setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; border-radius: 5px;");

    // Nút Rời phòng
    QPushButton *btnLeave = new QPushButton("Rời Phòng", this);
    btnLeave->setFixedSize(150, 50);
    btnLeave->setStyleSheet("background-color: #e74c3c; color: white; border-radius: 5px;");

    btnLayout->addWidget(btnJoinRoom);
    btnLayout->addStretch();
    btnLayout->addWidget(btnAction);
    btnLayout->addStretch();
    btnLayout->addWidget(btnLeave);

    // Lắp ráp Layout chính
    mainLayout->addWidget(lblRoomID);
    mainLayout->addStretch();
    mainLayout->addLayout(slotsLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // --- CONNECT SIGNALS ---
    connect(btnLeave, &QPushButton::clicked, this, &FriendRoomWidget::onLeaveBtnClicked);
    // connect(btnAction, &QPushButton::clicked, this, &FriendRoomWidget::startGame);
    connect(btnJoinRoom, &QPushButton::clicked, this, &FriendRoomWidget::onJoinRoomClicked);
    connect(btnAction, &QPushButton::clicked, this, [=](){
        QString text = lblRoomID->text(); 
        int roomId = text.split(":").last().trimmed().toInt();
        if (roomId > 0) {
            GameClient::instance().sendStartGame(roomId);
        } else {
            QMessageBox::warning(this, "Lỗi", "Không tìm thấy ID phòng hợp lệ!");
        }
    });
}

// Hàm cập nhật ID phòng lên Header
void FriendRoomWidget::setRoomID(const QString& id) {
    lblRoomID->setText(QString("Phòng ID: %1").arg(id));
}

// Hàm cập nhật thông tin chủ phòng (lúc mới tạo)
void FriendRoomWidget::setHostInfo(const QString& username) {
    updateMembers(username, "", "");
}

// Xử lý nút tham gia
void FriendRoomWidget::onJoinRoomClicked() {
    bool ok;
    QString roomIdText = QInputDialog::getText(this, "Tham gia", "Nhập mã phòng:", QLineEdit::Normal, "", &ok);
    if (ok && !roomIdText.isEmpty()) {
        GameClient::instance().sendJoinRoom(roomIdText.toInt());
    }
}

// Xử lý nút rời phòng
void FriendRoomWidget::onLeaveBtnClicked() {
    qDebug() << "[UI] Nut Roi Phong da duoc bam!"; 
    
    // Gửi lệnh lên Server
    GameClient::instance().sendLeaveRoom();
    emit leftRoom(); 
}


void FriendRoomWidget::updateMembers(const QString& p1, const QString& p2, const QString& p3) {
    qDebug() << "[UI] Update Room: P1=" << p1 << " | P2=" << p2 << " | P3=" << p3;

    if (lblUser1) {
        if (!p1.isEmpty()) {
            // Cập nhật thông tin Host
            lblUser1->setText(p1);
            if (lblStatus1) lblStatus1->setText("ĐÃ SẴN SÀNG");
            
            // Avatar Host: Chữ cái đầu, nền Xanh Dương hoặc Xanh Lá
            if (lblAvatar1) {
                lblAvatar1->setText(p1.left(1).toUpper());
                lblAvatar1->setStyleSheet(
                    "background-color: #2ecc71; " // Màu xanh lá (Host)
                    "color: white; "
                    "border-radius: 40px; "
                    "font-weight: bold; "
                    "font-size: 24px;"
                );
            }

            // --- LOGIC QUYỀN CHỦ PHÒNG 
            // Nếu người 1 rời phòng, Server đẩy người 2 lên làm p1.
            // Lso sánh lại: p1 mới có phải là mình không?
            if (p1 == m_myUsername) {
                // Mình là Host mới -> Hiện nút Bắt đầu
                btnAction->setVisible(true);
                btnAction->setText("BẮT ĐẦU");
                btnAction->setStyleSheet("background-color: #e67e22; color: white; font-weight: bold; border-radius: 5px;");
            } else {
                // Mình là khách -> Ẩn nút
                btnAction->setVisible(false);
            }

        } else {
            // Trường hợp phòng chưa có ai (Lỗi Server hoặc vừa khởi tạo)
            lblUser1->setText("Đang tải...");
            if (lblAvatar1) lblAvatar1->setText("...");
            btnAction->setVisible(false); 
        }
    }

    updateGuestSlot(frameSlot2, lblUser2, lblAvatar2, lblStatus2, p2);
    updateGuestSlot(frameSlot3, lblUser3, lblAvatar3, lblStatus3, p3);
}


void FriendRoomWidget::updateGuestSlot(QFrame* frame, QLabel* lblName, QLabel* lblAvatar, QLabel* lblStatus, const QString& playerName) {
    if (!frame || !lblName || !lblAvatar || !lblStatus) return;

    if (!playerName.isEmpty()) {
        // --- TRƯỜNG HỢP CÓ NGƯỜI 
        lblName->setText(playerName);
        lblStatus->setText("ĐÃ VÀO");
        lblAvatar->setText(playerName.left(1).toUpper());
        lblAvatar->setStyleSheet("background-color: #e67e22; color: white; border-radius: 40px; font-weight: bold; font-size: 24px;");
        frame->setStyleSheet("border: 2px solid #2ecc71; background-color: rgba(46, 204, 113, 0.1); border-radius: 10px;");
    } 
    else {         
        // Reset tên thành "Trống"
        lblName->setText("Trống");
        lblStatus->setText("Đang chờ...");
        lblAvatar->setText("?");
        lblAvatar->setStyleSheet(
            "background-color: #7f8c8d; " 
            "color: white; "
            "border-radius: 40px; "
            "font-weight: bold; "
            "font-size: 24px;"
        );

        frame->setStyleSheet("border: 1px solid #555; background-color: rgba(0,0,0,0.2); border-radius: 10px;");
    }
}