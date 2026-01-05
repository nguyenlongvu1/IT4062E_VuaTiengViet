#include "SettingsDialog.h"
#include "../utils/AudioManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include "ChangePasswordDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    // 1. Cấu hình cửa sổ không viền
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground); // Vẫn cần cái này để bo tròn góc
    setFixedSize(450, 520);

    // 2. STYLE SHEET (Chỉ chỉnh nút bấm và chữ, BỎ chỉnh nền dialog ở đây)
    setStyleSheet(
        // Label tiêu đề mục
        "QLabel.SectionTitle { color: #f1c40f; font-size: 16px; font-weight: bold; }"
        
        // Đường kẻ vàng
        "QFrame.Separator { border: none; background-color: #f1c40f; max-height: 2px; }"

        // Label thường
        "QLabel { color: white; font-size: 14px; font-weight: 500; }"

        // Checkbox
        "QCheckBox { color: white; font-size: 14px; spacing: 8px; }"
        "QCheckBox::indicator { width: 22px; height: 22px; border-radius: 4px; border: 2px solid #bdc3c7; background: transparent; }"
        "QCheckBox::indicator:checked { background-color: #f1c40f; border-color: #f1c40f; }"

        // Slider vàng óng
        "QSlider::groove:horizontal { border: 1px solid #3a3a3a; height: 10px; background: #1a1a1a; margin: 2px 0; border-radius: 5px; }"
        "QSlider::handle:horizontal { background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 #fff, stop:1 #f1c40f); "
        "border: 1px solid #f1c40f; width: 20px; height: 20px; margin: -6px 0; border-radius: 10px; }"
        "QSlider::sub-page:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e67e22, stop:1 #f1c40f); border-radius: 5px; }"

        // Nút bấm Gradient
        "QPushButton { border-radius: 8px; padding: 10px; font-size: 14px; font-weight: bold; color: white; border: 1px solid rgba(255,255,255,0.2); }"
        "QPushButton#btnChangePass { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f39c12, stop:1 #d35400); }"
        "QPushButton#btnSave { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60); }"
        "QPushButton#btnClose { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); }"
    );
    
    setupUi();
}

// --- [QUAN TRỌNG] HÀM VẼ NỀN ĐỂ KHÔNG BỊ TRONG SUỐT ---
void SettingsDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // Khử răng cưa cho đường bo

    // 1. Tạo màu Gradient xanh than (Giống ảnh mẫu)
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor("#0f2027")); // Xanh đen đậm
    gradient.setColorAt(0.5, QColor("#203a43"));
    gradient.setColorAt(1.0, QColor("#2c5364")); // Xanh xám

    // 2. Thiết lập bút vẽ
    painter.setBrush(gradient); // Đổ màu nền
    painter.setPen(QPen(QColor("#f1c40f"), 3)); // Viền vàng dày 3px

    // 3. Vẽ hình chữ nhật bo tròn
    // rect().adjusted(...) để viền không bị cắt mất một nửa
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 15, 15);
}

void SettingsDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(15);

    // Tiêu đề
    QLabel *lblTitle = new QLabel("Cài Đặt", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("background: transparent; font-size: 26px; font-weight: 900; color: white; text-transform: uppercase; margin-bottom: 5px;");
    
    QGraphicsDropShadowEffect *glowTitle = new QGraphicsDropShadowEffect(this);
    glowTitle->setBlurRadius(25);
    glowTitle->setColor(QColor(100, 200, 255));
    lblTitle->setGraphicsEffect(glowTitle);
    mainLayout->addWidget(lblTitle);

    // --- ÂM THANH ---
    QLabel *lblAudioTitle = new QLabel("Âm Thanh", this);
    lblAudioTitle->setStyleSheet("background: transparent;");
    lblAudioTitle->setProperty("class", "SectionTitle");
    QFrame *lineAudio = new QFrame(this);
    lineAudio->setFrameShape(QFrame::HLine);
    lineAudio->setProperty("class", "Separator");

    mainLayout->addWidget(lblAudioTitle);
    mainLayout->addWidget(lineAudio);

    QGridLayout *audioGrid = new QGridLayout();
    audioGrid->setSpacing(15);
    
    QLabel *lblMusic = new QLabel("Nhạc nền", this);
    lblMusic->setStyleSheet("background: transparent;");
    sliderMusic = new QSlider(Qt::Horizontal, this);
    sliderMusic->setRange(0, 100);
    sliderMusic->setValue(AudioManager::instance().getVolume());
    
    QLabel *lblSFX = new QLabel("Hiệu ứng (SFX)", this);
    lblSFX->setStyleSheet("background: transparent;");
    sliderSFX = new QSlider(Qt::Horizontal, this);
    sliderSFX->setRange(0, 100);
    sliderSFX->setValue(AudioManager::instance().getSFXVolume());

    audioGrid->addWidget(lblMusic, 0, 0);
    audioGrid->addWidget(sliderMusic, 0, 1);
    audioGrid->addWidget(lblSFX, 1, 0);
    audioGrid->addWidget(sliderSFX, 1, 1);
    audioGrid->setColumnStretch(1, 1);
    mainLayout->addLayout(audioGrid);

    // --- HỆ THỐNG ---
    QLabel *lblSysTitle = new QLabel("Hệ Thống", this);
    lblSysTitle->setStyleSheet("background: transparent;");
    lblSysTitle->setProperty("class", "SectionTitle");
    QFrame *lineSys = new QFrame(this);
    lineSys->setFrameShape(QFrame::HLine);
    lineSys->setProperty("class", "Separator");

    mainLayout->addWidget(lblSysTitle);
    mainLayout->addWidget(lineSys);

    QHBoxLayout *chkLayout = new QHBoxLayout();
    chkFullScreen = new QCheckBox("Chế độ toàn màn hình", this);
    chkFullScreen->setChecked(true);
    chkLayout->addStretch();
    chkLayout->addWidget(chkFullScreen);
    mainLayout->addLayout(chkLayout);

    QPushButton *btnChangePass = new QPushButton("Đổi Mật Khẩu", this);
    btnChangePass->setObjectName("btnChangePass");
    btnChangePass->setFixedHeight(40);
    mainLayout->addWidget(btnChangePass);

    // Version Text
    QLabel *lblFooter = new QLabel("Vua Tiếng Việt - Version 1.0.0\nDev by: Long Vu & Tu Phan", this);
    lblFooter->setAlignment(Qt::AlignCenter);
    lblFooter->setStyleSheet("background: transparent; color: #95a5a6; font-size: 11px; margin-top: 5px; font-style: italic;");
    mainLayout->addWidget(lblFooter);

    // --- NÚT BẤM DƯỚI CÙNG ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Lưu Cài Đặt", this);
    btnSave->setObjectName("btnSave");
    btnSave->setFixedHeight(45);
    
    QPushButton *btnClose = new QPushButton("Đóng", this);
    btnClose->setObjectName("btnClose");
    btnClose->setFixedHeight(45);

    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);

    // LOGIC
    connect(sliderMusic, &QSlider::valueChanged, [](int val){ AudioManager::instance().setVolume(val); });
    connect(sliderSFX, &QSlider::valueChanged, [](int val){ AudioManager::instance().setSFXVolume(val); });
    connect(btnClose, &QPushButton::clicked, this, &SettingsDialog::close);
    connect(btnSave, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(btnChangePass, &QPushButton::clicked, [=](){
        ChangePasswordDialog dialog(this);
        dialog.exec(); // Hiện Dialog dạng Modal (chặn cửa sổ cha)
    });
}

// --- XỬ LÝ KÉO THẢ CỬA SỔ ---
void SettingsDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
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