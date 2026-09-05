#include "mytcpserver.h"
#include "functionsforserver.h"
#include "dataBase.h"
#include <QDebug>
#include <QHostAddress>

static const quint16 kPort = 34944;

MyTcpServer::~MyTcpServer()
{
    for (QTcpSocket* s : mClients) s->close();
    mTcpServer->close();
}

MyTcpServer::MyTcpServer(QObject* parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);
    DataBase::instance().open();

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, kPort)) {
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started on port" << kPort;
    }
}

void MyTcpServer::slotNewConnection()
{
    while (mTcpServer->hasPendingConnections()) {
        QTcpSocket* sock = mTcpServer->nextPendingConnection();
        mClients.append(sock);
        mBuffers[sock].clear();
        mUsers[sock] = QString();

        sock->write(
            "TIMP AudioServer ready. AUTH first.\r\n"
            "Commands: REGISTER|login|pass  AUTH|login|pass  SHA1|data\r\n"
            "          AES_ENCRYPT|key|text  AES_DECRYPT|key|hex  NEWTON|x0|eps\r\n"
            "          AUDIO_EMBED|b64wav|msg  AUDIO_EXTRACT|b64wav\r\n"
        );

        connect(sock, &QTcpSocket::readyRead,
                this, &MyTcpServer::slotServerRead);
        connect(sock, &QTcpSocket::disconnected,
                this, &MyTcpServer::slotClientDisconnected);
    }
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;
    mBuffers[sock].append(sock->readAll());

    while (true) {
        QByteArray& buf = mBuffers[sock];
        int nlIdx  = buf.indexOf('\n');
        int delIdx = buf.indexOf('\x01');
        int idx = -1;
        if (nlIdx != -1 && delIdx != -1) idx = qMin(nlIdx, delIdx);
        else if (nlIdx != -1) idx = nlIdx;
        else if (delIdx != -1) idx = delIdx;
        else break;

        const QString command = QString::fromUtf8(buf.left(idx)).trimmed();
        buf.remove(0, idx + 1);
        if (command.isEmpty()) continue;

        qDebug() << ">>" << command.left(100);
        QString newUser;
        bool authOk = false;
        QString result = parsing(command, mUsers.value(sock), newUser, authOk);
        if (authOk) mUsers[sock] = newUser;

        DataBase::instance().log(mUsers.value(sock), command, result);
        sock->write((result + "\r\n").toUtf8());
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;
    mClients.removeAll(sock);
    mBuffers.remove(sock);
    mUsers.remove(sock);
    sock->deleteLater();
}
