#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>
#include <QByteArray>
#include <QObject>

// Singleton TCP-клиент: единственная точка сетевого доступа GUI.
class TcpClient : public QObject
{
    Q_OBJECT
public:
    static TcpClient& instance() {
        static TcpClient inst;
        return inst;
    }

    void connectTo(const QString& host, quint16 port) { m_socket->connectToHost(host, port); }
    void disconnectFrom() { m_socket->disconnectFromHost(); }
    bool isConnected() const { return m_socket->state() == QAbstractSocket::ConnectedState; }
    void sendLine(const QString& line) { m_socket->write((line + "\n").toUtf8()); }
    QTcpSocket* socket() { return m_socket; }

signals:
    void lineReceived(const QString& line);

private:
    explicit TcpClient(QObject* parent = nullptr) : QObject(parent) {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::readyRead, this, [this]{
            m_buffer.append(m_socket->readAll());
            while (m_buffer.contains('\n')) {
                int i = m_buffer.indexOf('\n');
                QString line = QString::fromUtf8(m_buffer.left(i)).trimmed();
                m_buffer.remove(0, i + 1);
                if (!line.isEmpty()) emit lineReceived(line);
            }
        });
    }
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    QTcpSocket* m_socket = nullptr;
    QByteArray m_buffer;
};

#endif
