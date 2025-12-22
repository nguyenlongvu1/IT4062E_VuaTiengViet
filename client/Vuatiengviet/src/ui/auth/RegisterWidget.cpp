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

RegisterWidget::RegisterWidget(QWidget *parent) : QWidget(parent) {

    this->setObjectName("LoginScreen");

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Tràn viền
    mainLayout->setSpacing(0); 

    QWidget *rightOverlay = new QWidget(this);
    rightOverlay->setObjectName("RightOverlay");

    QVBoxLayout *overlayLayout = new QVBoxLayout(rightOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    loginContainer = new QFrame(this);
    loginContainer->setObjectName("LoginContainer");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 80)); // Màu đen mờ
    shadow->setOffset(0, 8);
    loginContainer->setGraphicsEffect(shadow);

    QVBoxLayout *contentLayout = new QVBoxLayout(loginContainer);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 40, 30, 40);

    QLabel *lblTitle = new QLabel("ĐĂNG KÝ TÀI KHOẢN", this);
    lblTitle->setObjectName("lblTitle");
    lblTitle->setAlignment(Qt::AlignCenter);
    
    txtUser = new QLineEdit(this); 
    txtUser->setPlaceholderText("Tên đăng nhập");
    
    txtPass = new QLineEdit(this); 
    txtPass->setPlaceholderText("Mật khẩu");
    txtPass->setEchoMode(QLineEdit::Password);
    
    txtConfirm = new QLineEdit(this); 
    txtConfirm->setPlaceholderText("Nhập lại mật khẩu");
    txtConfirm->setEchoMode(QLineEdit::Password);
    
    QPushButton *btnSubmit = new QPushButton("ĐĂNG KÝ", this);
    btnSubmit->setObjectName("btnLogin"); // Dùng chung style nút xanh với Login
    btnSubmit->setCursor(Qt::PointingHandCursor);

    QPushButton *btnBack = new QPushButton("Quay lại Đăng nhập", this);
    btnBack->setObjectName("btnLink");
    btnBack->setFlat(true);
    btnBack->setCursor(Qt::PointingHandCursor);

    // --- ADD TO LAYOUT ---
    contentLayout->addWidget(lblTitle);
    contentLayout->addSpacing(10); // Khoảng cách nhỏ dưới tiêu đề
    contentLayout->addWidget(txtUser);
    contentLayout->addWidget(txtPass);
    contentLayout->addWidget(txtConfirm);
    contentLayout->addSpacing(10); // Khoảng cách trước nút bấm
    contentLayout->addWidget(btnSubmit);
    contentLayout->addWidget(btnBack);
    contentLayout->setAlignment(Qt::AlignCenter);

    overlayLayout->addWidget(loginContainer);
    mainLayout->setColumnStretch(0, 6); // Bên trái chiếm 4 phần (Rộng hơn)
    mainLayout->setColumnStretch(1, 5);

     mainLayout->addWidget(rightOverlay, 0, 1);

    // -- Logic --
    connect(btnBack, &QPushButton::clicked, this, &RegisterWidget::backToLogin);
    
    connect(btnSubmit, &QPushButton::clicked, [=](){
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
