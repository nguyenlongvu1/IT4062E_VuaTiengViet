#include "ResultWidget.h"
#include "../../utils/GameButton.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ResultWidget::ResultWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void ResultWidget::setupUi() {
    this->setObjectName("ResultWidget");
    this->setStyleSheet("#ResultWidget { background-color: #0f0e17; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 40, 50, 40);
    mainLayout->setSpacing(30);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    // Title
    lblTitle = new QLabel("🏆 KẾT QUẢ TRẬN ĐẤU 🏆", this);
    lblTitle->setStyleSheet(
        "QLabel { "
        "   color: #f1c40f; "
        "   font-size: 36px; "
        "   font-weight: bold; "
        "   padding: 20px; "
        "}"
    );
    lblTitle->setAlignment(Qt::AlignCenter);
    
    // Winner announcement
    lblWinner = new QLabel("", this);
    lblWinner->setStyleSheet(
        "QLabel { "
        "   color: #2ecc71; "
        "   font-size: 28px; "
        "   font-weight: bold; "
        "   padding: 15px; "
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "       stop:0 rgba(46, 204, 113, 0.2), "
        "       stop:1 rgba(52, 152, 219, 0.2)); "
        "   border-radius: 10px; "
        "}"
    );
    lblWinner->setAlignment(Qt::AlignCenter);
    
    // Rankings container
    QWidget *rankingsWidget = new QWidget(this);
    rankingsWidget->setStyleSheet(
        "QWidget { "
        "   background-color: #16213e; "
        "   border-radius: 15px; "
        "   padding: 20px; "
        "}"
    );
    
    QVBoxLayout *rankingsLayout = new QVBoxLayout(rankingsWidget);
    rankingsLayout->setSpacing(15);
    
    lblRank1 = new QLabel("", this);
    lblRank1->setStyleSheet(
        "QLabel { "
        "   color: #f1c40f; "
        "   font-size: 24px; "
        "   font-weight: bold; "
        "   padding: 10px; "
        "}"
    );
    
    lblRank2 = new QLabel("", this);
    lblRank2->setStyleSheet(
        "QLabel { "
        "   color: #c0c0c0; "
        "   font-size: 22px; "
        "   padding: 10px; "
        "}"
    );
    
    lblRank3 = new QLabel("", this);
    lblRank3->setStyleSheet(
        "QLabel { "
        "   color: #cd7f32; "
        "   font-size: 20px; "
        "   padding: 10px; "
        "}"
    );
    
    rankingsLayout->addWidget(lblRank1);
    rankingsLayout->addWidget(lblRank2);
    rankingsLayout->addWidget(lblRank3);
    
    // Return button
    btnReturn = new GameButton("VỀ SẢNH", this);
    btnReturn->setObjectName("btnReturn");
    btnReturn->setMinimumHeight(60);
    btnReturn->setMinimumWidth(200);
    btnReturn->setStyleSheet(
        "GameButton#btnReturn { "
        "   background-color: #3498db; "
        "   color: white; "
        "   font-size: 20px; "
        "   font-weight: bold; "
        "   border-radius: 10px; "
        "   padding: 15px 40px; "
        "}"
        "GameButton#btnReturn:hover { background-color: #2980b9; }"
    );
    
    connect(btnReturn, &GameButton::clicked, this, &ResultWidget::returnToLobby);
    
    mainLayout->addWidget(lblTitle);
    mainLayout->addWidget(lblWinner);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(rankingsWidget);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(btnReturn, 0, Qt::AlignCenter);
    mainLayout->addStretch();
}

void ResultWidget::showResults(const QList<QPair<QString, int>> &rankings, const QString &winnerId) {
    if (rankings.isEmpty()) return;
    
    // Show winner
    QString winnerName = rankings[0].first;
    int winnerScore = rankings[0].second;
    lblWinner->setText("🎉 NGƯỜI CHIẾN THẮNG: " + winnerName + " 🎉");
    
    // Show rankings
    if (rankings.size() > 0) {
        lblRank1->setText("🥇 #1: " + rankings[0].first + " - " + QString::number(rankings[0].second) + " điểm");
    }
    
    if (rankings.size() > 1) {
        lblRank2->setText("🥈 #2: " + rankings[1].first + " - " + QString::number(rankings[1].second) + " điểm");
    } else {
        lblRank2->setText("");
    }
    
    if (rankings.size() > 2) {
        lblRank3->setText("🥉 #3: " + rankings[2].first + " - " + QString::number(rankings[2].second) + " điểm");
    } else {
        lblRank3->setText("");
    }
}
