#include "GameButton.h"
#include <QSoundEffect> // Trỏ đúng đường dẫn tới AudioManager của bạn
#include "AudioManager.h"

GameButton::GameButton(QWidget *parent) : QPushButton(parent) {
    initButton();
}

GameButton::GameButton(const QString &text, QWidget *parent) : QPushButton(text, parent) {
    initButton();
}

void GameButton::initButton() {
    // 1. Thêm style mặc định (Tùy chọn)
    // Giúp con chuột biến thành bàn tay khi trỏ vào nút
    this->setCursor(Qt::PointingHandCursor);

    // 2. KẾT NỐI TỰ ĐỘNG
    // Mỗi khi nút bị nhấn (pressed), tự động gọi hàm phát tiếng
    // Dùng 'pressed' sẽ nhanh hơn 'clicked' (nghe tiếng ngay khi vừa chạm chuột)
    connect(this, &QPushButton::pressed, this, [](){
        AudioManager::instance().playClickSound();
    });
}