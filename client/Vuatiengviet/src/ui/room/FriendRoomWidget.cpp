#include "FriendRoomWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
// #include <GameButton>
#include <QFrame>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsDropShadowEffect> 
#include "../../network/GameClient.h"
#include "../../utils/GameButton.h"
#include <QTimer>

#include <QStyle>

FriendRoomWidget::FriendRoomWidget(QString myUsername, bool isHost, QWidget *parent) 
    : QWidget(parent), m_isHost(isHost), m_myUsername(myUsername)
{
    // Initialize pointers to nullptr for safety

    this->setAttribute(Qt::WA_DeleteOnClose);
    lblUser1 = nullptr; lblUser2 = nullptr; lblUser3 = nullptr;
    lblAvatar1 = nullptr; lblAvatar2 = nullptr; lblAvatar3 = nullptr;
    lblStatus1 = nullptr; lblStatus2 = nullptr; lblStatus3 = nullptr;
    
    // Store slot widget pointers if you declared them in .h (m_slotWidget1, etc.)
    // If not declared in .h, we just use local variables or findChild
    
    // Important for custom painting/styling on QWidget
    this->setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    connect(&GameClient::instance(), &GameClient::roomInfoReceived, 
            this, &FriendRoomWidget::updateMembers);
    connect(&GameClient::instance(), &GameClient::roomJoined, this, [=](QString roomId){
        qDebug() << "[UI] Joined Room ID:" << roomId;
        setRoomID(roomId);
    });
    // 2. Thêm dòng này để gọi lấy thông tin ngay khi Widget hiện lên
    if (GameClient::instance().isConnected()) {
        GameClient::instance().sendGetRoomInfo();
    }
}

// Helper function to create a styled slot container
QWidget* FriendRoomWidget::createStyledSlot(int slotIndex) {
    // Fixed dimensions for each slot
    int w = 220; 
    int h = 320;

    // 1. Main Container (Border Widget)
    QWidget *borderWidget = new QWidget(this);
    borderWidget->setFixedSize(w, h);
    
    // Outer Glow Gradient
    borderWidget->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1," 
        "   stop:0    rgba(255, 255, 255, 0),"   
        "   stop:0.5  rgba(255, 255, 255, 0.4)," // Bright border
        "   stop:1    rgba(255, 255, 255, 0));"
        "border-radius: 15px;" 
    );

    // Glow Effect
    QGraphicsDropShadowEffect *panelShadow = new QGraphicsDropShadowEffect(borderWidget);
    panelShadow->setBlurRadius(20);
    panelShadow->setColor(QColor(255, 255, 255, 100)); // Light white glow
    panelShadow->setOffset(0, 0);
    borderWidget->setGraphicsEffect(panelShadow);

    // Layout for the border (Margin 2px to show the border gradient)
    QVBoxLayout *borderLayout = new QVBoxLayout(borderWidget);
    borderLayout->setContentsMargins(2, 2, 2, 2); 

    // 2. Glass Panel (Background)
    QWidget *glassPanel = new QWidget(borderWidget);
    // Assign object name to easily find it later for content updates
    glassPanel->setObjectName(QString("glassPanel_%1").arg(slotIndex)); 
    glassPanel->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "   stop:0 rgba(50, 50, 80, 0.95),"   
        "   stop:1 rgba(20, 20, 35, 0.98));"  
        "border-radius: 13px;" // 15px (outer) - 2px (margin)
    );
    borderLayout->addWidget(glassPanel);

    // 3. Neon Vertical Lines
    int lineH = h - 40; 
    int lineY = 20;

    // --- Left Line ---
    QWidget *leftLine = new QWidget(glassPanel);
    leftLine->setStyleSheet(
       "background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0    rgba(255, 255, 255, 0),"
        "    stop:0.5  rgba(255, 255, 255, 1.0),"
        "    stop:1    rgba(255, 255, 255, 0));"
    );
    QGraphicsDropShadowEffect *glowL = new QGraphicsDropShadowEffect(leftLine);
    glowL->setBlurRadius(15); 
    glowL->setColor(QColor(0, 255, 255, 200)); 
    glowL->setOffset(0,0);
    leftLine->setGraphicsEffect(glowL);
    leftLine->setGeometry(0, lineY, 2, lineH); 

    // --- Right Line ---
    QWidget *rightLine = new QWidget(glassPanel);
    rightLine->setStyleSheet(leftLine->styleSheet());
    QGraphicsDropShadowEffect *glowR = new QGraphicsDropShadowEffect(rightLine);
    glowR->setBlurRadius(15); 
    glowR->setColor(QColor(0, 255, 255, 200)); 
    glowR->setOffset(0,0);
    rightLine->setGraphicsEffect(glowR);
    rightLine->setGeometry(w - 2 - 4, lineY, 2, lineH); // -2 width, -margin

    // 4. Content Layout (Where Avatar/Text will go)
    QVBoxLayout *contentLayout = new QVBoxLayout(glassPanel);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setSpacing(10);

    return borderWidget; 
}

