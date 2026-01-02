#include "SettingsDialog.h"
#include "../utils/AudioManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <../../utils/GameButton.h>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QApplication>

// --- PHẦN KHỞI TẠO ---
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    // 1. Cấu hình cửa sổ: Bỏ viền Windows, nền trong suốt để bo góc
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(450, 520); // Kích thước giống ảnh

    // 2. STYLE SHEET "HARDCORE" (Gradient, Glow, Gold Lines)
    setStyleSheet(
        // Nền chính: Gradient xanh than đậm (Dark Blue)
        "QDialog#SettingsDialog { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0f2027, stop:0.5 #203a43, stop:1 #2c5364);"
        "   border: 2px solid #f1c40f;" // Viền vàng bao quanh cửa sổ
        "   border-radius: 15px;"
        "}"
        
        // Label tiêu đề mục (Âm thanh, Hệ thống): Màu vàng gold
        "QLabel.SectionTitle { color: #f1c40f; font-size: 16px; font-weight: bold; }"
        
        // Đường kẻ ngang (Line): Màu vàng
        "QFrame.Separator { border: none; background-color: #f1c40f; max-height: 2px; }"

        // Label thường: Màu trắng
        "QLabel { color: white; font-size: 14px; font-weight: 500; background: transparent; }"

        // Checkbox: Hiện đại
        "QCheckBox { color: white; font-size: 14px; spacing: 8px; }"
        "QCheckBox::indicator { width: 22px; height: 22px; border-radius: 4px; border: 2px solid #bdc3c7; background: transparent; }"
        "QCheckBox::indicator:checked { background-color: #f1c40f; border-color: #f1c40f; image: url(:/images/check_icon.png); }"

        // --- SLIDER (Thanh trượt vàng óng) ---
        "QSlider::groove:horizontal { border: 1px solid #3a3a3a; height: 10px; background: #1a1a1a; margin: 2px 0; border-radius: 5px; }"
        "QSlider::handle:horizontal { background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 #fff, stop:1 #f1c40f); "
        "border: 1px solid #f1c40f; width: 20px; height: 20px; margin: -6px 0; border-radius: 10px; }"
        "QSlider::sub-page:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e67e22, stop:1 #f1c40f); border-radius: 5px; }"

        // --- BUTTONS (Bóng bẩy, 3D) ---
        "GameButton { border-radius: 8px; padding: 10px; font-size: 14px; font-weight: bold; color: white; border: 1px solid rgba(255,255,255,0.2); }"
        
        // Nút Đổi Mật Khẩu (Vàng Cam - Gradient giống ảnh)
        "GameButton#btnChangePass { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f39c12, stop:1 #d35400); }"
        "GameButton#btnChangePass:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f1c40f, stop:1 #e67e22); }"

        // Nút Lưu (Xanh lá - Gradient)
        "GameButton#btnSave { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60); }"
        "GameButton#btnSave:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #58d68d, stop:1 #2ecc71); }"

        // Nút Đóng (Đỏ Cam - Gradient)
        "GameButton#btnClose { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); }"
        "GameButton#btnClose:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6b6b, stop:1 #e74c3c); }"
    );
    
    // Đặt tên Object để CSS ăn đúng chỗ
    setObjectName("SettingsDialog");
    
    setupUi();
}

