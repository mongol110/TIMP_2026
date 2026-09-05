#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("Demo.db");
    if (!db.open()) { qDebug() << "open failed"; return 1; }
    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS demo(id INTEGER PRIMARY KEY, v TEXT)");
    q.exec("INSERT INTO demo(v) VALUES('hello')");
    q.exec("SELECT v FROM demo");
    while (q.next()) qDebug() << q.value(0).toString();
    return 0;
}
