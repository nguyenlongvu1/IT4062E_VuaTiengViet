#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>
class ProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProfileDialog(const QString &username, int score, const QString &rankName, QWidget *parent = nullptr);
private:
    void setupUi(const QString &username, int score, const QString &rankName);
    QPoint m_dragPosition;
    class QTableWidget* m_historyTable;

private slots:
    // Slot để cập nhật dữ liệu từ Server
    void onHistoryReceived(const QString &data);
    void onMatchLogReceived(const QString &data);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};
#endif