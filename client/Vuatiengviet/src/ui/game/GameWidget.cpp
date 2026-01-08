#include "GameWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QLineEdit>
#include <QPushButton>

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
    
    // Header: Question number and timer
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    lblQuestionNum = new QLabel("Câu 1/10", this);
    lblQuestionNum->setStyleSheet("QLabel { color: #f1c40f; font-size: 24px; font-weight: bold; }");
    
    lblTimer = new QLabel("10s", this);
    lblTimer->setStyleSheet("QLabel { color: #e74c3c; font-size: 28px; font-weight: bold; }");
    
    lblScore = new QLabel("Điểm: 0", this);
    lblScore->setStyleSheet("QLabel { color: #2ecc71; font-size: 22px; font-weight: bold; }");
    
    headerLayout->addWidget(lblQuestionNum);
    headerLayout->addStretch();
    headerLayout->addWidget(lblTimer);
    headerLayout->addStretch();
    headerLayout->addWidget(lblScore);
    
    // Question text
    lblQuestion = new QLabel("Nhập đáp án đúng:", this);
    lblQuestion->setStyleSheet("QLabel { color: white; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; }");
    lblQuestion->setWordWrap(true);
    lblQuestion->setAlignment(Qt::AlignCenter);
    lblQuestion->setMinimumHeight(100);

    // Scoreboard
    lblScoreboard = new QLabel("Điểm: --", this);
    lblScoreboard->setStyleSheet("QLabel { color: #bdc3c7; font-size: 16px; background-color: #0f2137; padding: 12px; border-radius: 8px; }");
    lblScoreboard->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    lblScoreboard->setMinimumHeight(80);
    
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(lblQuestion);
    mainLayout->addWidget(lblScoreboard);
    mainLayout->addSpacing(20);

    // Text input answer
    inputAnswer = new QLineEdit(this);
    inputAnswer->setPlaceholderText("Nhập đáp án (không phân biệt hoa thường)");
    inputAnswer->setMinimumHeight(50);
    inputAnswer->setStyleSheet("QLineEdit { background:#0f2137; color:white; font-size:18px; padding:12px; border-radius:8px; }");

    btnSubmit = new QPushButton("Gửi đáp án", this);
    btnSubmit->setMinimumHeight(50);
    btnSubmit->setStyleSheet("QPushButton { background:#1abc9c; color:white; font-size:18px; border-radius:8px; padding:12px; } QPushButton:hover { background:#16a085; }");
    connect(btnSubmit, &QPushButton::clicked, [=]() { onAnswerClicked(inputAnswer->text()); });

    mainLayout->addWidget(inputAnswer);
    mainLayout->addWidget(btnSubmit);
    mainLayout->addStretch();
}

void GameWidget::startGame(int matchId, const QString &questionNum, const QString &questionId, 
                           const QString &questionText, int timeLimit) {
    m_matchId = matchId;
    m_currentQuestionId = questionId;
    m_timeLimit = timeLimit;
    m_totalScore = 0;
    m_answered = false;
    
    lblQuestionNum->setText("Câu " + questionNum + "/10");
    lblQuestion->setText(questionText);
    lblScore->setText("Điểm: 0");
    lblScoreboard->setText("Điểm từng người sẽ hiển thị tại đây");
    inputAnswer->clear();
    inputAnswer->setEnabled(true);
    btnSubmit->setEnabled(true);
    
    resetTimer();
}

void GameWidget::showNextQuestion(const QString &questionNum, const QString &questionId,
                                   const QString &questionText, int timeLimit) {
    m_currentQuestionId = questionId;
    m_timeLimit = timeLimit;
    m_answered = false;
    
    lblQuestionNum->setText("Câu " + questionNum + "/10");
    lblQuestion->setText(questionText);
    inputAnswer->clear();
    inputAnswer->setEnabled(true);
    btnSubmit->setEnabled(true);
    
    resetTimer();
}

void GameWidget::updateScores(const QList<QPair<QString,int>> &scores) {
    QStringList lines;
    for (const auto &p : scores) {
        lines << QString("ID %1: %2 điểm").arg(p.first).arg(p.second);
    }
    if (lines.isEmpty()) {
        lblScoreboard->setText("Chưa có điểm");
    } else {
        lblScoreboard->setText(lines.join("\n"));
    }
}

void GameWidget::showAnswerResult(bool correct, int pointsEarned, int totalScore) {
    m_totalScore = totalScore;
    lblScore->setText("Điểm: " + QString::number(m_totalScore));
    
    // Show feedback briefly
    if (correct) {
        lblQuestion->setText("✓ Đúng! +" + QString::number(pointsEarned) + " điểm");
        lblQuestion->setStyleSheet("QLabel { color: #2ecc71; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; }");
    } else {
        lblQuestion->setText("✗ Sai! +0 điểm");
        lblQuestion->setStyleSheet("QLabel { color: #e74c3c; font-size: 20px; padding: 20px; background-color: #16213e; border-radius: 10px; }");
    }
}

void GameWidget::onAnswerClicked(const QString &answer) {
    if (m_answered) return;
    
    m_answered = true;
    m_timer->stop();
    
    inputAnswer->setEnabled(false);
    btnSubmit->setEnabled(false);
    
    emit answerSubmitted(m_matchId, answer, m_timeElapsed);
}

void GameWidget::onTimerTick() {
    m_timeElapsed++;
    int remaining = m_timeLimit - m_timeElapsed;
    
    if (remaining <= 0) {
        m_timer->stop();
        lblTimer->setText("0s");
        // Auto-submit empty answer
        if (!m_answered) {
            m_answered = true;
            inputAnswer->setEnabled(false);
            btnSubmit->setEnabled(false);
            emit answerSubmitted(m_matchId, "", m_timeElapsed);
        }
    } else {
        lblTimer->setText(QString::number(remaining) + "s");
        
        // Change color when time is running out
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
    m_timer->start(1000); // 1 second interval
}
