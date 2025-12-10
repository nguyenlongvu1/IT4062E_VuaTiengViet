#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <stdio.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Load Style với đường dẫn alias ngắn gọn
    QFile file(":/style.qss"); 
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        a.setStyleSheet(stream.readAll());
        file.close();
        printf(">>> STYLE LOADED SUCCESSFULLY <<<\n");
    } else {
        printf(">>> ERROR: CANNOT LOAD STYLE FILE! <<<\n");
    }

    MainWindow w;
    w.show();
    return a.exec();
}
