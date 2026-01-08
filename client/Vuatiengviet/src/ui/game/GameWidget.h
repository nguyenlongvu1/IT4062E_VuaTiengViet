#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

class GameButton;

class GameWidget : public QWidget {
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);
    
    void startGame(int matchId, const QString &questionNum, const QString &questionId, 
                   const QString &questionText, int timeLimit);
    void showNextQuestion(const QString &questionNum, const QString &questionId,
                         const QString &questionText, int timeLimit);
    void showAnswerResult(bool correct, int pointsEarned, int totalScore);
    void updateScores(const QList<QPair<QString,int>> &scores);
    
signals:
    void answerSubmitted(int matchId, const QString &answer, int timeElapsed);
    void gameFinished();
    
private slots:
    void onAnswerClicked(const QString &answer);
    void onTimerTick();
    
private:
    void setupUi();
    void resetTimer();
    
    int m_matchId;
    int m_timeLimit;
    int m_timeElapsed;
    int m_totalScore;
    
    QLabel *lblQuestionNum;
    QLabel *lblQuestion;
    QLabel *lblTimer;
    QLabel *lblScore;
    QLabel *lblScoreboard;
    
    QLineEdit *inputAnswer;
    QPushButton *btnSubmit;
    
    QTimer *m_timer;
    
    QString m_currentQuestionId;
    bool m_answered;
};

#endif // GAMEWIDGET_H
