#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
class QLineEdit; 
class QPushButton;
class QLabel;
class QFrame;
class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
signals:
    void switchToRegister();
    void loginSuccess();    

private:
    QFrame *loginContainer;
    QLineEdit *txtUser;
    QLineEdit *txtPass;
    QPushButton *btnLogin;
    QPushButton *btnReg;
};

#endif // LOGINWIDGET_H