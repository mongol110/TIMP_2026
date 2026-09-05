#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QString>

class QTabWidget;
class QTextEdit;
class QLabel;
class QLineEdit;
class QDoubleSpinBox;
class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onConnectClicked();
    void onConnected();
    void onDisconnected();
    void onLine(const QString& line);
    void onSocketError();

    void onRegister();
    void onLogin();
    void onAesEncrypt();
    void onAesDecrypt();
    void onSha1();
    void onNewton();
    void onAudioEmbed();
    void onAudioExtract();

private:
    QWidget* buildAuthTab();
    QWidget* buildAesTab();
    QWidget* buildHashTab();
    QWidget* buildNewtonTab();
    QWidget* buildAudioTab();
    void buildUi();
    void sendCommand(const QString& cmd, const QString& logHint = {});
    void appendLog(const QString& msg);
    void pushTableRow(const QString& cmd, const QString& resp);
    void setLoggedIn(const QString& user);

    QByteArray m_greeting;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_authFlag = nullptr;
    QTextEdit* m_log = nullptr;
    QTableWidget* m_table = nullptr;

    QLineEdit* m_login = nullptr;
    QLineEdit* m_pass = nullptr;
    QLabel* m_authState = nullptr;

    QLineEdit* m_aesKey = nullptr;
    QLineEdit* m_aesInput = nullptr;
    QLineEdit* m_aesOutput = nullptr;

    QLineEdit* m_shaInput = nullptr;
    QTextEdit* m_shaOutput = nullptr;

    QDoubleSpinBox* m_newtonX0 = nullptr;
    QLineEdit* m_newtonEps = nullptr; // QLineEdit: спинбокс режет ввод вида 4.4e-07
    QLineEdit* m_newtonOut = nullptr;

    QLineEdit* m_audioSrc = nullptr;
    QLineEdit* m_audioDst = nullptr;
    QLineEdit* m_audioMsg = nullptr;
    QLineEdit* m_audioExt = nullptr;
    QTextEdit* m_audioOut = nullptr;

    QString m_pendingCmd;
    QString m_user;
};

#endif
