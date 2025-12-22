#include "SocialWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QTabWidget> 
#include <QDebug>
#include "../../network/GameClient.h"

SocialWidget::SocialWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
    reloadFriendList(); 
}

void SocialWidget::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // 1. THANH TÌM KIẾM
    QWidget *searchContainer = new QWidget();
    searchContainer->setStyleSheet("background-color: #2980b9; padding: 5px;");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchContainer);
    
    txtSearch = new QLineEdit(this);
    txtSearch->setPlaceholderText("Tìm bạn bè...");
    txtSearch->setStyleSheet("QLineEdit { border: none; border-radius: 15px; padding: 5px 10px; background: white; color: #2c3e50; }");
    
    btnSearch = new QPushButton(this);
    btnSearch->setFixedSize(30, 30);
    btnSearch->setIcon(QIcon::fromTheme("edit-find")); 
    btnSearch->setStyleSheet("QPushButton { background: transparent; border: none; }");

    searchLayout->addWidget(txtSearch);
    searchLayout->addWidget(btnSearch);

    // 2. TAB WIDGET (Chỉ giữ 2 Tab: Bạn bè & Gần đây)
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 0; background: rgba(0,0,0,0.1); }"
        "QTabBar::tab { background: #34495e; color: white; padding: 8px 15px; }"
        "QTabBar::tab:selected { background: #2980b9; font-weight: bold; }"
    );

    QWidget *tabFriend = new QWidget();
    setupTabFriends(tabFriend);
    tabWidget->addTab(tabFriend, "Bạn Bè");

    QWidget *tabRecent = new QWidget();
    setupTabRecent(tabRecent);
    tabWidget->addTab(tabRecent, "Gần Đây");

    layout->addWidget(searchContainer); 
    layout->addWidget(tabWidget);

    // 3. KẾT NỐI LOGIC TÌM KIẾM
    connect(btnSearch, &QPushButton::clicked, this, &SocialWidget::onSearchFriend);
    connect(txtSearch, &QLineEdit::textChanged, this, &SocialWidget::onSearchFriend);
    connect(&GameClient::instance(), &GameClient::searchResultReceived, 
            this, &SocialWidget::onSearchResultReceived);
    connect(&GameClient::instance(), &GameClient::friendListUpdated, 
        this, &SocialWidget::onFriendListUpdated);
    // SocialWidget.cpp -> setupUi()

// Kết nối này cực kỳ quan trọng: Khi Server trả về danh sách bạn bè
connect(&GameClient::instance(), &GameClient::friendListReceived, 
        this, [=](const QList<UserSearchResult>& friends){
    
    // Nếu người dùng đang gõ tìm kiếm thì KHÔNG đè danh sách bạn bè lên kết quả tìm kiếm
    if (!txtSearch->text().isEmpty()) return;

    listFriends->clear();
    if (friends.isEmpty()) {
        // Có thể thêm 1 label thông báo "Chưa có bạn bè"
        return;
    }

    for (const auto& user : friends) {
        addSearchResultItem(user); // Vẽ người bạn đó lên màn hình
    }
});
    connect(&GameClient::instance(), &GameClient::friendListReceived, 
        this, [=](const QList<UserSearchResult>& friends){
    listFriends->clear();
    if (friends.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(listFriends);
        QLabel *lbl = new QLabel("Chưa có bạn bè.", listFriends);
        lbl->setStyleSheet("color: #bdc3c7; padding: 10px;");
        lbl->setAlignment(Qt::AlignCenter);
        listFriends->setItemWidget(item, lbl);
        return;
    }
   QStringList addedUsers; 

    for (const auto& f : friends) {
        if (!addedUsers.contains(f.username)) {
            addSearchResultItem(f); 
            addedUsers.append(f.username);
        }
    }
});
}

void SocialWidget::setupTabFriends(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    listFriends = new QListWidget();
    listFriends->setStyleSheet("QListWidget { background: transparent; border: none; }");
    layout->addWidget(listFriends);
}

void SocialWidget::setupTabRecent(QWidget *tab) {
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    listRecent = new QListWidget();
    listRecent->setStyleSheet(listFriends->styleSheet());
    addPlayerItem(listRecent, "DoiThu_X", false, "Vừa chơi");
    layout->addWidget(listRecent);
}


