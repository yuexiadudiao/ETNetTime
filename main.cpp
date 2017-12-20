#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::addLibraryPath(".");//在当前目录寻找dll，不过还是无法解决
    MainWindow w;
    w.show();

    return a.exec();
}
