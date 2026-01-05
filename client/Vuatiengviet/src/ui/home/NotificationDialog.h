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
    int getNotificationCount() const { return listNoti ? listNoti->count() : 0; };
signals:
    // --- THÊM TÍN HIỆU NÀY ---
    void notificationCountChanged(int count);
private:
    QListWidget *listNoti;
    void setupUi();
    QPoint m_dragPosition;
    void checkCount();

protected:
    // Thêm 3 hàm này để vẽ nền và kéo thả cửa sổ
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // NOTIFICATIONDIALOG_H