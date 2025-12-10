#include "LoginWidget.h"
#include "GameClient.h" // Nhờ INCLUDEPATH trong .pro nên không cần gõ ../../network
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *lblTitle = new QLabel("VUA TIẾNG VIỆT", this);
    QFont f = lblTitle->font(); f.setPointSize(20); lblTitle->setFont(f);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setObjectName("lblTitle");

    txtUser = new QLineEdit(this); txtUser->setPlaceholderText("Tên đăng nhập");
    txtPass = new QLineEdit(this); txtPass->setPlaceholderText("Mật khẩu");
    txtPass->setEchoMode(QLineEdit::Password);

    QPushButton *btnLogin = new QPushButton("Đăng Nhập", this);
    QPushButton *btnReg = new QPushButton("Chưa có tài khoản? Đăng ký", this);
    btnReg->setObjectName("btnRegister");
    btnReg->setFlat(true);

    layout->addWidget(lblTitle);
    layout->addSpacing(20);
    layout->addWidget(txtUser);
    layout->addWidget(txtPass);
    layout->addWidget(btnLogin);
    layout->addWidget(btnReg);

    // -- Logic --
    // Chuyển màn hình
    connect(btnReg, &QPushButton::clicked, this, &LoginWidget::switchToRegister);

    // Gửi yêu cầu đăng nhập
    connect(btnLogin, &QPushButton::clicked, [=](){
        GameClient::instance().sendLogin(txtUser->text(), txtPass->text());
    });

    // Lắng nghe phản hồi từ mạng
    connect(&GameClient::instance(), &GameClient::loginSuccess, [=](){
        QMessageBox::information(this, "Thông báo", "Đăng nhập thành công!");
        emit loginSuccess();
    });

    connect(&GameClient::instance(), &GameClient::loginFailed, [=](QString msg){
        QMessageBox::critical(this, "Lỗi", msg);
    });
}