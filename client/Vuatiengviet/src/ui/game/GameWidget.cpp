#include "GameWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent) : QWidget(parent), 
    m_matchId(0), m_timeLimit(10), m_timeElapsed(0), m_totalScore(0), m_answered(false) {
    
    setupUi();
    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GameWidget::onTimerTick);
}

void GameWidget::setupUi() {
    this->setObjectName("GameWidget");
    this->setStyleSheet("#GameWidget { background-color: #1a1a2e; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);
    
    // 1. Header: Question number and timer
    QHBoxLayout *headerLayout = new QHBoxLayout();

    lblPlayerName = new QLabel("Unknown", this);
    lblPlayerName->setStyleSheet("QLabel { color: #3498db; font-size: 20px; font-weight: bold; border: 2px solid #3498db; border-radius: 5px; padding: 5px 10px; }");

    btnSurrender = new QPushButton("Bỏ cuộc", this);
    btnSurrender->setCursor(Qt::PointingHandCursor);
    btnSurrender->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; font-weight: bold; border-radius: 5px; padding: 5px 10px; } QPushButton:hover { background-color: #c0392b; }");
    connect(btnSurrender, &QPushButton::clicked, [=](){
        // Xác nhận (Optional: có thể thêm QMessageBox hỏi lại nếu muốn)
        emit surrenderRequested(m_matchId);
    });
    
    lblQuestionNum = new QLabel("Câu 1/10", this);
    lblQuestionNum->setStyleSheet("QLabel { color: #f1c40f; font-size: 24px; font-weight: bold; }");
    
    lblTimer = new QLabel("10s", this);
    lblTimer->setStyleSheet("QLabel { color: #e74c3c; font-size: 28px; font-weight: bold; }");
    
    lblScore = new QLabel("Điểm: 0", this);
    lblScore->setStyleSheet("QLabel { color: #2ecc71; font-size: 22px; font-weight: bold; }");
    
    headerLayout->addWidget(lblPlayerName); // <--- Đặt tên ở đầu tiên bên trái
    headerLayout->addSpacing(10);
    headerLayout->addWidget(btnSurrender);
    headerLayout->addSpacing(10);

    headerLayout->addWidget(lblQuestionNum);
    headerLayout->addStretch();
    headerLayout->addWidget(lblTimer);
    headerLayout->addStretch();
    headerLayout->addWidget(lblScore);
    
    // 2. Question text
    lblQuestion = new QLabel("Đang tải câu hỏi...", this);
    lblQuestion->setStyleSheet("QLabel { color: white; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; }");
    lblQuestion->setWordWrap(true);
    lblQuestion->setAlignment(Qt::AlignCenter);
    lblQuestion->setMinimumHeight(100);

    // 3. Scoreboard
    lblScoreboard = new QLabel("Điểm: --", this);
    lblScoreboard->setStyleSheet("QLabel { color: #bdc3c7; font-size: 16px; background-color: #0f2137; padding: 12px; border-radius: 8px; }");
    lblScoreboard->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lblScoreboard->setMinimumHeight(80);
    
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(lblQuestion);
    mainLayout->addWidget(lblScoreboard);
    mainLayout->addSpacing(20);

    // ============================================================
    // 4. UI VÒNG 2 & 3: NHẬP TEXT (Container)
    // ============================================================
    containerTextInput = new QWidget(this);
    QVBoxLayout *layoutText = new QVBoxLayout(containerTextInput);
    layoutText->setContentsMargins(0,0,0,0);

    inputAnswer = new QLineEdit(this);
    inputAnswer->setPlaceholderText("Nhập đáp án (không phân biệt hoa thường)");
    inputAnswer->setMinimumHeight(50);
    inputAnswer->setStyleSheet("QLineEdit { background:#0f2137; color:white; font-size:18px; padding:12px; border-radius:8px; }");

    btnSubmit = new QPushButton("Gửi đáp án", this);
    btnSubmit->setMinimumHeight(50);
    btnSubmit->setStyleSheet("QPushButton { background:#1abc9c; color:white; font-size:18px; border-radius:8px; padding:12px; } QPushButton:hover { background:#16a085; }");
    connect(btnSubmit, &QPushButton::clicked, [=]() { onAnswerClicked(inputAnswer->text()); });

    layoutText->addWidget(inputAnswer);
    layoutText->addWidget(btnSubmit);

    // ============================================================
    // 5. UI VÒNG 1: TRẮC NGHIỆM (Container)
    // ============================================================
    containerChoice = new QWidget(this);
    QHBoxLayout *layoutChoice = new QHBoxLayout(containerChoice);
    layoutChoice->setSpacing(20);
    layoutChoice->setContentsMargins(0,0,0,0);

    // Style chung cho nút chọn
    QString btnStyle = "QPushButton { background: #34495e; color: white; font-size: 20px; font-weight: bold; border-radius: 12px; height: 80px; border: 2px solid #2c3e50; } QPushButton:hover { background: #3498db; border-color: #3498db; } QPushButton:pressed { background: #2980b9; }";

    btnOption1 = new QPushButton("Option A", this);
    btnOption1->setStyleSheet(btnStyle);
    btnOption1->setCursor(Qt::PointingHandCursor);
    connect(btnOption1, &QPushButton::clicked, this, &GameWidget::onOptionBtnClicked);

    btnOption2 = new QPushButton("Option B", this);
    btnOption2->setStyleSheet(btnStyle);
    btnOption2->setCursor(Qt::PointingHandCursor);
    connect(btnOption2, &QPushButton::clicked, this, &GameWidget::onOptionBtnClicked);

    layoutChoice->addWidget(btnOption1);
    layoutChoice->addWidget(btnOption2);

    // Thêm cả 2 container vào layout chính
    mainLayout->addWidget(containerTextInput);
    mainLayout->addWidget(containerChoice);
    
    // Mặc định ẩn cả 2, sẽ show khi start game
    containerTextInput->hide();
    containerChoice->hide();

    mainLayout->addStretch();
}

void GameWidget::startGame(int matchId, const QString &questionNum, const QString &questionId, 
                           const QString &questionText, const QString &options, int roundId, int timeLimit) {
    m_matchId = matchId;
    m_currentQuestionId = questionId;
    m_timeLimit = timeLimit;
    m_totalScore = 0;
    m_answered = false;
    
    // Hiển thị vòng chơi
    lblQuestionNum->setText("Vòng " + QString::number(roundId) + " - Câu " + questionNum);
    lblQuestion->setText(questionText);
    
    lblScore->setText("Điểm: 0");
    lblScoreboard->setText("Bảng điểm sẽ cập nhật...");
    
    // Reset style câu hỏi về mặc định
    lblQuestion->setStyleSheet("QLabel { color: white; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; }");

    // --- LOGIC CHUYỂN ĐỔI UI ---
    if (roundId == 1) {
        // VÒNG 1: TRẮC NGHIỆM
        containerTextInput->hide();
        containerChoice->show();

        // Parse options (ví dụ: "sáng lạng | xán lạn")
        QStringList opts = options.split("|");
        if (opts.size() >= 2) {
            btnOption1->setText(opts[0].trimmed());
            btnOption2->setText(opts[1].trimmed());
        } else {
            // Fallback nếu lỗi data
            btnOption1->setText("Lỗi");
            btnOption2->setText("Data");
        }
        
        btnOption1->setEnabled(true);
        btnOption2->setEnabled(true);
    } else {
        // VÒNG 2, 3: NHẬP TỪ
        containerChoice->hide();
        containerTextInput->show();

        inputAnswer->clear();
        inputAnswer->setEnabled(true);
        btnSubmit->setEnabled(true);
        inputAnswer->setFocus(); // Focus vào ô nhập để gõ luôn
    }
    
    resetTimer();
}

void GameWidget::showNextQuestion(const QString &questionNum, const QString &questionId,
                                  const QString &questionText, const QString &options, int roundId, int timeLimit) {
    m_currentQuestionId = questionId;
    m_timeLimit = timeLimit;
    m_answered = false;
    
    lblQuestionNum->setText("Vòng " + QString::number(roundId) + " - Câu " + questionNum);
    lblQuestion->setText(questionText);
    
    // Reset style
    lblQuestion->setStyleSheet("QLabel { color: white; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; }");

    if (roundId == 1) {
        containerTextInput->hide();
        containerChoice->show();

        QStringList opts = options.split("|");
        if (opts.size() >= 2) {
            btnOption1->setText(opts[0].trimmed());
            btnOption2->setText(opts[1].trimmed());
        }
        btnOption1->setEnabled(true);
        btnOption2->setEnabled(true);
    } else {
        containerChoice->hide();
        containerTextInput->show();

        inputAnswer->clear();
        inputAnswer->setEnabled(true);
        btnSubmit->setEnabled(true);
        inputAnswer->setFocus();
    }
    
    resetTimer();
}

void GameWidget::onOptionBtnClicked() {
    if (m_answered) return;
    
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        // Lấy text trên nút làm đáp án
        onAnswerClicked(btn->text());
    }
}

