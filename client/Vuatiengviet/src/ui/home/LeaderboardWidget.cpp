#include "LeaderboardWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>

LeaderboardWidget::LeaderboardWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
    updateData(); // Load thử dữ liệu
}

void LeaderboardWidget::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Tiêu đề
    QLabel *lblTitle = new QLabel("🏆 BẢNG XẾP HẠNG", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-weight: bold; font-size: 16px; color: #f1c40f; margin-bottom: 5px;");

    // List hiển thị
    listRank = new QListWidget(this);
    listRank->setFocusPolicy(Qt::NoFocus); // Bỏ viền khi click
    listRank->setStyleSheet(
        "QListWidget { background-color: rgba(0,0,0,0.3); border-radius: 10px; border: 1px solid rgba(255,255,255,0.1); }"
        "QListWidget::item { color: white; padding: 10px; border-bottom: 1px solid rgba(255,255,255,0.05); }"
    );

    layout->addWidget(lblTitle);
    layout->addWidget(listRank);
}

void LeaderboardWidget::updateData() {
    listRank->clear();
    // Dữ liệu giả lập (Sau này lấy từ Server)
    struct RankItem { QString name; int score; QString rank; };
    QList<RankItem> data = {
        {"VuaTiengViet_VIP", 2500, "Đế Vương"},
        {"NguyenVanA", 1800, "Bậc Thầy"},
        {"TranThiB", 1500, "Thánh Chém"},
        {"LeVanC", 1200, "Thủ Khoa"},
        {"MeoMeo", 800, "Đủ Đậu"}
    };

    for(int i=0; i<data.size(); i++) {
        QString text = QString("#%1  %2\n      %3 (%4 điểm)")
                       .arg(i+1)
                       .arg(data[i].name)
                       .arg(data[i].rank)
                       .arg(data[i].score);
        QListWidgetItem *item = new QListWidgetItem(text);
        
        // Tô màu top 3
        if (i == 0) item->setForeground(QColor("#f1c40f")); // Vàng
        else if (i == 1) item->setForeground(QColor("#bdc3c7")); // Bạc
        else if (i == 2) item->setForeground(QColor("#e67e22")); // Đồng
        
        listRank->addItem(item);
    }
}