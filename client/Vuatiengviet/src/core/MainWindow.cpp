#include "MainWindow.h"
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "HomeWidget.h"
#include "GameClient.h"
#include <QStackedWidget>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include "../ui/room/FriendRoomWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(1024, 768);
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 1. Khởi tạo các màn hình
    LoginWidget *loginScreen = new LoginWidget(this);
    RegisterWidget *registerScreen = new RegisterWidget(this);
    HomeWidget *homeScreen = new HomeWidget(this);
    

    // 2. Thêm vào Stack (Thứ tự index: 0, 1, 2)
    m_stackedWidget->addWidget(loginScreen);    // Index 0
    m_stackedWidget->addWidget(registerScreen); // Index 1
    m_stackedWidget->addWidget(homeScreen);// Index 2

    // 3. Xử lý điều hướng
    connect(loginScreen, &LoginWidget::switchToRegister, [=](){
        m_stackedWidget->setCurrentIndex(1);
    });

    connect(registerScreen, &RegisterWidget::backToLogin, [=](){
        m_stackedWidget->setCurrentIndex(0);
    });

    connect(loginScreen, &LoginWidget::loginSuccess, [=](){
        m_stackedWidget->setCurrentWidget(homeScreen);
    });
   
    connect(homeScreen, &HomeWidget::logout, [=](){
        GameClient::instance().sendLogout();
        // Quay về màn hình Login
        m_stackedWidget->setCurrentWidget(loginScreen);
    });

    // 4. Kết nối tới Server khi mở App
    GameClient::instance().connectToServer("127.0.0.1", 8080);
    
    // Báo lỗi nếu không kết nối được
    connect(&GameClient::instance(), &GameClient::disconnected, [=](){
       // Có thể hiện popup báo mất kết nối tại đây
    });
    // TỰ ĐỘNG KẾT NỐI VÀ LOGIN LUÔN
    QTimer::singleShot(500, [=](){
        if (!GameClient::instance().isConnected()) {
            GameClient::instance().connectToServer("127.0.0.1", 12345);
        }
    });

   

    
}

MainWindow::~MainWindow() {}