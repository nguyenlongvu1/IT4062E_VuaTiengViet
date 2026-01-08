#include "MainWindow.h"
#include "LoginWidget.h"
#include "RegisterWidget.h"
#include "HomeWidget.h"
#include "GameClient.h"
#include "GameWidget.h"
#include "ResultWidget.h"
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
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 1. Khởi tạo các màn hình
    LoginWidget *loginScreen = new LoginWidget(this);
    RegisterWidget *registerScreen = new RegisterWidget(this);
    HomeWidget *homeScreen = new HomeWidget(this);
    GameWidget *gameScreen = new GameWidget(this);
    ResultWidget *resultScreen = new ResultWidget(this);
    

    // 2. Thêm vào Stack (Thứ tự index: 0, 1, 2, 3, 4)
    m_stackedWidget->addWidget(loginScreen);    // Index 0
    m_stackedWidget->addWidget(registerScreen); // Index 1
    m_stackedWidget->addWidget(homeScreen);     // Index 2
    m_stackedWidget->addWidget(gameScreen);     // Index 3
    m_stackedWidget->addWidget(resultScreen);   // Index 4

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

    // Khi match được server tạo thành công, chuyển ngay sang màn game (chờ câu hỏi)
    connect(&GameClient::instance(), &GameClient::matchStartedDirectly,
            [=](QString matchId, QString roomId) {
        Q_UNUSED(matchId);
        Q_UNUSED(roomId);
        m_stackedWidget->setCurrentWidget(gameScreen);
    });

    // 5. Kết nối Game Flow
    // Khi nhận được câu hỏi đầu tiên -> chuyển sang GameWidget
    connect(&GameClient::instance(), &GameClient::gameQuestionReceived, 
            [=](int matchId, QString questionNum, QString questionId, QString questionText, int timeLimit) {
        if (questionNum.toInt() == 1) {
            // Câu hỏi đầu tiên -> chuyển sang màn game
            m_stackedWidget->setCurrentWidget(gameScreen);
        }
        gameScreen->startGame(matchId, questionNum, questionId, questionText, timeLimit);
    });

    // Nhận kết quả trả lời -> cập nhật điểm trên GameWidget
    connect(&GameClient::instance(), &GameClient::answerResultReceived,
            gameScreen, &GameWidget::showAnswerResult);

    // Nhận câu tiếp theo
        connect(&GameClient::instance(), &GameClient::nextQuestionReceived,
            gameScreen, &GameWidget::showNextQuestion);

        // Cập nhật bảng điểm
        connect(&GameClient::instance(), &GameClient::gameScoresUpdated,
            gameScreen, &GameWidget::updateScores);

    // Khi player submit answer -> gửi lên server
    connect(gameScreen, &GameWidget::answerSubmitted, 
            [=](int matchId, const QString& answer, int timeElapsed) {
        GameClient::instance().sendAnswer(matchId, answer, timeElapsed);
    });

    // Khi game kết thúc -> hiện kết quả
    connect(&GameClient::instance(), &GameClient::gameEnded,
            [=](const QList<QPair<QString, int>>& rankings, const QString& winnerId) {
        resultScreen->showResults(rankings, winnerId);
        m_stackedWidget->setCurrentWidget(resultScreen);
    });

    // Khi click Return to Lobby -> về HomeWidget
    connect(resultScreen, &ResultWidget::returnToLobby, [=]() {
        m_stackedWidget->setCurrentWidget(homeScreen);
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

  AudioManager::instance().playBackgroundMusic(":/bgMusic.ogg");
    AudioManager::instance().loadClickSound(":/clickSound.wav");

    
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