#ifndef GAMEBUTTON_H
#define GAMEBUTTON_H

#include <QPushButton>
#include <QWidget>

class GameButton : public QPushButton {
    Q_OBJECT
public:
    // Khai báo 2 constructor phổ biến để dùng giống hệt QPushButton
    explicit GameButton(QWidget *parent = nullptr);
    explicit GameButton(const QString &text, QWidget *parent = nullptr);

private:
    void initButton(); // Hàm thiết lập chung
};

#endif // GAMEBUTTON_H