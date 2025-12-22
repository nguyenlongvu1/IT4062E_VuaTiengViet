#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
class ProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProfileDialog(const QString &username, int score, const QString &rankName, QWidget *parent = nullptr);
private:
    void setupUi(const QString &username, int score, const QString &rankName);
};
#endif