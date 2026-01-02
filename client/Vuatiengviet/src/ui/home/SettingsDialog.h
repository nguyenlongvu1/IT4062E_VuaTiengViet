#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSlider;
class QCheckBox;
class GameButton;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    void setupUi();
    
    QSlider *sliderMusic;
    QSlider *sliderSFX;
    QCheckBox *chkFullScreen;
    QPoint m_dragPosition;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // SETTINGSDIALOG_H