#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H

#include <QDialog>
#include <QListWidget> // Cần include đầy đủ để dùng trong file .cpp

class NotificationDialog : public QDialog {
    Q_OBJECT
public:
    explicit NotificationDialog(QWidget *parent = nullptr);

    // [MỚI] Hàm thêm thông báo từ bên ngoài vào list
    void addFriendRequest(const QString &senderName);

private:
    QListWidget *listNoti;
    void setupUi();
    // void loadFakeData(); // XÓA BỎ cái này
};

#endif // NOTIFICATIONDIALOG_H