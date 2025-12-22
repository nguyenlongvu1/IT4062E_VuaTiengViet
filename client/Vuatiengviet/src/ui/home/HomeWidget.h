#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include "LeaderboardWidget.h" 
#include "SocialWidget.h"

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class NotificationDialog;
class HomeWidget : public QWidget {
    Q_OBJECT
public:
    explicit HomeWidget(QWidget *parent = nullptr);
    
    // Hàm cập nhật thông tin người chơi (gọi sau khi Login thành công)
    void setPlayerInfo(const QString& name, int score);

protected:
    // Vẽ hình nền cầu vồng
    void paintEvent(QPaintEvent *) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }

signals:
    void logout();       // Signal đăng xuất
     // Signal tạo phòng

private:
    QString getRankNameFromScore(int score);
    // UI Components
    QLabel *lblAvatar;
    QLabel *lblUsername;
    QLabel *lblRank;

    QToolButton *btnRank;
    QToolButton *btnFriend;
    LeaderboardWidget *leaderboardWidget; 
    SocialWidget *socialWidget;
    void openSettings();
    void openHistory();
    void setupUi();
     void playRanked();    // Signal tìm trận
    void playWithFriend(); 
    void openInbox();
    QString getRankName(int score);

    QString m_currentUsername;
    int m_currentScore;
    QString m_currentRankName;
    QPushButton *btnInbox;
    NotificationDialog *m_notifyDialog = nullptr;
    void joinRankedRoom(const QString& roomId);
    bool m_isProcessingMatch = false;
};

#endif // HOMEWIDGET_H