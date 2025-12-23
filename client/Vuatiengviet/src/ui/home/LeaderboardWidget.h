#ifndef LEADERBOARDWIDGET_H
#define LEADERBOARDWIDGET_H

#include <QWidget>
#include <QListWidget>
#include "GameClient.h"

class LeaderboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit LeaderboardWidget(QWidget *parent = nullptr);
    void updateData(); // Hàm để load dữ liệu giả lập hoặc thật
public slots:
    void updateLeaderboard(const QList<RankItem> &items);

private:
    QListWidget *listRank;
    void setupUi();
};

#endif // LEADERBOARDWIDGET_H