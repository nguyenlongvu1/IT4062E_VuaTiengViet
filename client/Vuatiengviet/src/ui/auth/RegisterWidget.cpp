#include "RegisterWidget.h"
#include "GameClient.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include "../../utils/AudioManager.h"

RegisterWidget::RegisterWidget(QWidget *parent) : QWidget(parent) {

    this->setObjectName("LoginScreen");

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0); 

    QWidget *rightOverlay = new QWidget(this);
    rightOverlay->setObjectName("RightOverlay");

    QVBoxLayout *overlayLayout = new QVBoxLayout(rightOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    loginContainer = new QFrame(this);
    loginContainer->setObjectName("LoginContainer");

    QWidget *borderWidget = new QWidget(loginContainer);
    borderWidget->setObjectName("borderWidget");
    borderWidget->setFixedSize(603, 702); // Khớp hoàn toàn với kích thước widget
    
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
    panelShadow->setColor(QColor(255, 255, 255, 250)); 
    panelShadow->setOffset(0, 0);
    borderWidget->setGraphicsEffect(panelShadow);

    borderLayout->addWidget(glassPanel);

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
    leftLine->setGeometry(-1, 40, 2, 500); 

    QWidget *rightLine = new QWidget(glassPanel);
    rightLine->setStyleSheet(leftLine->styleSheet());
    QGraphicsDropShadowEffect *glowR = new QGraphicsDropShadowEffect(rightLine);
    glowR->setBlurRadius(80); glowR->setColor(QColor(200, 230, 255, 255)); glowR->setOffset(0,0);
    rightLine->setGraphicsEffect(glowR);
    
    // SetGeometry: x = 446 (width) - 3 (line width) = 443
    rightLine->setGeometry(597, 40, 2, 500);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 80)); 
    shadow->setOffset(0, 8);
    loginContainer->setGraphicsEffect(shadow);

    QVBoxLayout *contentLayout = new QVBoxLayout(loginContainer);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 40, 30, 40);

    QLabel *lblTitle = new QLabel("ĐĂNG KÝ TÀI KHOẢN", this);
    lblTitle->setObjectName("lblTitle");
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("QLabel#lblTitle { color: #f1c40f; font-size: 26px; font-weight: bold; background: transparent; }");
    
    txtUser = new QLineEdit(this); 
    txtUser->setPlaceholderText("Tên đăng nhập");
    
    txtPass = new QLineEdit(this); 
    txtPass->setPlaceholderText("Mật khẩu");
    txtPass->setEchoMode(QLineEdit::Password);
    
    txtConfirm = new QLineEdit(this); 
    txtConfirm->setPlaceholderText("Nhập lại mật khẩu");
    txtConfirm->setEchoMode(QLineEdit::Password);
    
    QPushButton *btnSubmit = new QPushButton("ĐĂNG KÝ", this);
    btnSubmit->setObjectName("btnLogin"); 
    btnSubmit->setCursor(Qt::PointingHandCursor);

    QPushButton *btnBack = new QPushButton("Quay lại Đăng nhập", this);
    btnBack->setObjectName("btnLink");
    btnBack->setFlat(true);
    btnBack->setCursor(Qt::PointingHandCursor);

    // --- ADD TO LAYOUT ---
    contentLayout->addWidget(lblTitle);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(txtUser);
    contentLayout->addWidget(txtPass);
    contentLayout->addWidget(txtConfirm);
    contentLayout->addSpacing(10); 
    contentLayout->addWidget(btnSubmit);
    contentLayout->addWidget(btnBack);
    contentLayout->setAlignment(Qt::AlignCenter);

    overlayLayout->addWidget(loginContainer);
    mainLayout->setColumnStretch(0, 6); 
    mainLayout->setColumnStretch(1, 5);

     mainLayout->addWidget(rightOverlay, 0, 1);

    // -- Logic --
    connect(btnBack, &QPushButton::clicked, this, &RegisterWidget::backToLogin);
    
    connect(btnSubmit, &QPushButton::clicked, [=](){
        AudioManager::instance().playClickSound();
        if(txtPass->text() != txtConfirm->text()) {
            QMessageBox::warning(this, "Lỗi", "Mật khẩu không khớp!");
            return;
        }
        if(txtUser->text().isEmpty()) {
             QMessageBox::warning(this, "Lỗi", "Chưa nhập tên đăng nhập!");
             return;
        }
        GameClient::instance().sendRegister(txtUser->text(), txtPass->text());
    });

    connect(&GameClient::instance(), &GameClient::registerSuccess, [=](){
        QMessageBox::information(this, "Thành công", "Đăng ký thành công! Hãy đăng nhập.");
        emit backToLogin();
       
    });
    
    // Xử lý khi Đăng ký THẤT BẠI (Dùng QMessageBox)
    connect(&GameClient::instance(), &GameClient::registerFailed, [=](QString msg, QString code){
        qDebug() << "[UI DEBUG] RegisterWidget received registerFailed signal!";
        QString errorTitle = "Đăng ký thất bại";
        QString errorContent = msg; // Mặc định là lỗi gốc từ server

         if (code == "InvalidUsername" ) {
            errorContent = "Tên đăng ký không hợp lệ!\n(Phải từ 4-20 ký tự, không dấu cách, không ký tự đặc biệt)";
            txtUser->setFocus();
            txtUser->selectAll();
        }
        // 2. Lỗi định dạng Mật khẩu (Quá ngắn...)
        else if (code == "InvalidPassword") {
            errorContent = "Mật khẩu không hợp lệ!\n(Tối thiểu 6 ký tự)";
            txtPass->clear();
            txtPass->setFocus();
        }
        // 3. Lỗi trùng tên (User Existed)
        else if (code == "USERNAME_TAKEN") {
            errorContent = "Tên đăng nhập này đã được sử dụng.\nVui lòng chọn tên khác!";
            txtUser->selectAll();
            txtUser->setFocus();
        }

        // --- HIỆN HỘP THOẠI ---
        // Sử dụng QMessageBox::critical để hiện icon lỗi (X đỏ)
        QMessageBox::critical(this, errorTitle, errorContent);
    });
}
