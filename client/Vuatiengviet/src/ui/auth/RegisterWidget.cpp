#include "RegisterWidget.h"
#include "GameClient.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

RegisterWidget::RegisterWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *lblTitle = new QLabel("ĐĂNG KÝ", this);
    QFont f = lblTitle->font(); f.setPointSize(18); lblTitle->setFont(f);
    lblTitle->setAlignment(Qt::AlignCenter);

    txtUser = new QLineEdit(this); txtUser->setPlaceholderText("Tên đăng nhập");
    txtPass = new QLineEdit(this); txtPass->setPlaceholderText("Mật khẩu");
    txtPass->setEchoMode(QLineEdit::Password);
    txtConfirm = new QLineEdit(this); txtConfirm->setPlaceholderText("Nhập lại mật khẩu");
    txtConfirm->setEchoMode(QLineEdit::Password);

    QPushButton *btnReg = new QPushButton("Đăng Ký", this);
    QPushButton *btnBack = new QPushButton("Quay lại", this);

    layout->addWidget(lblTitle);
    layout->addWidget(txtUser);
    layout->addWidget(txtPass);
    layout->addWidget(txtConfirm);
    layout->addWidget(btnReg);
    layout->addWidget(btnBack);

    connect(btnBack, &QPushButton::clicked, this, &RegisterWidget::backToLogin);

    connect(btnReg, &QPushButton::clicked, [=](){
        if(txtPass->text() != txtConfirm->text()) {
            QMessageBox::warning(this, "Lỗi", "Mật khẩu xác nhận không khớp!");
            return;
        }
        GameClient::instance().sendRegister(txtUser->text(), txtPass->text());
    });

    connect(&GameClient::instance(), &GameClient::registerSuccess, [=](){
        QMessageBox::information(this, "Thành công", "Đăng ký thành công! Hãy đăng nhập.");
        emit backToLogin();
    });
    
    connect(&GameClient::instance(), &GameClient::registerFailed, [=](QString msg){
        QMessageBox::critical(this, "Lỗi", msg);
    });
}