void GameWidget::updateScores(const QList<QPair<QString, QString>> &scores) {
    QStringList lines;
    for (const auto &p : scores) {
        // p.second bây giờ là chuỗi (VD: "50 + (10)")
        lines << QString("%1: %2 điểm").arg(p.first).arg(p.second);

        if (p.first == m_myUsername) {
            // Tách lấy số điểm tổng (nếu muốn) hoặc hiển thị y nguyên
            // Nếu chuỗi là "50 + (10)", hiển thị lên Header cũng sẽ là "Điểm: 50 + (10)"
            lblScore->setText("Điểm: " + p.second);
            
            // Nếu bạn muốn lấy màu mè (VD: đổi màu nếu điểm cao), xử lý ở đây
        }
    }
    
    if (lines.isEmpty()) {
        lblScoreboard->setText("Chờ người chơi...");
    } else {
        lblScoreboard->setText(lines.join("\n"));
    }
}

void GameWidget::showAnswerResult(bool correct, int pointsEarned, int totalScore) {
    m_totalScore = totalScore;
    lblScore->setText("Điểm: " + QString::number(m_totalScore));
    
    // Feedback màu sắc
    if (correct) {
        lblQuestion->setText(lblQuestion->text() + "\n(✓ Chính xác! +" + QString::number(pointsEarned) + ")");
        lblQuestion->setStyleSheet("QLabel { color: #2ecc71; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; border: 2px solid #2ecc71; }");
    } else {
        lblQuestion->setText(lblQuestion->text() + "\n(✗ Sai rồi!)");
        lblQuestion->setStyleSheet("QLabel { color: #e74c3c; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; border: 2px solid #e74c3c; }");
    }
}

