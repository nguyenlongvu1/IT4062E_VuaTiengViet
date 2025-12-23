#ifndef LEADERBOARDWIDGET_H
#define LEADERBOARDWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "GameClient.h"

class LeaderboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit LeaderboardWidget(QWidget *parent = nullptr);
    void updateData(); 
public slots:
    void updateLeaderboard(const QList<RankItem> &items);

private:
    QListWidget *listRank;
    void setupUi();
};

#endif 