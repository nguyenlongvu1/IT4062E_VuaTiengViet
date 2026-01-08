#ifndef RESULTWIDGET_H
#define RESULTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class GameButton;

class ResultWidget : public QWidget {
    Q_OBJECT
public:
    explicit ResultWidget(QWidget *parent = nullptr);
    
    void showResults(const QList<QPair<QString, int>> &rankings, const QString &winnerId);
    
signals:
    void returnToLobby();
    
private:
    void setupUi();
    
    QLabel *lblTitle;
    QLabel *lblWinner;
    QLabel *lblRank1;
    QLabel *lblRank2;
    QLabel *lblRank3;
    GameButton *btnReturn;
};

#endif // RESULTWIDGET_H
