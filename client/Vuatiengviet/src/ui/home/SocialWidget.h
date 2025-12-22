#ifndef SOCIALWIDGET_H
#define SOCIALWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "../../network/GameClient.h" // Include để nhận diện struct UserSearchResult
#include <QApplication>


class SocialWidget : public QWidget {
    Q_OBJECT
public:
    explicit SocialWidget(QWidget *parent = nullptr);

private slots:
    void onSearchFriend(); // Xử lý tìm kiếm
    void onSearchResultReceived(const QList<UserSearchResult>& results); // Nhận kết quả từ Client

    

private:
    void setupUi();
    
    // --- CÁC HÀM BẠN ĐANG THIẾU TRONG FILE HEADER ---
    void setupTabFriends(QWidget *tab);
    void setupTabRecent(QWidget *tab);
    void reloadFriendList(); 
    void addSearchResultItem(const UserSearchResult& user);
    void addPlayerItem(QListWidget *list, const QString& name, bool isFriend, const QString& status);
    // ------------------------------------------------

    QLineEdit *txtSearch;
    QPushButton *btnSearch;
    
    QListWidget *listFriends; // Danh sách bạn bè
    QListWidget *listRecent;  // Danh sách gần đây
    void onFriendListUpdated();
    
};

#endif // SOCIALWIDGET_H