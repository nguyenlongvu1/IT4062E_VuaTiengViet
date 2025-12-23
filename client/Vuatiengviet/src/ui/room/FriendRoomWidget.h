#ifndef FRIENDROOMWIDGET_H
#define FRIENDROOMWIDGET_H

#include <QWidget>
#include <QList> 
#include <QLabel> 
#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;

class FriendRoomWidget : public QWidget {
    Q_OBJECT
public:
   
    explicit FriendRoomWidget(QString myUsername, bool isHost, QWidget *parent = nullptr);
    
    void updateSlot(int index, const QString& name, bool isReady, bool isEmpty);
    void setRoomID(const QString& id);
    void setHostInfo(const QString& username);
    void updateMembers(const QString& p1, const QString& p2, const QString& p3);
    

signals:
    void leftRoom();
    void startGame();
private slots:
    void onJoinRoomClicked(); 
    // void onLeaveBtnClicked(); 
    // void onStartBtnClicked();
    void onLeaveBtnClicked();
private:
    QString m_myUsername;
    bool m_isHost;
    QLabel *lblRoomID;
    
    QList<QWidget*> playerSlots; 

    QPushButton *btnAction; 

    void setupUi();
    QWidget* createPlayerSlot(int index); 
    void updateGuestSlot(QFrame* frame, QLabel* lblName, QLabel* lblAvatar, QLabel* lblStatus, const QString& playerName);
    QLabel *lblUser1;   // Tên chủ phòng (Slot 1)
    QLabel *lblStatus1; // Trạng thái Slot 1
    QFrame *frameSlot1; // Khung viền Slot 1
    
    QLabel *lblUser2;   // Tên khách (Slot 2)
    QLabel *lblStatus2; // Trạng thái Slot 2
    QFrame *frameSlot2; // Khung viền Slot 2
    QLabel *lblAvatar1; // Avatar Host
    QLabel *lblAvatar2;

    QFrame *frameSlot3;
    QLabel *lblAvatar3;
    QLabel *lblUser3;
    QLabel *lblStatus3; 
};

#endif // FRIENDROOMWIDGET_H