void FriendRoomWidget::setupUi() {
    this->setObjectName("FriendRoomScreen"); 
    
    // Background Image
    this->setStyleSheet(
        "#FriendRoomScreen {"
        " border-image: url(:/bgHome.png) 0 0 0 0 stretch stretch; "
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // --- HEADER: ROOM ID ---
    lblRoomID = new QLabel("Đang tạo phòng...", this);
    lblRoomID->setStyleSheet("font-size: 30px; font-weight: bold; color: #f1c40f; background: transparent;");
    lblRoomID->setAlignment(Qt::AlignCenter);

    // --- BODY: SLOTS AREA ---
    QHBoxLayout *slotsLayout = new QHBoxLayout();
    slotsLayout->setSpacing(40); // Increased spacing for better look
    slotsLayout->setAlignment(Qt::AlignCenter);

    // === SLOT 1: HOST ===
    // Create the styled container
    QWidget* slot1Widget = createStyledSlot(1);
    // Find the internal glass panel to add content to its layout
    QWidget* glass1 = slot1Widget->findChild<QWidget*>("glassPanel_1");
    QVBoxLayout* layout1 = qobject_cast<QVBoxLayout*>(glass1->layout());

    // Content for Slot 1
    lblAvatar1 = new QLabel("H", glass1);
    lblAvatar1->setFixedSize(90, 90);
    lblAvatar1->setAlignment(Qt::AlignCenter);
    lblAvatar1->setStyleSheet("background-color: #3498db; color: white; border-radius: 45px; font-weight: bold; font-size: 28px; border: 2px solid rgba(255,255,255,0.3);");

    lblUser1 = new QLabel("Đang tải...", glass1); 
    lblUser1->setAlignment(Qt::AlignCenter);
    lblUser1->setStyleSheet("font-size: 18px; font-weight: bold; color: white; background: transparent;");

    lblStatus1 = new QLabel("ĐÃ SẴN SÀNG", glass1);
    lblStatus1->setAlignment(Qt::AlignCenter);
    lblStatus1->setStyleSheet("color: #2ecc71; font-weight: bold; font-style: italic; background: transparent;");

    layout1->addStretch();
    layout1->addWidget(lblAvatar1, 0, Qt::AlignCenter);
    layout1->addWidget(lblUser1);
    layout1->addWidget(lblStatus1);
    layout1->addStretch();

    // === SLOT 2: GUEST 1 ===
    QWidget* slot2Widget = createStyledSlot(2);
    QWidget* glass2 = slot2Widget->findChild<QWidget*>("glassPanel_2");
    QVBoxLayout* layout2 = qobject_cast<QVBoxLayout*>(glass2->layout());

    lblAvatar2 = new QLabel("?", glass2);
    lblAvatar2->setFixedSize(90, 90);
    lblAvatar2->setAlignment(Qt::AlignCenter);
    lblAvatar2->setStyleSheet("background-color: rgba(255,255,255,0.1); color: gray; border-radius: 45px; font-weight: bold; font-size: 28px; border: 2px dashed rgba(255,255,255,0.2);");

    lblUser2 = new QLabel("Trống", glass2); 
    lblUser2->setAlignment(Qt::AlignCenter);
    lblUser2->setStyleSheet("font-size: 16px; color: #bdc3c7; background: transparent;");

    lblStatus2 = new QLabel("Đang chờ...", glass2);
    lblStatus2->setAlignment(Qt::AlignCenter);
    lblStatus2->setStyleSheet("color: gray; font-style: italic; background: transparent;");

    layout2->addStretch();
    layout2->addWidget(lblAvatar2, 0, Qt::AlignCenter);
    layout2->addWidget(lblUser2);
    layout2->addWidget(lblStatus2);
    layout2->addStretch();

    // === SLOT 3: GUEST 2 ===
    QWidget* slot3Widget = createStyledSlot(3);
    QWidget* glass3 = slot3Widget->findChild<QWidget*>("glassPanel_3");
    QVBoxLayout* layout3 = qobject_cast<QVBoxLayout*>(glass3->layout());

    lblAvatar3 = new QLabel("?", glass3);
    lblAvatar3->setFixedSize(90, 90);
    lblAvatar3->setAlignment(Qt::AlignCenter);
    lblAvatar3->setStyleSheet(lblAvatar2->styleSheet()); // Reuse empty style

    lblUser3 = new QLabel("Trống", glass3); 
    lblUser3->setAlignment(Qt::AlignCenter);
    lblUser3->setStyleSheet(lblUser2->styleSheet());

    lblStatus3 = new QLabel("Đang chờ...", glass3);
    lblStatus3->setAlignment(Qt::AlignCenter);
    lblStatus3->setStyleSheet(lblStatus2->styleSheet());

    layout3->addStretch();
    layout3->addWidget(lblAvatar3, 0, Qt::AlignCenter);
    layout3->addWidget(lblUser3);
    layout3->addWidget(lblStatus3);
    layout3->addStretch();

    // Add slots to main horizontal layout
    slotsLayout->addWidget(slot1Widget);
    slotsLayout->addWidget(slot2Widget);
    slotsLayout->addWidget(slot3Widget);

    // --- FOOTER: BUTTONS ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    GameButton*btnJoinRoom = new GameButton("Nhập ID Phòng", this);
    btnJoinRoom->setObjectName("btnJoinRoom"); // Đặt ID để CSS ăn theo
    btnJoinRoom->setFixedSize(180, 60);
    btnJoinRoom->setCursor(Qt::PointingHandCursor);

    // Hiệu ứng Glow Xanh Dương
    QGraphicsDropShadowEffect *glowJoin = new QGraphicsDropShadowEffect(btnJoinRoom);
    glowJoin->setBlurRadius(30); 
    glowJoin->setColor(QColor(52, 152, 219, 180)); // Xanh dương, trong suốt
    glowJoin->setOffset(0, 0);
    btnJoinRoom->setGraphicsEffect(glowJoin);

    // Style Gradient Xanh
    btnJoinRoom->setStyleSheet(
        "#btnJoinRoom {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "   border: 2px solid #5dade2;"
        "   border-radius: 15px;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 18px;"
        "   font-family: 'Nunito', sans-serif;"
        "   outline: none;" // Quan trọng: Bỏ viền tím khi focus
        "}"
        "#btnJoinRoom:hover {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #3498db);"
        "   margin-top: 2px;"
        "}"
        "#btnJoinRoom:pressed {"
        "   background-color: #1a5276;" // Xanh đậm khi nhấn
        "   margin-top: 4px;"
        "}"
    );
    
    btnAction = new GameButton(m_isHost ? "BẮT ĐẦU" : "SẴN SÀNG", this);
    btnAction->setFixedSize(220, 70); // Kích thước vừa vặn cho footer
    btnAction->setCursor(Qt::PointingHandCursor);

    // 1. Hiệu ứng Hào quang (Glow) màu cam
    QGraphicsDropShadowEffect *glowAction = new QGraphicsDropShadowEffect(btnAction);
    glowAction->setBlurRadius(20); // Độ tỏa
    glowAction->setColor(QColor(255, 140, 0, 200)); // Màu cam đậm, độ trong suốt 200
    glowAction->setOffset(0, 0);
    btnAction->setGraphicsEffect(glowAction);

    // 2. Style Gradient Cam (Copy từ btnRank)
    btnAction->setStyleSheet(
        "GameButton{"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ac43, stop:1 #f5613d);"
        "   border: 2px solid #fbad6c;"
        "   border-radius: 15px;"
        "   color: white;"
        "   font-weight: 900;"       // Chữ đậm
        "   font-size: 24px;"        // Cỡ chữ to rõ
        "   font-family: 'Nunito', sans-serif;"
        "}"
        "GameButton:hover {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ac43, stop:1 #b04d0e);"
        "   margin-top: 2px;"        // Hiệu ứng nhấn nhẹ khi di chuột
        "}"
        "GameButton:pressed {"
        "   background-color: #6e3108;"
        "   margin-top: 4px;"        // Hiệu ứng lún xuống khi bấm
        "}"
        "GameButton:disabled {"     // Style khi nút bị vô hiệu hóa (nếu cần)
        "   background-color: #95a5a6;"
        "   border: 2px solid #7f8c8d;"
        "   color: #bdc3c7;"
        "}"
    );

    GameButton*btnLeave = new GameButton("Rời Phòng", this);
    btnLeave->setObjectName("btnLeave");
    btnLeave->setFixedSize(180, 60);
    btnLeave->setCursor(Qt::PointingHandCursor);

    // Hiệu ứng Glow Đỏ
    QGraphicsDropShadowEffect *glowLeave = new QGraphicsDropShadowEffect(btnLeave);
    glowLeave->setBlurRadius(30); 
    glowLeave->setColor(QColor(231, 76, 60, 180)); // Đỏ, trong suốt
    glowLeave->setOffset(0, 0);
    btnLeave->setGraphicsEffect(glowLeave);

    // Style Gradient Đỏ
    btnLeave->setStyleSheet(
        "#btnLeave {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b);"
        "   border: 2px solid #ec7063;"
        "   border-radius: 15px;"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 18px;"
        "   font-family: 'Nunito', sans-serif;"
        "   outline: none;" // Quan trọng
        "}"
        "#btnLeave:hover {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ec7063, stop:1 #e74c3c);"
        "   margin-top: 2px;"
        "}"
        "#btnLeave:pressed {"
        "   background-color: #922b21;" // Đỏ đậm khi nhấn
        "   margin-top: 4px;"
        "}"
    );

    btnLayout->addWidget(btnJoinRoom);
    btnLayout->addStretch();
    btnLayout->addWidget(btnAction);
    btnLayout->addStretch();
    btnLayout->addWidget(btnLeave);

    // Assemble Main Layout
    mainLayout->addWidget(lblRoomID);
    mainLayout->addStretch();
    mainLayout->addLayout(slotsLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // --- CONNECT SIGNALS ---
    connect(btnLeave, &GameButton::clicked, this, &FriendRoomWidget::onLeaveBtnClicked);
    connect(btnJoinRoom, &GameButton::clicked, this, &FriendRoomWidget::onJoinRoomClicked);
    connect(btnAction, &GameButton::clicked, this, [=](){
        if (m_isHost) {
        emit startGame();   // 🔥 thông báo cho MainWindow
    } 
        QString text = lblRoomID->text(); 
        // Logic to extract ID: "Phòng ID: 12345" -> "12345"
        int roomId = text.split(":").last().trimmed().toInt();
        if (roomId > 0) {
            GameClient::instance().sendStartGame(roomId);
        } else {
            QMessageBox::warning(this, "Lỗi", "Không tìm thấy ID phòng hợp lệ!");
        }
    });
}

// Update Room ID Header
void FriendRoomWidget::setRoomID(const QString& id) {
    lblRoomID->setText(QString("Phòng ID: %1").arg(id));
}

// Update Host info initially
void FriendRoomWidget::setHostInfo(const QString& username) {
    updateMembers(username, "", "");
}

// Join Room Button Handler
void FriendRoomWidget::onJoinRoomClicked() {
    bool ok;
    QString roomIdText = QInputDialog::getText(this, "Tham gia", "Nhập mã phòng:", QLineEdit::Normal, "", &ok);
    if (ok && !roomIdText.isEmpty()) {
        GameClient::instance().sendJoinRoom(roomIdText.toInt());
    }
}

// Leave Room Button Handler
void FriendRoomWidget::onLeaveBtnClicked() {
    qDebug() << "[UI] Nut Roi Phong da duoc bam!"; 
    GameClient::instance().sendLeaveRoom();
    emit leftRoom(); 
}

// Update all slots based on player data
void FriendRoomWidget::updateMembers(const QString& p1, const QString& p2, const QString& p3) {
    qDebug() << "[UI] Update Room: P1=" << p1 << " | P2=" << p2 << " | P3=" << p3;

    // 1. Đếm số người chơi
    int playerCount = 0;
    if (!p1.trimmed().isEmpty()) playerCount++;
    if (!p2.trimmed().isEmpty()) playerCount++;
    if (!p3.trimmed().isEmpty()) playerCount++;

    // --- UPDATE SLOT 1 (HOST) ---
    if (lblUser1) {
        QString hostName = p1.trimmed();
        lblAvatar1->setGraphicsEffect(nullptr); // Xóa hiệu ứng cũ

        if (!hostName.isEmpty()) {
            lblUser1->setText(hostName);
            lblUser1->setStyleSheet("font-size: 18px; font-weight: bold; color: white; background: transparent;");
            
            if (lblStatus1) {
                lblStatus1->setText("ĐÃ SẴN SÀNG");
                lblStatus1->setStyleSheet("color: #2ecc71; font-weight: bold; font-style: italic; background: transparent;");
            }
            
            if (lblAvatar1) {
                lblAvatar1->setText(hostName.left(1).toUpper());
                lblAvatar1->setStyleSheet("background-color: #2ecc71; color: white; border-radius: 45px; font-weight: bold; font-size: 28px; border: 2px solid white;");
                QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect(this);
                glow->setBlurRadius(20); glow->setColor(QColor("#2ecc71")); glow->setOffset(0,0);
                lblAvatar1->setGraphicsEffect(glow);
            }

            // -------------------------------------------------
            // LOGIC QUYẾT ĐỊNH NÚT START (BẮT ĐẦU)
            // -------------------------------------------------
            if (hostName == m_myUsername) {
                btnAction->setVisible(true);

                if (playerCount == 3) {
                    // --- ĐỦ 3 NGƯỜI: CHO PHÉP BẮT ĐẦU ---
                    btnAction->setEnabled(true);
                    btnAction->setText("BẮT ĐẦU");
                    
                    // Style Cam (Active)
                    btnAction->setStyleSheet(
                        "GameButton{"
                        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ac43, stop:1 #f5613d);"
                        "   border: 2px solid #fbad6c; border-radius: 15px; color: white;"
                        "   font-weight: 900; font-size: 24px; font-family: 'Nunito', sans-serif;"
                        "}"
                        "GameButton:hover { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f6ac43, stop:1 #b04d0e); margin-top: 2px; }"
                        "GameButton:pressed { background-color: #6e3108; margin-top: 4px; }"
                    );
                } else {
                    // --- THIẾU NGƯỜI: CHẶN BẮT ĐẦU ---
                    btnAction->setEnabled(false);
                    btnAction->setText(QString("CHỜ (%1/3)").arg(playerCount));
                    
                    // Style Xám (Disabled)
                    btnAction->setStyleSheet(
                        "GameButton{"
                        "   background-color: #7f8c8d;" // Xám
                        "   border: 2px solid #95a5a6; border-radius: 15px; color: #bdc3c7;"
                        "   font-weight: 900; font-size: 24px; font-family: 'Nunito', sans-serif;"
                        "}"
                    );
                }

                // Force Repaint Button
                btnAction->style()->unpolish(btnAction);
                btnAction->style()->polish(btnAction);
                btnAction->update();

            } else {
                btnAction->setVisible(false);
            }
            // -------------------------------------------------

        } else {
            lblUser1->setText("Đang tải...");
            if (lblAvatar1) lblAvatar1->setText("...");
            btnAction->setVisible(false); 
        }

        // Force Repaint Slot 1
        lblUser1->style()->unpolish(lblUser1); lblUser1->style()->polish(lblUser1); lblUser1->update();
        lblAvatar1->style()->unpolish(lblAvatar1); lblAvatar1->style()->polish(lblAvatar1); lblAvatar1->update();
    }

    // --- UPDATE SLOT 2 & 3 ---
    updateGuestSlot(nullptr, lblUser2, lblAvatar2, lblStatus2, p2);
    updateGuestSlot(nullptr, lblUser3, lblAvatar3, lblStatus3, p3);
}
void FriendRoomWidget::updateGuestSlot(QFrame* frame, QLabel* lblName, QLabel* lblAvatar, QLabel* lblStatus, const QString& playerName) {
    Q_UNUSED(frame); 
    
    QString cleanName = playerName.trimmed();
    
    // 1. Xóa hiệu ứng cũ (Rất quan trọng trên Linux/VM)
    lblAvatar->setGraphicsEffect(nullptr);

    if (!cleanName.isEmpty()) {
        qDebug() << "[UI DEBUG] Guest Slot -> OCCUPIED:" << cleanName;
        
        lblName->setVisible(true);
        lblAvatar->setVisible(true);
        lblStatus->setVisible(true);

        lblName->setText(cleanName);
        lblName->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: white; background: transparent; }");
        
        lblStatus->setText("ĐÃ VÀO");
        lblStatus->setStyleSheet("QLabel { color: #2ecc71; font-weight: bold; background: transparent; }");
        
        lblAvatar->setText(cleanName.left(1).toUpper());
        lblAvatar->setStyleSheet(
            "QLabel { background-color: #e67e22; color: white; border-radius: 45px; font-weight: bold; font-size: 28px; border: 2px solid white; }"
        );
        
        // Tạo lại hiệu ứng Glow
        QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect(this);
        glow->setBlurRadius(20); glow->setColor(QColor("#e67e22")); glow->setOffset(0,0);
        lblAvatar->setGraphicsEffect(glow);

    } else { 
        // --- EMPTY ---
        lblName->setText("Trống");
        lblName->setStyleSheet("QLabel { font-size: 16px; color: #bdc3c7; background: transparent; }");
        
        lblStatus->setText("Đang chờ...");
        lblStatus->setStyleSheet("QLabel { color: gray; font-style: italic; background: transparent; }");
        
        lblAvatar->setText("?");
        lblAvatar->setStyleSheet(
            "QLabel { background-color: rgba(255,255,255,0.05); color: gray; border-radius: 45px; font-weight: bold; font-size: 28px; border: 2px dashed #555; }"
        );
    }

    // [LINUX FIX] Force Repaint cực mạnh
    // unpolish/polish ép Qt tính toán lại Style
    lblName->style()->unpolish(lblName);
    lblName->style()->polish(lblName);
    lblName->repaint();

    lblAvatar->style()->unpolish(lblAvatar);
    lblAvatar->style()->polish(lblAvatar);
    lblAvatar->repaint();

    lblStatus->repaint();
}