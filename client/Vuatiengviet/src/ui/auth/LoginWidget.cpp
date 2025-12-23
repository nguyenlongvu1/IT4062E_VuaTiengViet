#include "LoginWidget.h"
#include "GameClient.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent) {

    this->setObjectName("LoginScreen");

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Tràn viền
    mainLayout->setSpacing(0); // Không có khe hở giữa 2 cột

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

    QLabel *lblTitle = new QLabel("VUA TIẾNG VIỆT", this);
    lblTitle->setObjectName("lblTitle");
    lblTitle->setAlignment(Qt::AlignCenter);

    txtUser = new QLineEdit(this); 
    txtUser->setPlaceholderText("Tên đăng nhập");
    txtPass = new QLineEdit(this); 
    txtPass->setPlaceholderText("Mật khẩu");
    txtPass->setEchoMode(QLineEdit::Password);

    QPushButton *btnLogin = new QPushButton("ĐĂNG NHẬP", this);
    btnLogin->setObjectName("btnLogin"); // Đặt ID để chỉnh màu riêng
    btnLogin->setCursor(Qt::PointingHandCursor);

    QPushButton *btnReg = new QPushButton("Chưa có tài khoản? Đăng ký ngay", this);
    btnReg->setObjectName("btnLink"); // Đặt ID kiểu Link
    btnReg->setFlat(true);
    btnReg->setCursor(Qt::PointingHandCursor);

    // --- THÊM VÀO LAYOUT (Quan trọng: Qt::AlignHCenter) ---
    // AlignHCenter giúp widget giữ kích thước thật, không bị kéo giãn
    contentLayout->addWidget(lblTitle);
    contentLayout->addSpacing(10); // Khoảng cách nhỏ dưới tiêu đề
    contentLayout->addWidget(txtUser);
    contentLayout->addWidget(txtPass);
    contentLayout->addSpacing(10); // Khoảng cách trước nút bấm
    contentLayout->addWidget(btnLogin);
    contentLayout->addWidget(btnReg);
    contentLayout->setAlignment(Qt::AlignCenter);

    overlayLayout->addWidget(loginContainer);
    mainLayout->setColumnStretch(0, 6); // Bên trái chiếm 4 phần (Rộng hơn)
    mainLayout->setColumnStretch(1, 5);

    // mainLayout->setColumnStretch(0, 1);
    // mainLayout->addWidget(loginContainer, 1, 0, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->addWidget(rightOverlay, 0, 1);

    // -- Logic --
    connect(btnReg, &QPushButton::clicked, this, &LoginWidget::switchToRegister);
    
    connect(btnLogin, &QPushButton::clicked, [=](){
        if(txtUser->text().isEmpty() || txtPass->text().isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đủ thông tin!");
            return;
        }
        GameClient::instance().sendLogin(txtUser->text(), txtPass->text());
    });

    connect(&GameClient::instance(), &GameClient::loginSuccess, [=](){
        // QMessageBox::information(this, "Thông báo", "Đăng nhập thành công!");
        QString user = txtUser->text();
        emit loginSuccess();
    });

    connect(&GameClient::instance(), &GameClient::loginFailed, [=](QString msg, QString code){ // Nhớ thêm tham số code
        qDebug() << "[UI DEBUG] LoginWidget received loginFailed signal!"; // <--- THÊM DÒNG NÀY
        QString errorTitle = "Đăng nhập thất bại";
        QString errorContent = msg; 
      

        // Dịch lỗi sang tiếng Việt dựa vào CODE
        // Lưu ý: Server của bạn đang trả về USER_NOT_FOUND (Viết hoa), hãy dùng code để check
        if (code == "InvalidUsername" || code == "USER_NOT_FOUND" || msg.contains("USER_NOT_FOUND", Qt::CaseInsensitive)) {
            errorContent = "Tài khoản không tồn tại!\nVui lòng kiểm tra lại hoặc đăng ký mới.";
            txtUser->selectAll();
            txtUser->setFocus();
        }
        else if (code == "InvalidPassword" || code == "WRONG_PASSWORD" || msg.contains("WRONG_PASSWORD", Qt::CaseInsensitive)) {
            errorContent = "Sai mật khẩu!\nVui lòng thử lại.";
            txtPass->clear();
            txtPass->setFocus();
        }
        else if (code == "ALREADY_LOGGED_IN" || msg.contains("ALREADY_LOGGED_IN", Qt::CaseInsensitive)) {
            errorContent = "Tài khoản này đang được đăng nhập ở nơi khác!";
        }
        else if (code == "ACCOUNT_LOCKED" || msg.contains("ACCOUNT_LOCKED", Qt::CaseInsensitive)) {
            errorContent = "Tài khoản này đã bị khóa do vi phạm quy định.";
        }

        // --- BỔ SUNG DÒNG NÀY ĐỂ HIỆN HỘP THOẠI ---
        QMessageBox::critical(this, errorTitle, errorContent);
    });
}
