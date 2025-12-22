#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H
#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
class QLineEdit; // Forward declaration giúp biên dịch nhanh hơn
class QPushButton;
class QLabel;
class QFrame;
class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);
protected:
    // Hàm này bắt buộc phải có để hiện hình nền trên Widget tự tạo
    void paintEvent(QPaintEvent *) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
signals:
    void switchToRegister(); // Chuyển sang màn đăng ký
    void loginSuccess(const QString &username, int score); // Chuyển sang màn Home

private:
    QFrame *loginContainer;
    QLineEdit *txtUser;
    QLineEdit *txtPass;
    QPushButton *btnLogin;
    QPushButton *btnReg;
};

#endif // LOGINWIDGET_H