// --- HÀM DỰNG GIAO DIỆN ---
void SettingsDialog::setupUi() {
    // Layout chính bao quanh toàn bộ
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 30); // Căn lề rộng như ảnh
    mainLayout->setSpacing(15);

    // 1. TIÊU ĐỀ "CÀI ĐẶT" (Phát sáng neon)
    QLabel *lblTitle = new QLabel("Cài Đặt", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-size: 26px; font-weight: 900; color: white; text-transform: uppercase; margin-bottom: 5px;");
    
    // Hiệu ứng Glow xanh trắng cho tiêu đề
    QGraphicsDropShadowEffect *glowTitle = new QGraphicsDropShadowEffect(this);
    glowTitle->setBlurRadius(25);
    glowTitle->setColor(QColor(100, 200, 255)); // Màu xanh neon
    glowTitle->setOffset(0, 0);
    lblTitle->setGraphicsEffect(glowTitle);

    mainLayout->addWidget(lblTitle);

    // =========================================================
    // SECTION 1: ÂM THANH (Dùng Label + Line vàng thay cho GroupBox)
    // =========================================================
    QLabel *lblAudioTitle = new QLabel("Âm Thanh", this);
    lblAudioTitle->setProperty("class", "SectionTitle"); // Để ăn CSS màu vàng
    
    QFrame *lineAudio = new QFrame(this);
    lineAudio->setFrameShape(QFrame::HLine);
    lineAudio->setProperty("class", "Separator"); // Để ăn CSS đường kẻ vàng

    mainLayout->addWidget(lblAudioTitle);
    mainLayout->addWidget(lineAudio);
    mainLayout->addSpacing(5);

    // Grid Layout cho phần chỉnh âm thanh (Label trái - Slider phải)
    QGridLayout *audioGrid = new QGridLayout();
    audioGrid->setSpacing(15);

    // -- Nhạc nền
    QLabel *lblMusic = new QLabel("Nhạc nền", this);
    sliderMusic = new QSlider(Qt::Horizontal, this);
    sliderMusic->setRange(0, 100);
    sliderMusic->setValue(AudioManager::instance().getVolume());
    
    audioGrid->addWidget(lblMusic, 0, 0);
    audioGrid->addWidget(sliderMusic, 0, 1);

    // -- Hiệu ứng
    QLabel *lblSFX = new QLabel("Hiệu ứng (SFX)", this);
    sliderSFX = new QSlider(Qt::Horizontal, this);
    sliderSFX->setRange(0, 100);
    sliderSFX->setValue(AudioManager::instance().getSFXVolume());

    audioGrid->addWidget(lblSFX, 1, 0);
    audioGrid->addWidget(sliderSFX, 1, 1);
    
    // Set tỷ lệ cột: Label chiếm ít, Slider chiếm nhiều
    audioGrid->setColumnStretch(0, 1);
    audioGrid->setColumnStretch(1, 3);

    mainLayout->addLayout(audioGrid);
    mainLayout->addSpacing(10);

    // =========================================================
    // SECTION 2: HỆ THỐNG
    // =========================================================
    QLabel *lblSysTitle = new QLabel("Hệ Thống", this);
    lblSysTitle->setProperty("class", "SectionTitle");
    
    QFrame *lineSys = new QFrame(this);
    lineSys->setFrameShape(QFrame::HLine);
    lineSys->setProperty("class", "Separator");

    mainLayout->addWidget(lblSysTitle);
    mainLayout->addWidget(lineSys);
    mainLayout->addSpacing(5);

    
    
    // Layout ngang để đẩy Checkbox sang phải (giống ảnh)
    QHBoxLayout *chkLayout = new QHBoxLayout();
    chkLayout->addStretch();
    mainLayout->addLayout(chkLayout);

    // -- Nút Đổi Mật Khẩu (To, Dài)
    GameButton *btnChangePass = new GameButton("Đổi Mật Khẩu", this);
    btnChangePass->setObjectName("btnChangePass");
    btnChangePass->setFixedHeight(45);
    btnChangePass->setCursor(Qt::PointingHandCursor);
    
    // Hiệu ứng Glow vàng cho nút này
    QGraphicsDropShadowEffect *glowPass = new QGraphicsDropShadowEffect(this);
    glowPass->setBlurRadius(15);
    glowPass->setColor(QColor(243, 156, 18, 150));
    glowPass->setOffset(0, 0);
    btnChangePass->setGraphicsEffect(glowPass);

    mainLayout->addWidget(btnChangePass);

    // -- Footer Version
    QLabel *lblFooter = new QLabel("Vua Tiếng Việt - Version 1.0.0\nDev by: Long Vu & Tu Phan", this);
    lblFooter->setAlignment(Qt::AlignCenter);
    lblFooter->setStyleSheet("color: #95a5a6; font-size: 11px; margin-top: 5px; font-style: italic;");
    mainLayout->addWidget(lblFooter);

    // =========================================================
    // SECTION 3: BOTTOM ACTIONS
    // =========================================================
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);

    GameButton *btnSave = new GameButton("Lưu Cài Đặt", this);
    btnSave->setObjectName("btnSave");
    btnSave->setFixedHeight(50);
    btnSave->setCursor(Qt::PointingHandCursor);

    GameButton *btnClose = new GameButton("Đóng", this);
    btnClose->setObjectName("btnClose");
    btnClose->setFixedHeight(50);
    btnClose->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnClose);

    mainLayout->addLayout(btnLayout);

    // --- KẾT NỐI TÍN HIỆU ---
    // 1. Logic Slider
    connect(sliderMusic, &QSlider::valueChanged, [](int val){
        AudioManager::instance().setVolume(val);
    });
    connect(sliderSFX, &QSlider::valueChanged, [](int val){
        AudioManager::instance().setSFXVolume(val);
    });

    // 2. Nút Đóng
    connect(btnClose, &GameButton::clicked, this, &SettingsDialog::close);
    
    // 3. Nút Lưu
    connect(btnSave, &GameButton::clicked, [=](){
        // Logic lưu file config ở đây
        this->accept();
    });
}

// --- XỬ LÝ KÉO THẢ CỬA SỔ (VÌ ĐÃ BỎ TITLE BAR) ---
// Bạn cần khai báo 2 biến này trong file .h:
// QPoint m_dragPosition;
// bool m_isDragging;

void SettingsDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Chỉ cho phép kéo khi bấm vào vùng trống (không bấm vào Slider/Button)
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void SettingsDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}