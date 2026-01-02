#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../ui/room/FriendRoomWidget.h"
#include <QTemporaryFile>
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *m_stackedWidget;
    FriendRoomWidget *friendRoomWidget;
    QString m_currentUsername;
    QTemporaryFile* m_tempMusicFile = nullptr;
};

#endif // MAINWINDOW_H