void GameWidget::onAnswerClicked(const QString &answer) {
    if (m_answered) return;
    
    m_answered = true;
    m_timer->stop();
    
    // Khóa UI
    inputAnswer->setEnabled(false);
    btnSubmit->setEnabled(false);
    btnOption1->setEnabled(false);
    btnOption2->setEnabled(false);
    
    emit answerSubmitted(m_matchId, answer, m_timeElapsed);
}

void GameWidget::onTimerTick() {
    m_timeElapsed++;
    int remaining = m_timeLimit - m_timeElapsed;
    
    if (remaining <= 0) {
        m_timer->stop();
        lblTimer->setText("0s");
        // Auto-submit rỗng khi hết giờ
        if (!m_answered) {
            onAnswerClicked(""); 
        }
    } else {
        lblTimer->setText(QString::number(remaining) + "s");
        
        // Đổi màu khi sắp hết giờ
        if (remaining <= 3) {
            lblTimer->setStyleSheet("QLabel { color: #ff0000; font-size: 32px; font-weight: bold; }");
        } else if (remaining <= 5) {
            lblTimer->setStyleSheet("QLabel { color: #ff6b6b; font-size: 28px; font-weight: bold; }");
        }
    }
}

void GameWidget::resetTimer() {
    m_timeElapsed = 0;
    lblTimer->setText(QString::number(m_timeLimit) + "s");
    lblTimer->setStyleSheet("QLabel { color: #2ecc71; font-size: 28px; font-weight: bold; }");
    m_timer->start(1000); 
}

void GameWidget::stopGame() {
    if (m_timer->isActive()) {
        m_timer->stop();
    }

    inputAnswer->clear();
    inputAnswer->setEnabled(false);
    btnSubmit->setEnabled(false);
    btnOption1->setEnabled(false);
    btnOption2->setEnabled(false);
    
    m_answered = true;
}
void GameWidget::setPlayerName(const QString &name) {
    m_myUsername = name;
    if (lblPlayerName) {
        lblPlayerName->setText("👤 " + name); // Thêm icon cho đẹp
    }
}