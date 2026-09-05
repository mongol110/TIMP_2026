#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QMap>
#include <QList>

// Multi-client TCP-сервер: отдельный буфер и auth-состояние на каждый сокет.
class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject* parent = nullptr);
    ~MyTcpServer() override;

public slots:
    void slotNewConnection();
    void slotServerRead();
    void slotClientDisconnected();

private:
    QTcpServer* mTcpServer;
    QList<QTcpSocket*> mClients;
    QMap<QTcpSocket*, QByteArray> mBuffers;
    QMap<QTcpSocket*, QString> mUsers; // login после AUTH, "" = гость
};

#endif
