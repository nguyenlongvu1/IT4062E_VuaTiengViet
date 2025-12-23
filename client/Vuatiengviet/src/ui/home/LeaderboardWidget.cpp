#include "LeaderboardWidget.h"
#include "GameClient.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QTimer>

LeaderboardWidget::LeaderboardWidget(QWidget *parent) : QWidget(parent) {
    setupUi();

    // 1. Kết nối với GameClient để nhận dữ liệu
    connect(&GameClient::instance(), &GameClient::leaderboardReceived, 
            this, &LeaderboardWidget::updateLeaderboard);

    // 2. Gửi yêu cầu lấy dữ liệu ngay khi khởi tạo
    // Dùng QTimer::singleShot để đảm bảo an toàn luồng khi khởi tạo UI
    QTimer::singleShot(500, [=](){
        GameClient::instance().sendGetLeaderboardRequest();
    });
}

void LeaderboardWidget::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    // Thay setFixedWidth bằng setMinimumWidth
    this->setMinimumWidth(350);
    this->setMaximumWidth(600);

    // Tiêu đề
    QLabel *lblTitle = new QLabel("🏆 BẢNG XẾP HẠNG", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-weight: bold; font-size: 16px; color: #f1c40f; margin-bottom: 5px;");

    // List hiển thị
    listRank = new QListWidget(this);
    listRank->setFocusPolicy(Qt::NoFocus); 
    listRank->setStyleSheet(
        "QListWidget { background-color: rgba(0,0,0,0.3); border-radius: 10px; border: 1px solid rgba(255,255,255,0.1); }"
        "QListWidget::item { color: white; padding: 10px; border-bottom: 1px solid rgba(255,255,255,0.05); }"
    );

    layout->addWidget(lblTitle);
    layout->addWidget(listRank);
}

// Hàm này được gọi khi Server trả về dữ liệu
void LeaderboardWidget::updateLeaderboard(const QList<RankItem> &items) {
    listRank->clear();

    for(int i=0; i<items.size(); i++) {
        const RankItem &data = items[i];

        QString text = QString("%1.  %2\n      %3 (%4 điểm)")
                        .arg(i+1)
                        .arg(data.name)
                        .arg(data.rank)
                        .arg(data.score);
        
        QListWidgetItem *item = new QListWidgetItem(text);
        
        // Tô màu top 3
        if (i == 0) {
            item->setForeground(QColor("#f1c40f")); 
            item->setIcon(QIcon(":/rank1.png"));    
        }
        else if (i == 1) item->setForeground(QColor("#bdc3c7")); 
        else if (i == 2) item->setForeground(QColor("#e67e22")); 
        
        // Font chữ
        QFont font = item->font();
        if(i < 3) font.setBold(true);
        item->setFont(font);

        listRank->addItem(item);
    }
}