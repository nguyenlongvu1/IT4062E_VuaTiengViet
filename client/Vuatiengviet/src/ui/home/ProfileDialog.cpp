#include "ProfileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>

ProfileDialog::ProfileDialog(const QString &username, int score, const QString &rankName, QWidget *parent) 
    : QDialog(parent) 
{
    setWindowTitle("Hồ sơ người chơi");
    setFixedSize(500, 600);
    setStyleSheet("QDialog { background-color: #2c3e50; color: white; }");
    
    // Gọi hàm setupUi với đầy đủ tham số
    setupUi(username, score, rankName);
}

void ProfileDialog::setupUi(const QString &username, int score, const QString &rankName) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(20);

    // --- PHẦN 1: INFO ---
    QHBoxLayout *infoLayout = new QHBoxLayout();
    
    QLabel *lblAvatar = new QLabel(username.left(1).toUpper());
    lblAvatar->setFixedSize(80, 80);
    lblAvatar->setAlignment(Qt::AlignCenter);
    lblAvatar->setStyleSheet("background-color: #e67e22; border-radius: 40px; font-size: 30px; font-weight: bold; border: 3px solid white;");

    QVBoxLayout *textLayout = new QVBoxLayout();
    QLabel *lblName = new QLabel(username);
    lblName->setStyleSheet("font-size: 24px; font-weight: bold; color: #f1c40f;");
    
    QLabel *lblStats = new QLabel(QString("Điểm Rank: %1\nThắng: 15 | Thua: 5").arg(score));
    lblStats->setStyleSheet("font-size: 14px; color: #ecf0f1;");
    
    textLayout->addWidget(lblName);
    textLayout->addWidget(lblStats);

    infoLayout->addWidget(lblAvatar);
    infoLayout->addLayout(textLayout);
    infoLayout->addStretch();

    // --- PHẦN 2: LỊCH SỬ ĐẤU ---
    QLabel *lblHistoryTitle = new QLabel("Lịch sử đấu gần đây");
    lblHistoryTitle->setStyleSheet("font-size: 16px; font-weight: bold; margin-top: 10px;");

    QTableWidget *table = new QTableWidget(10, 3); // 10 hàng, 3 cột
    table->setHorizontalHeaderLabels({"Thời gian", "Đối thủ", "Kết quả"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setStyleSheet("QTableWidget { background-color: rgba(0,0,0,0.2); border: 1px solid gray; gridline-color: gray; color: white; }"
                         "QHeaderView::section { background-color: #34495e; color: white; padding: 5px; }");

    // Fake data history
    for(int i=0; i<10; i++) {
        table->setItem(i, 0, new QTableWidgetItem("20/12 10:30"));
        table->setItem(i, 1, new QTableWidgetItem(QString("DoiThu_%1").arg(i)));
        
        QTableWidgetItem *resItem = new QTableWidgetItem(i % 2 == 0 ? "Thắng (+25)" : "Thua (-15)");
        resItem->setForeground(i % 2 == 0 ? QColor("#2ecc71") : QColor("#e74c3c"));
        table->setItem(i, 2, resItem);
    }

    layout->addLayout(infoLayout);
    layout->addWidget(lblHistoryTitle);
    layout->addWidget(table);
}