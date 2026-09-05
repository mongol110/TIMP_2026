#include <QCoreApplication>
#include "mytcpserver.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    MyTcpServer server;
    return QCoreApplication::exec();
}
