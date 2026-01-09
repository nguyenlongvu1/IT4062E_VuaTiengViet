#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QList>
#include <QPair>

class GameWidget : public QWidget {
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);

    // CẬP NHẬT: Thêm options và roundId vào tham số
    void startGame(int matchId, const QString &questionNum, const QString &questionId, 
                   const QString &questionText, const QString &options, int roundId, int timeLimit);

    void showNextQuestion(const QString &questionNum, const QString &questionId,
                          const QString &questionText, const QString &options, int roundId, int timeLimit);

    void updateScores(const QList<QPair<QString,int>> &scores);
    void showAnswerResult(bool correct, int pointsEarned, int totalScore);
    void stopGame();

signals:
    void answerSubmitted(int matchId, const QString &answer, int timeElapsed);
    void surrenderRequested(int matchId);

private slots:
    void onAnswerClicked(const QString &answer);
    void onTimerTick();
    void onOptionBtnClicked(); // Slot mới xử lý khi bấm nút chọn (Vòng 1)

private:
    QPushButton *btnSurrender;
    void setupUi();
    void resetTimer();

    // UI Elements
    QLabel *lblQuestionNum;
    QLabel *lblTimer;
    QLabel *lblScore;
    QLabel *lblQuestion;
    QLabel *lblScoreboard;

    // --- Container cho Vòng 2, 3 (Nhập text) ---
    QWidget *containerTextInput;
    QLineEdit *inputAnswer;
    QPushButton *btnSubmit;

    // --- Container cho Vòng 1 (Chọn đáp án) ---
    QWidget *containerChoice;
    QPushButton *btnOption1;
    QPushButton *btnOption2;

    // Game State
    QTimer *m_timer;
    int m_matchId;
    QString m_currentQuestionId;
    int m_timeLimit;
    int m_timeElapsed;
    int m_totalScore;
    bool m_answered;
};

#endif // GAMEWIDGET_H