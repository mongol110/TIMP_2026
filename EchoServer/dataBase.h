#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <cstdint>

// Singleton-хранилище: пользователи (авторизация/регистрация) + лог команд.
class DataBase
{
public:
    static DataBase& instance() {
        static DataBase db;
        return db;
    }

    bool open(const QString& path = "AudioServer.db") {
        if (m_opened) return true;
        m_db = QSqlDatabase::addDatabase("QSQLITE", "audio_server_connection");
        m_db.setDatabaseName(path);
        if (!m_db.open()) {
            qDebug() << "DataBase::open failed:" << m_db.lastError().text();
            return false;
        }
        QSqlQuery q(m_db);
        q.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "login TEXT UNIQUE NOT NULL, "
               "pass_hash TEXT NOT NULL, "
               "created DATETIME DEFAULT CURRENT_TIMESTAMP)");
        q.exec("CREATE TABLE IF NOT EXISTS logs ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user TEXT, "
               "command TEXT, "
               "result TEXT, "
               "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)");
        m_opened = true;
        return true;
    }

    static QString hashPassword(const QString& login, const QString& pass) {
        // Ручной SHA1(login:pass), без QCryptographicHash.
        QByteArray data = (login + ":" + pass).toUtf8();
        uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
                 h3 = 0x10325476, h4 = 0xC3D2E1F0;
        QByteArray msg = data;
        uint64_t bitLen = (uint64_t)msg.size() * 8;
        msg.append(char(0x80));
        while (msg.size() % 64 != 56) msg.append(char(0x00));
        for (int i = 7; i >= 0; --i) msg.append(char((bitLen >> (i * 8)) & 0xFF));
        for (int off = 0; off < msg.size(); off += 64) {
            uint32_t w[80];
            const unsigned char* p = reinterpret_cast<const unsigned char*>(msg.constData() + off);
            for (int i = 0; i < 16; ++i)
                w[i] = ((uint32_t)p[i*4] << 24) | ((uint32_t)p[i*4+1] << 16) |
                       ((uint32_t)p[i*4+2] << 8) | (uint32_t)p[i*4+3];
            for (int i = 16; i < 80; ++i) {
                uint32_t v = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
                w[i] = (v << 1) | (v >> 31);
            }
            uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
            for (int i = 0; i < 80; ++i) {
                uint32_t f, k;
                if (i < 20)      { f = (b & c) | ((~b) & d); k = 0x5A827999; }
                else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
                else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
                else             { f = b ^ c ^ d; k = 0xCA62C1D6; }
                uint32_t tmp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
                e = d; d = c; c = ((b << 30) | (b >> 2)); b = a; a = tmp;
            }
            h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
        }
        QByteArray out;
        for (uint32_t h : {h0, h1, h2, h3, h4})
            for (int i = 3; i >= 0; --i) out.append(char((h >> (i * 8)) & 0xFF));
        return QString::fromLatin1(out.toHex());
    }

    QString registerUser(const QString& login, const QString& pass) {
        if (login.isEmpty() || pass.isEmpty())
            return "error: empty login or password";
        if (login.contains('|') || pass.contains('|') || login.contains('\n'))
            return "error: forbidden characters";
        QSqlQuery q(m_db);
        q.prepare("SELECT id FROM users WHERE login=:l");
        q.bindValue(":l", login);
        q.exec();
        if (q.next()) return "error: user already exists";
        q.prepare("INSERT INTO users(login, pass_hash) VALUES(:l, :h)");
        q.bindValue(":l", login);
        q.bindValue(":h", hashPassword(login, pass));
        if (!q.exec()) return "error: db insert failed";
        return "ok: registered";
    }

    QString authUser(const QString& login, const QString& pass) {
        QSqlQuery q(m_db);
        q.prepare("SELECT pass_hash FROM users WHERE login=:l");
        q.bindValue(":l", login);
        q.exec();
        if (!q.next()) return "error: invalid credentials";
        if (q.value(0).toString() != hashPassword(login, pass))
            return "error: invalid credentials";
        return "ok: authenticated";
    }

    void log(const QString& user, const QString& command, const QString& result) const {
        if (!m_opened) return;
        // Не логируем пароли целиком: режем REGISTER/AUTH.
        QString cmd = command;
        if (cmd.startsWith("REGISTER|") || cmd.startsWith("AUTH|")) {
            QStringList p = cmd.split('|');
            if (p.size() >= 3) cmd = p[0] + "|" + p[1] + "|***";
        }
        // Не раздуваем БД base64-аудио.
        if (cmd.size() > 200) cmd = cmd.left(80) + "...[" + QString::number(cmd.size()) + " chars]";
        QString res = result;
        if (res.size() > 300) res = res.left(80) + "...[" + QString::number(res.size()) + " chars]";
        QSqlQuery q(m_db);
        q.prepare("INSERT INTO logs(user, command, result) VALUES(:u, :cmd, :res)");
        q.bindValue(":u", user);
        q.bindValue(":cmd", cmd);
        q.bindValue(":res", res);
        if (!q.exec())
            qDebug() << "DataBase::log error:" << q.lastError().text();
    }

private:
    DataBase() : m_opened(false) {}
    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    QSqlDatabase m_db;
    bool m_opened;
};

#endif
