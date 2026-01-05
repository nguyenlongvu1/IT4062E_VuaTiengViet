#include "ChangePasswordDialog.h"
#include "network/GameClient.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QPainter> // Nhớ include cái này để vẽ

ChangePasswordDialog::ChangePasswordDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Đổi Mật Khẩu");
    setFixedSize(400, 450); // Tăng kích thước chút cho thoáng
    
    // 1. Cấu hình không viền & nền trong suốt
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 2. CSS chỉ dành cho các widget con (Nút, Ô nhập)
    setStyleSheet(
        "QLabel { background: transparent; color: white; font-weight: bold; font-size: 14px; margin-top: 5px; }"
        "QLineEdit { padding: 10px; border-radius: 5px; border: 1px solid #bdc3c7; background: #ecf0f1; color: #2c3e50; font-size: 14px; }"
        "QPushButton { padding: 10px; border-radius: 8px; font-weight: bold; color: white; font-size: 14px; }"
        
        // Nút Xác Nhận (Cam Gradient)
        "QPushButton#btnConfirm { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f39c12, stop:1 #d35400); border: 1px solid #e67e22; }"
        "QPushButton#btnConfirm:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f1c40f, stop:1 #e67e22); }"
        
        // Nút Hủy (Đỏ Gradient)
        "QPushButton#btnCancel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #c0392b); border: 1px solid #c0392b; }"
        "QPushButton#btnCancel:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6b6b, stop:1 #e74c3c); }"
    );

    setupUi();

    // Logic kết nối Server (Giữ nguyên)
    connect(&GameClient::instance(), &GameClient::changePasswordSuccess, this, [=]() {
        QMessageBox::information(this, "Thành công", "Đổi mật khẩu thành công!");
        this->accept();
    });

    connect(&GameClient::instance(), &GameClient::changePasswordFailed, this, [=](QString msg) {
        QMessageBox::warning(this, "Thất bại", msg);
    });
}

// [QUAN TRỌNG] HÀM VẼ NỀN (Khắc phục lỗi xuyên thấu)
void ChangePasswordDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Tạo màu Gradient xanh than (Đồng bộ với SettingsDialog)
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor("#0f2027")); 
    gradient.setColorAt(0.5, QColor("#203a43"));
    gradient.setColorAt(1.0, QColor("#2c5364"));

    // 2. Vẽ nền hình chữ nhật bo tròn
    painter.setBrush(gradient);
    painter.setPen(QPen(QColor("#f1c40f"), 2)); // Viền vàng 2px
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
}

void ChangePasswordDialog::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);

    // Tiêu đề
    QLabel *lblTitle = new QLabel("ĐỔI MẬT KHẨU", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("background: transparent; font-size: 20px; color: #f1c40f; font-weight: 900; margin-bottom: 10px; text-transform: uppercase;");
    layout->addWidget(lblTitle);

    // Form nhập liệu
    layout->addWidget(new QLabel("Mật khẩu hiện tại:", this));
    txtOldPass = new QLineEdit(this);
    txtOldPass->setEchoMode(QLineEdit::Password);
    txtOldPass->setPlaceholderText("Nhập mật khẩu cũ...");
    layout->addWidget(txtOldPass);

    layout->addWidget(new QLabel("Mật khẩu mới:", this));
    txtNewPass = new QLineEdit(this);
    txtNewPass->setEchoMode(QLineEdit::Password);
    txtNewPass->setPlaceholderText("Ít nhất 6 ký tự...");
    layout->addWidget(txtNewPass);

    layout->addWidget(new QLabel("Xác nhận mật khẩu mới:", this));
    txtConfirmPass = new QLineEdit(this);
    txtConfirmPass->setEchoMode(QLineEdit::Password);
    txtConfirmPass->setPlaceholderText("Nhập lại mật khẩu mới...");
    layout->addWidget(txtConfirmPass);

    layout->addStretch(); // Đẩy nút xuống dưới cùng

    // Button Layout
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    QPushButton *btnConfirm = new QPushButton("Xác Nhận", this);
    btnConfirm->setObjectName("btnConfirm");
    btnConfirm->setCursor(Qt::PointingHandCursor);
    btnConfirm->setFixedHeight(45);
    
    QPushButton *btnCancel = new QPushButton("Hủy", this);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setFixedHeight(45);

    btnLayout->addWidget(btnConfirm);
    btnLayout->addWidget(btnCancel);
    layout->addLayout(btnLayout);

    // Kết nối nút
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnConfirm, &QPushButton::clicked, this, &ChangePasswordDialog::onConfirm);
}

// Logic confirm giữ nguyên như cũ
void ChangePasswordDialog::onConfirm() {
    QString oldP = txtOldPass->text();
    QString newP = txtNewPass->text();
    QString confP = txtConfirmPass->text();

    if (oldP.isEmpty() || newP.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đầy đủ thông tin!");
        return;
    }
    if (newP != confP) {
        QMessageBox::warning(this, "Lỗi", "Mật khẩu xác nhận không khớp!");
        return;
    }
    if (newP.length() < 6) {
        QMessageBox::warning(this, "Lỗi", "Mật khẩu mới quá ngắn (tối thiểu 6 ký tự)!");
        return;
    }

    GameClient::instance().sendChangePassword(oldP, newP);
}