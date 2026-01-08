#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <stdio.h>
#include <QFontDatabase>
#include <QDirIterator> 
#include "utils/AudioManager.h"
#include <QDebug>
#include <QLocale>

int main(int argc, char *argv[]) {
    qputenv("SDL_AUDIODRIVER", "pulseaudio");
    qputenv("PULSE_SERVER", "unix:/mnt/wslg/PulseServer");
    qputenv("PULSE_LATENCY_MSEC", "30");
    // qputenv("PULSE_LATENCY_MSEC", "60");
    QApplication a(argc, argv);
    // Set default locale to Vietnamese to improve IME behavior
    QLocale::setDefault(QLocale(QLocale::Vietnamese, QLocale::Vietnam));
   

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
