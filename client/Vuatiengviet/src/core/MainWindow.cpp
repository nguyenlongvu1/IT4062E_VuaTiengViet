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
#include <QTemporaryFile>
#include "../utils/AudioManager.h"
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setMinimumSize(1024, 768); 
    this->resize(1024, 768);
    this->showFullScreen();
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

   QFile resourceFile(":/bgMusic.ogg");
    if (resourceFile.open(QIODevice::ReadOnly)) {
        
        // Tạo file tạm, đặt tên pattern để dễ nhận biết
        m_tempMusicFile = new QTemporaryFile("qt_temp_music_XXXXXX.ogg");
        
        // Quan trọng: setAutoRemove(true) để khi tắt app file tự xóa
        m_tempMusicFile->setAutoRemove(true); 
        
        if (m_tempMusicFile->open()) {
            m_tempMusicFile->write(resourceFile.readAll());
            m_tempMusicFile->flush(); // Đảm bảo ghi xong xuống đĩa
            m_tempMusicFile->close(); // Đóng handle để SDL có thể mở lại
            
            // 2. Lấy đường dẫn thật
            QString realPath = m_tempMusicFile->fileName();
            qDebug() << "Music extracted to:" << realPath;

            // 3. Phát nhạc
            AudioManager::instance().playBackgroundMusic(realPath.toStdString());
        }
    } else {
        qDebug() << "ERROR: Cannot find music resource!";
    }
    QFile clickRes(":/clickSound.wav");
if (clickRes.open(QIODevice::ReadOnly)) {

    QTemporaryFile* tmpClick = new QTemporaryFile("qt_temp_click_XXXXXX.wav");
    tmpClick->setAutoRemove(true);

    if (tmpClick->open()) {
        tmpClick->write(clickRes.readAll());
        tmpClick->flush();
        tmpClick->close();

        QString clickPath = tmpClick->fileName();
        qDebug() << "Click sound extracted to:" << clickPath;
AudioManager::instance().loadClickSound(clickPath.toStdString());


    }
}

    
}

MainWindow::~MainWindow() {
    // Khi cửa sổ đóng:
    AudioManager::instance().stopMusic();
    
    // m_tempMusicFile sẽ tự động được delete nhờ cơ chế của Qt (nếu set parent)
    // hoặc ta delete thủ công ở đây:
    if (m_tempMusicFile) {
        delete m_tempMusicFile; // File trên ổ cứng sẽ biến mất luôn
    }
}