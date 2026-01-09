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
#include "GameWidget.h"
#include "ResultWidget.h"
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setMinimumSize(1024, 768); 
    this->resize(1280, 800);
    // Run in windowed mode by default to avoid forced fullscreen
    // this->showFullScreen();
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 1. Khởi tạo các màn hình
    LoginWidget *loginScreen = new LoginWidget(this);
    RegisterWidget *registerScreen = new RegisterWidget(this);
    HomeWidget *homeScreen = new HomeWidget(this);
    GameWidget *gameScreen = new GameWidget(this);
    ResultWidget *resultScreen = new ResultWidget(this);
    

    // 2. Thêm vào Stack (Thứ tự index: 0, 1, 2)
    m_stackedWidget->addWidget(loginScreen);    // Index 0
    m_stackedWidget->addWidget(registerScreen); // Index 1
    m_stackedWidget->addWidget(homeScreen);// Index 2
     m_stackedWidget->addWidget(gameScreen);     // Index 3
    m_stackedWidget->addWidget(resultScreen);

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
     connect(&GameClient::instance(), &GameClient::matchStartedDirectly,
            [=](QString matchId, QString roomId) {
        Q_UNUSED(matchId);
        Q_UNUSED(roomId);
        m_stackedWidget->setCurrentWidget(gameScreen);
    });

    // 5. Kết nối Game Flow
    // Khi nhận được câu hỏi đầu tiên -> chuyển sang GameWidget
    connect(&GameClient::instance(), &GameClient::gameQuestionReceived, this, 
    [=](int matchId, QString questionNum, QString questionId, QString questionText, QString options, int roundId, int timeLimit) {
        // Code xử lý UI
        if (questionNum.toInt() == 1) {
             m_stackedWidget->setCurrentWidget(gameScreen);
        }
        gameScreen->startGame(matchId, questionNum, questionId, questionText, options, roundId, timeLimit);
});
    

    // Nhận kết quả trả lời -> cập nhật điểm trên GameWidget
    connect(&GameClient::instance(), &GameClient::answerResultReceived,
            gameScreen, &GameWidget::showAnswerResult);

    // Nhận câu tiếp theo
        connect(&GameClient::instance(), &GameClient::nextQuestionReceived, this, 
    // THÊM "int matchId" VÀO ĐẦU DANH SÁCH THAM SỐ
    [=](int matchId, QString questionNum, QString questionId, QString questionText, QString options, int roundId, int timeLimit) {
        
        // Nếu hàm showNextQuestion của bạn không cần matchId, bạn cứ để matchId ở đó nhưng không dùng.
        // Hoặc nếu cần thì truyền vào.
        
        if (gameScreen) {
             // Giữ nguyên các tham số truyền vào hàm hiển thị (trừ khi hàm này cũng thay đổi)
             gameScreen->showNextQuestion(questionNum, questionId, questionText, options, roundId, timeLimit);
        }
    }
);

        // Cập nhật bảng điểm
        connect(&GameClient::instance(), &GameClient::gameScoresUpdated, 
            this, [=](const QList<QPair<QString, QString>> &scores) { // SỬA: int -> QString
        if (gameScreen) {
            gameScreen->updateScores(scores);
        }
    });

    // Khi player submit answer -> gửi lên server
    connect(gameScreen, &GameWidget::answerSubmitted, 
            [=](int matchId, const QString& answer, int timeElapsed) {
        GameClient::instance().sendAnswer(matchId, answer, timeElapsed);
    });

    // Khi game kết thúc -> hiện kết quả
    // MainWindow.cpp

// Tìm đến đoạn connect gameEnded trong Constructor
connect(&GameClient::instance(), &GameClient::gameEnded,
        [=](const QList<QPair<QString, int>>& rankings, const QString& winnerId) {
    
    // 1. Dừng màn chơi game ngay lập tức (Timer, Input)
    if (gameScreen) {
        gameScreen->stopGame();
    }

    // 2. Hiển thị kết quả
    resultScreen->showResults(rankings, winnerId);
    
    // 3. Chuyển màn hình
    m_stackedWidget->setCurrentWidget(resultScreen);
});
    // Khi click Return to Lobby -> về HomeWidget
    connect(resultScreen, &ResultWidget::returnToLobby, [=]() {
        m_stackedWidget->setCurrentWidget(homeScreen);
    });
    connect(&GameClient::instance(), &GameClient::playerEliminated, this, [=]() {
        // 1. Dừng game
        if (gameScreen) gameScreen->stopGame();
        
        // 2. Hiện thông báo
        QMessageBox::information(this, "Thông báo", "Rất tiếc, bạn đã bị loại vì điểm thấp nhất vòng này!");
        
        // 3. Về màn hình chính
        m_stackedWidget->setCurrentWidget(homeScreen);
        
        // 4. Refresh lại trạng thái
        GameClient::instance().sendGetRoomInfo();
    });
    connect(gameScreen, &GameWidget::surrenderRequested, this, [=](int matchId) {
    // 1. Gửi lệnh lên Server
    GameClient::instance().sendSurrender(matchId);
    
    // 2. Dừng game ở Client
    gameScreen->stopGame();
    
    // 3. Chuyển ngay về màn hình Home (Lobby)
    m_stackedWidget->setCurrentWidget(homeScreen);
    
    // 4. (Tuỳ chọn) Reset trạng thái phòng
    GameClient::instance().sendGetRoomInfo(); 
});
    connect(&GameClient::instance(), &GameClient::userInfoReceived, 
            this, [=](const QString &username, int points, const QString &rank) {
        
        // Cập nhật tên vào màn hình Game ngay khi đăng nhập xong
        if (gameScreen) {
            gameScreen->setPlayerName(username);
        }

        // (Code cũ của bạn nếu có xử lý hiển thị điểm/rank ở Home thì giữ nguyên)
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