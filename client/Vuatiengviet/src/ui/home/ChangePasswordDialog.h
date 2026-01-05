#pragma once
#include <QDialog>
#include <QLineEdit>

class ChangePasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangePasswordDialog(QWidget *parent = nullptr);

private slots:
    void onConfirm();

private:
    QLineEdit *txtOldPass;
    QLineEdit *txtNewPass;
    QLineEdit *txtConfirmPass;
    void setupUi();
protected:
    void paintEvent(QPaintEvent *event) override;
};