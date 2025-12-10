#include "MainWindow.h"
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "GameClient.h"
#include <QStackedWidget>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(800, 600);
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 1. Khởi tạo các màn hình
    LoginWidget *loginScreen = new LoginWidget(this);
    RegisterWidget *registerScreen = new RegisterWidget(this);
    
    // Tạm thời dùng Label làm màn hình Home (sẽ thay thế bằng HomeWidget sau)
    QLabel *homePlaceholder = new QLabel("LOBBY SCREEN (Đang phát triển)", this);
    homePlaceholder->setAlignment(Qt::AlignCenter);

    // 2. Thêm vào Stack (Thứ tự index: 0, 1, 2)
    m_stackedWidget->addWidget(loginScreen);    // Index 0
    m_stackedWidget->addWidget(registerScreen); // Index 1
    m_stackedWidget->addWidget(homePlaceholder);// Index 2

    // 3. Xử lý điều hướng
    connect(loginScreen, &LoginWidget::switchToRegister, [=](){
        m_stackedWidget->setCurrentIndex(1);
    });

    connect(registerScreen, &RegisterWidget::backToLogin, [=](){
        m_stackedWidget->setCurrentIndex(0);
    });

    connect(loginScreen, &LoginWidget::loginSuccess, [=](){
        m_stackedWidget->setCurrentIndex(2); // Vào Lobby
    });

    // 4. Kết nối tới Server khi mở App
    GameClient::instance().connectToServer("127.0.0.1", 8080);
    
    // Báo lỗi nếu không kết nối được
    connect(&GameClient::instance(), &GameClient::disconnected, [=](){
       // Có thể hiện popup báo mất kết nối tại đây
    });
}

MainWindow::~MainWindow() {}