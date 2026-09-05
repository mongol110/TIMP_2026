#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>
#include <QCryptographicHash>

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
        // Демо-хэш пароля на SHA1 (по теме проекта).
        return QString::fromLatin1(
            QCryptographicHash::hash((login + ":" + pass).toUtf8(),
                                     QCryptographicHash::Sha1).toHex());
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