void SocialWidget::onSearchFriend() {
    QString keyword = txtSearch->text().trimmed();
    if (keyword.isEmpty()) {
        // Nếu xóa trắng ô tìm kiếm, hiển thị lại danh sách bạn bè
        reloadFriendList(); 
        return;
    }
    // Nếu có chữ thì mới đi tìm kiếm người lạ
    listFriends->clear();
    GameClient::instance().sendSearchRequest(keyword);
}

void SocialWidget::onSearchResultReceived(const QList<UserSearchResult>& results) {
    if (txtSearch->text().isEmpty()) return;
    listFriends->clear();
    for (const auto& user : results) {
        addSearchResultItem(user);
    }
}

void SocialWidget::addSearchResultItem(const UserSearchResult& user) {
    QListWidgetItem *item = new QListWidgetItem(listFriends);
    item->setSizeHint(QSize(0, 60)); 
    QWidget *wid = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout(wid);

    // 1. Định dạng màu sắc trạng thái
    QString statusColor = (user.status == "Online") ? "#2ecc71" : "#95a5a6";

    // 2. Tạo Label thông tin
    QLabel *lblInfo = new QLabel(wid);
    lblInfo->setText(QString("<b>%1</b> <br> <span style='color:%2; font-size:11px;'>%3</span>")
                     .arg(user.username)
                     .arg(statusColor)
                     .arg(user.status));
    lblInfo->setTextFormat(Qt::RichText);
    lblInfo->setStyleSheet("color: white;");

    hLayout->addWidget(lblInfo);
    hLayout->addStretch();

    // 3. LOGIC BỎ NÚT NHẮN TIN: Chỉ hiện nút nếu CHƯA là bạn bè
    if (!user.isFriend) {
        QPushButton *btn = new QPushButton("+ Kết bạn", wid);
        btn->setFixedSize(80, 28);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet("QPushButton { background-color: #2980b9; color: white; border-radius: 4px; border: none; font-weight: bold; }"
                           "QPushButton:hover { background-color: #3498db; }");

        connect(btn, &QPushButton::clicked, [=](){
            btn->setText("Đã gửi");
            btn->setEnabled(false);
            btn->setStyleSheet("background-color: #7f8c8d; color: #bdc3c7; border-radius: 4px; border: none;");
            GameClient::instance().sendAddFriendRequest(user.username);
        });

        hLayout->addWidget(btn);
    } 
    // Nếu user.isFriend == true, đoạn code trên sẽ bị bỏ qua -> Không có nút nào được thêm vào bên phải

    listFriends->setItemWidget(item, wid);
}

void SocialWidget::addPlayerItem(QListWidget *list, const QString& name, bool isFriend, const QString& status) {
    QListWidgetItem *item = new QListWidgetItem(list);
    item->setSizeHint(QSize(0, 50));
    QWidget *wid = new QWidget();
    QHBoxLayout *hLayout = new QHBoxLayout(wid);
    
    QLabel *lblInfo = new QLabel(QString("<b>%1</b> - %2").arg(name).arg(status), wid);
    lblInfo->setStyleSheet("color: white;");
    
    hLayout->addWidget(lblInfo);
    hLayout->addStretch();

    if (!isFriend) {
        QPushButton *btnAdd = new QPushButton("+", wid);
        btnAdd->setFixedSize(30, 25);
        btnAdd->setStyleSheet("background-color: #2ecc71; color: white; border-radius: 4px;");
        connect(btnAdd, &QPushButton::clicked, [=](){
            btnAdd->setEnabled(false);
            GameClient::instance().sendAddFriendRequest(name);
        });
        hLayout->addWidget(btnAdd);
    }
    list->setItemWidget(item, wid);
}
void SocialWidget::onFriendListUpdated() {
    qDebug() << "[UI] Danh sách bạn bè thay đổi, đang cập nhật...";
    // Thay vì dùng data giả, ta yêu cầu Server gửi danh sách mới nhất
    GameClient::instance().sendGetFriendList(); 
}
void SocialWidget::reloadFriendList() {
    listFriends->clear(); // Xóa sạch list
    // Gửi lệnh lên Server lấy danh sách bạn bè chính thức
    GameClient::instance().sendGetFriendList(); 
}