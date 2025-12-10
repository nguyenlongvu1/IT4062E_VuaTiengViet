#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
class QLineEdit; // Forward declaration giúp biên dịch nhanh hơn

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void switchToRegister(); // Chuyển sang màn đăng ký
    void loginSuccess();     // Chuyển sang màn Home

private:
    QLineEdit *txtUser;
    QLineEdit *txtPass;
};

#endif // LOGINWIDGET_H