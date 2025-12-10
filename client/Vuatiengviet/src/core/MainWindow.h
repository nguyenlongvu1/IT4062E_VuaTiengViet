#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *m_stackedWidget;
};

#endif // MAINWINDOW_H