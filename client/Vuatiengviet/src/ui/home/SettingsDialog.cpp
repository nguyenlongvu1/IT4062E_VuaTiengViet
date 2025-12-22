#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Cài Đặt");
    setFixedSize(400, 450);
    // Style giao diện tối
    setStyleSheet("QDialog { background-color: #2c3e50; color: white; }"
                  "QGroupBox { font-weight: bold; border: 1px solid #7f8c8d; border-radius: 5px; margin-top: 10px; padding-top: 15px; }"
                  "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; color: #f1c40f; }"
                  "QLabel { font-size: 14px; }"
                  "QPushButton { padding: 8px; border-radius: 5px; font-weight: bold; color: white; }"
                  );
    setupUi();
}

void SettingsDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    // --- NHÓM 1: ÂM THANH ---
    QGroupBox *grpAudio = new QGroupBox("Âm Thanh", this);
    QVBoxLayout *audioLayout = new QVBoxLayout(grpAudio);

    // Nhạc nền
    QLabel *lblMusic = new QLabel("Nhạc nền:", this);
    sliderMusic = new QSlider(Qt::Horizontal, this);
    sliderMusic->setRange(0, 100);
    sliderMusic->setValue(70);

    // Hiệu ứng
    QLabel *lblSFX = new QLabel("Hiệu ứng (SFX):", this);
    sliderSFX = new QSlider(Qt::Horizontal, this);
    sliderSFX->setRange(0, 100);
    sliderSFX->setValue(100);

    audioLayout->addWidget(lblMusic);
    audioLayout->addWidget(sliderMusic);
    audioLayout->addWidget(lblSFX);
    audioLayout->addWidget(sliderSFX);

    // --- NHÓM 2: HỆ THỐNG ---
    QGroupBox *grpSystem = new QGroupBox("Hệ Thống", this);
    QVBoxLayout *sysLayout = new QVBoxLayout(grpSystem);

    chkFullScreen = new QCheckBox("Chế độ toàn màn hình", this);
    chkFullScreen->setStyleSheet("color: white; font-size: 14px;");

    QPushButton *btnChangePass = new QPushButton("Đổi Mật Khẩu", this);
    btnChangePass->setStyleSheet("background-color: #e67e22; border: none;");
    btnChangePass->setCursor(Qt::PointingHandCursor);

    sysLayout->addWidget(chkFullScreen);
    sysLayout->addWidget(btnChangePass);

    // --- NHÓM 3: THÔNG TIN ---
    QLabel *lblVersion = new QLabel("Vua Tiếng Việt - Version 1.0.0\nDev by: You", this);
    lblVersion->setAlignment(Qt::AlignCenter);
    lblVersion->setStyleSheet("color: #95a5a6; font-size: 12px; font-style: italic;");

    // --- NÚT SAVE / CLOSE ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Lưu Cài Đặt", this);
    btnSave->setStyleSheet("background-color: #27ae60;");
    
    QPushButton *btnClose = new QPushButton("Đóng", this);
    btnClose->setStyleSheet("background-color: #c0392b;");

    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnClose);

    // ADD VÀO MAIN LAYOUT
    mainLayout->addWidget(grpAudio);
    mainLayout->addWidget(grpSystem);
    mainLayout->addStretch(); // Đẩy lên trên
    mainLayout->addWidget(lblVersion);
    mainLayout->addLayout(btnLayout);

    // Logic đơn giản
    connect(btnClose, &QPushButton::clicked, this, &SettingsDialog::close);
    connect(btnSave, &QPushButton::clicked, [=](){
        // Lưu config xuống file hoặc biến toàn cục
        QMessageBox::information(this, "Đã Lưu", "Cài đặt đã được lưu thành công!");
        this->close();
    });
    connect(btnChangePass, &QPushButton::clicked, [=](){
        QMessageBox::information(this, "Tính năng", "Hộp thoại đổi mật khẩu sẽ hiện ở đây.");
    });
}