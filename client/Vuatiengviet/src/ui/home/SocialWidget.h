#ifndef SOCIALWIDGET_H
#define SOCIALWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <../../utils/GameButton.h>
#include "../../network/GameClient.h" 
#include <QApplication>

class GameButton;
class SocialWidget : public QWidget {
    Q_OBJECT
public:
    explicit SocialWidget(QWidget *parent = nullptr);

private slots:
    void onSearchFriend(); // Xử lý tìm kiếm
    void onSearchResultReceived(const QList<UserSearchResult>& results); // Nhận kết quả từ Client

    

private:
    void setupUi();
    
    void setupTabFriends(QWidget *tab);
    void setupTabRecent(QWidget *tab);
    void reloadFriendList(); 
    void addSearchResultItem(const UserSearchResult& user);
    void addPlayerItem(QListWidget *list, const QString& name, bool isFriend, const QString& status);
  

    QLineEdit *txtSearch;
    GameButton *btnSearch;
    
    QListWidget *listFriends; // Danh sách bạn bè
    QListWidget *listRecent;  // Danh sách gần đây
    void onFriendListUpdated();
    
    
    // Nút Tab tùy chỉnh
    GameButton *btnTabFriend;
    GameButton *btnTabRecent;
    QStringList m_friendCache;
    
};

#endif // SOCIALWIDGET_H