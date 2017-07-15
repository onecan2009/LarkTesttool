#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase base;
    base.addApplicationFont("/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf");

    MainWindow w;
    w.show();

    return a.exec();
}
