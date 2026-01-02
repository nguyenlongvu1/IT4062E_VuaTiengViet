#ifndef MATCHMAKINGWIDGET_H
#define MATCHMAKINGWIDGET_H

#include <QWidget>
#include <QMovie>
#include <QLabel>

class QLabel;
class QTimer;

class MatchmakingWidget : public QWidget {
    Q_OBJECT
public:
    explicit MatchmakingWidget(QWidget *parent = nullptr);
    
    // Hàm bắt đầu tìm
    void startSearching(); 
    
    // Hàm hủy tìm (được gọi khi bấm nút)
    void cancelSearch(); 

signals:
    // Signal báo cho HomeWidget biết người dùng đã hủy
    void cancelSearchSignal(); 
    
    // Signal báo tìm thấy trận (để đóng dialog)
    void matchFound(QString roomId);

public slots:
    // Slot nhận tin từ mạng
    void onMatchFoundNetwork(const QString& roomId);
    void updateStatusText(); // Hàm cập nhật hiệu ứng chữ (...)

private:
    QLabel *lblRadar;
    QLabel *lblStatus;
    QTimer *statusTimer;
    int dotCount; 
    bool handledMatchFound = false;
    void setupUi();
    QLabel *m_backgroundLabel;
    QMovie *m_backgroundMovie;
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MATCHMAKINGWIDGET_H