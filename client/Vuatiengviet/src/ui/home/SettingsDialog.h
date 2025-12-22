#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSlider;
class QCheckBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private:
    void setupUi();
    
    QSlider *sliderMusic;
    QSlider *sliderSFX;
    QCheckBox *chkFullScreen;
};

#endif // SETTINGSDIALOG_H