#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H
#include <QWidget>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
class QLineEdit;
class QPushButton;
class QLabel;
class QFrame;
class RegisterWidget : public QWidget {
    Q_OBJECT
public:
    explicit RegisterWidget(QWidget *parent = nullptr);
protected:
    // Hàm này bắt buộc phải có để hiện hình nền trên Widget tự tạo
    void paintEvent(QPaintEvent *) override {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
signals:
    void backToLogin();

private:
 QFrame *loginContainer;
    QLineEdit *txtUser;
    QLineEdit *txtPass;
    QLineEdit *txtConfirm;
};

#endif // REGISTERWIDGET_H