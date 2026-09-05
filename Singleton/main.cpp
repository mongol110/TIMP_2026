#include <QCoreApplication>
#include <QDebug>
#include "singleton_classic.h"
#include "singleton_safe.h"

SingletonClassic* SingletonClassic::p_instance = nullptr;
SingletonSafe* SingletonSafe::p_instance = nullptr;
SingletonDestroyer SingletonSafe::destroyer;
SingletonDestroyer::~SingletonDestroyer() { delete p_instance; }

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "Classic:" << SingletonClassic::getInstance();
    qDebug() << "Safe:" << SingletonSafe::getInstance();
    qDebug() << "Modern:" << &SingletonModern::getInstance();
    // DataBase и TcpClient в проекте используют Meyers-синглтон.
    return 0;
}
