#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include <QFontDatabase>
#include <QDirIterator> 

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    int idBold = QFontDatabase::addApplicationFont(":/Nunito-Bold.ttf");
    int idRegular = QFontDatabase::addApplicationFont(":/Nunito-Regular.ttf");
    if (idBold != -1) {
        QString family = QFontDatabase::applicationFontFamilies(idBold).at(0);
        qDebug() << "Loaded Bold Font:" << family; // Sẽ in ra "Nunito"
    } else {
        qDebug() << "Lỗi: Không tìm thấy file Nunito-Bold.ttf tại đường dẫn trên!";
    }
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
