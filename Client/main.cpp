#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("TIMP Audio Client");
    MainWindow w;
    w.show();
    return QApplication::exec();
}
