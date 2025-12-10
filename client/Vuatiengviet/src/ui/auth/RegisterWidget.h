#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
class QLineEdit;

class RegisterWidget : public QWidget {
    Q_OBJECT
public:
    explicit RegisterWidget(QWidget *parent = nullptr);

signals:
    void backToLogin();

private:
    QLineEdit *txtUser;
    QLineEdit *txtPass;
    QLineEdit *txtConfirm;
};

#endif // REGISTERWIDGET_H