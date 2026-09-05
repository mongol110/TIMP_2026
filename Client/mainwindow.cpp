#include "mainwindow.h"
#include "connectiondialog.h"
#include "tcpclient.h"

#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFile>
#include <QFileDialog>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QDateTime>
#include <QTableWidget>
#include <QHeaderView>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("TIMP Audio — AES/SHA1/Newton/WAV");
    setMinimumSize(760, 600);
    buildUi();
    connect(&TcpClient::instance(), &TcpClient::lineReceived,
            this, &MainWindow::onLine);
    connect(TcpClient::instance().socket(), &QTcpSocket::connected,
            this, &MainWindow::onConnected);
    connect(TcpClient::instance().socket(), &QTcpSocket::disconnected,
            this, &MainWindow::onDisconnected);
    connect(TcpClient::instance().socket(), &QAbstractSocket::errorOccurred,
            this, &MainWindow::onSocketError);
}

void MainWindow::buildUi()
{
    auto* menuFile = menuBar()->addMenu("&Файл");
    menuFile->addAction("&Подключиться...", this, &MainWindow::onConnectClicked);
    menuFile->addAction("&Отключиться", this, []{ TcpClient::instance().disconnectFrom(); });
    menuFile->addAction("&Выход", this, &QWidget::close);

    auto* splitter = new QSplitter(Qt::Vertical, this);
    auto* tabs = new QTabWidget(splitter);
    tabs->addTab(buildAuthTab(), "Авторизация");
    tabs->addTab(buildAesTab(), "AES");
    tabs->addTab(buildHashTab(), "SHA1");
    tabs->addTab(buildNewtonTab(), "Ньютон");
    tabs->addTab(buildAudioTab(), "Аудио-стего");

    auto* bottom = new QTabWidget(splitter);
    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setFont(QFont("Courier", 9));
    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"Время", "Команда", "Ответ"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bottom->addTab(m_log, "Лог");
    bottom->addTab(m_table, "Таблица");

    splitter->addWidget(tabs);
    splitter->addWidget(bottom);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

    m_statusLabel = new QLabel("Не подключён");
    statusBar()->addPermanentWidget(m_statusLabel);
    m_authFlag = new QLabel;
    m_authFlag->setFont(QFont("Segoe UI", 10, QFont::Bold));
    statusBar()->addWidget(m_authFlag);
    setLoggedIn(QString());
    auto* btn = new QPushButton("Подключиться");
    statusBar()->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
}

QWidget* MainWindow::buildAuthTab()
{
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    m_login = new QLineEdit(w);
    m_login->setPlaceholderText("login");
    m_pass = new QLineEdit(w);
    m_pass->setPlaceholderText("password");
    m_pass->setEchoMode(QLineEdit::Password);
    auto* bReg = new QPushButton("Зарегистрироваться");
    auto* bAuth = new QPushButton("Войти (AUTH)");
    m_authState = new QLabel("Гость (нужен AUTH для функций)");
    l->addWidget(new QLabel("Логин:")); l->addWidget(m_login);
    l->addWidget(new QLabel("Пароль:")); l->addWidget(m_pass);
    l->addWidget(bReg); l->addWidget(bAuth); l->addWidget(m_authState);
    l->addStretch();
    connect(bReg, &QPushButton::clicked, this, &MainWindow::onRegister);
    connect(bAuth, &QPushButton::clicked, this, &MainWindow::onLogin);
    return w;
}

QWidget* MainWindow::buildAesTab()
{
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    m_aesKey = new QLineEdit("secretkey", w);
    m_aesInput = new QLineEdit(w);
    m_aesInput->setPlaceholderText("текст или hex");
    m_aesOutput = new QLineEdit(w);
    m_aesOutput->setReadOnly(true);
    auto* bE = new QPushButton("Зашифровать");
    auto* bD = new QPushButton("Расшифровать");
    auto* bSwap = new QPushButton("↑ Результат во вход");
    auto* row = new QHBoxLayout; row->addWidget(bE); row->addWidget(bD);
    auto* row2 = new QHBoxLayout; row2->addWidget(bSwap);
    l->addWidget(new QLabel("Ключ (до 16 байт):")); l->addWidget(m_aesKey);
    l->addWidget(new QLabel("Вход:")); l->addWidget(m_aesInput);
    l->addLayout(row);
    l->addLayout(row2);
    l->addWidget(new QLabel("Результат:")); l->addWidget(m_aesOutput);
    l->addStretch();
    connect(bE, &QPushButton::clicked, this, &MainWindow::onAesEncrypt);
    connect(bD, &QPushButton::clicked, this, &MainWindow::onAesDecrypt);
    connect(bSwap, &QPushButton::clicked, this, [this]{ m_aesInput->setText(m_aesOutput->text().trimmed()); });
    return w;
}

QWidget* MainWindow::buildHashTab()
{
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    m_shaInput = new QLineEdit(w);
    m_shaInput->setPlaceholderText("строка для SHA1");
    m_shaOutput = new QTextEdit(w);
    m_shaOutput->setReadOnly(true);
    m_shaOutput->setMaximumHeight(70);
    auto* b = new QPushButton("Вычислить SHA1");
    l->addWidget(new QLabel("Строка:")); l->addWidget(m_shaInput);
    l->addWidget(b);
    l->addWidget(new QLabel("SHA1 (40 hex):")); l->addWidget(m_shaOutput);
    l->addStretch();
    connect(b, &QPushButton::clicked, this, &MainWindow::onSha1);
    return w;
}

QWidget* MainWindow::buildNewtonTab()
{
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    auto* info = new QLabel("<b>f(x)=x^3-x-2=0</b>, корень ~1.5213797<br>Метод Ньютона: x1 = x - f(x)/f'(x)");
    info->setTextFormat(Qt::RichText);
    auto* form = new QFormLayout;
    m_newtonX0 = new QDoubleSpinBox; m_newtonX0->setRange(-1e6, 1e6); m_newtonX0->setValue(1.5);
    m_newtonEps = new QDoubleSpinBox; m_newtonEps->setDecimals(10);
    m_newtonEps->setRange(1e-12, 1.0); m_newtonEps->setValue(1e-9);
    form->addRow("x0:", m_newtonX0);
    form->addRow("eps:", m_newtonEps);
    m_newtonOut = new QLineEdit; m_newtonOut->setReadOnly(true);
    auto* b = new QPushButton("Найти корень");
    l->addWidget(info); l->addLayout(form); l->addWidget(b);
    l->addWidget(new QLabel("Результат:")); l->addWidget(m_newtonOut);
    l->addStretch();
    connect(b, &QPushButton::clicked, this, &MainWindow::onNewton);
    return w;
}

QWidget* MainWindow::buildAudioTab()
{
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    auto* eg = new QGroupBox("Внедрение (AUDIO_EMBED)");
    m_audioSrc = new QLineEdit; m_audioSrc->setPlaceholderText("исходный .wav");
    m_audioDst = new QLineEdit; m_audioDst->setPlaceholderText("куда сохранить .wav");
    m_audioMsg = new QLineEdit; m_audioMsg->setPlaceholderText("секретное сообщение");
    auto* bSrc = new QPushButton("...");
    auto* bDst = new QPushButton("...");
    auto* r1 = new QHBoxLayout; r1->addWidget(m_audioSrc); r1->addWidget(bSrc);
    auto* r2 = new QHBoxLayout; r2->addWidget(m_audioDst); r2->addWidget(bDst);
    auto* bE = new QPushButton("Внедрить в WAV");
    auto* el = new QVBoxLayout(eg);
    el->addWidget(new QLabel("WAV:")); el->addLayout(r1);
    el->addWidget(new QLabel("Сохранить как:")); el->addLayout(r2);
    el->addWidget(new QLabel("Сообщение:")); el->addWidget(m_audioMsg); el->addWidget(bE);

    auto* xg = new QGroupBox("Извлечение (AUDIO_EXTRACT)");
    m_audioExt = new QLineEdit; m_audioExt->setPlaceholderText("wav со сообщением");
    auto* bExt = new QPushButton("...");
    auto* r3 = new QHBoxLayout; r3->addWidget(m_audioExt); r3->addWidget(bExt);
    auto* bX = new QPushButton("Извлечь");
    m_audioOut = new QTextEdit; m_audioOut->setReadOnly(true); m_audioOut->setMaximumHeight(60);
    auto* xl = new QVBoxLayout(xg);
    xl->addLayout(r3); xl->addWidget(bX); xl->addWidget(m_audioOut);

    l->addWidget(eg); l->addWidget(xg); l->addStretch();
    connect(bSrc, &QPushButton::clicked, this, [this]{
        QString f = QFileDialog::getOpenFileName(this, "WAV", "", "WAV (*.wav)");
        if (!f.isEmpty()) m_audioSrc->setText(f); });
    connect(bDst, &QPushButton::clicked, this, [this]{
        QString f = QFileDialog::getSaveFileName(this, "Сохранить", "", "WAV (*.wav)");
        if (!f.isEmpty()) m_audioDst->setText(f); });
    connect(bExt, &QPushButton::clicked, this, [this]{
        QString f = QFileDialog::getOpenFileName(this, "WAV", "", "WAV (*.wav)");
        if (!f.isEmpty()) m_audioExt->setText(f); });
    connect(bE, &QPushButton::clicked, this, &MainWindow::onAudioEmbed);
    connect(bX, &QPushButton::clicked, this, &MainWindow::onAudioExtract);
    return w;
}

void MainWindow::onConnectClicked()
{
    ConnectionDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    appendLog(QString("[%1] Подключение %2:%3...")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(dlg.host()).arg(dlg.port()));
    TcpClient::instance().connectTo(dlg.host(), (quint16)dlg.port());
}

void MainWindow::onConnected()
{
    auto* s = TcpClient::instance().socket();
    m_statusLabel->setText(QString("Подключён %1:%2").arg(s->peerAddress().toString()).arg(s->peerPort()));
    appendLog("[OK] Подключено. Сначала AUTH/REGISTER.");
    setLoggedIn(QString());
    m_authState->setText("Гость (нужен AUTH для функций)");
}

void MainWindow::onDisconnected()
{
    m_statusLabel->setText("Не подключён");
    appendLog("[--] Отключено.");
    setLoggedIn(QString());
}

void MainWindow::onSocketError()
{
    appendLog("[ERR] " + TcpClient::instance().socket()->errorString());
}

void MainWindow::onLine(const QString& line)
{
    QString show = line.size() > 120 ? line.left(60) + "...[" + QString::number(line.size()) + "]" : line;
    appendLog("[<<] " + show);

    // Первый большой приветственный текст сервера игнорируем как таблицу.
    if (!m_pendingCmd.isEmpty()) {
        pushTableRow(m_pendingCmd, show);
        if (m_pendingCmd.startsWith("REGISTER") || m_pendingCmd.startsWith("AUTH")) {
            if (!line.startsWith("error")) {
                if (m_pendingCmd.startsWith("AUTH")) {
                    m_user = m_pendingCmd.split('|').value(1);
                    m_authState->setText("Авторизован: " + m_user);
                    setLoggedIn(m_user);
                } else {
                    m_authState->setText("Зарегистрирован, теперь AUTH.");
                }
            }
            m_audioOut->setPlainText(line);
        }
        else if (m_pendingCmd.startsWith("AES_ENCRYPT") || m_pendingCmd.startsWith("AES_DECRYPT"))
            m_aesOutput->setText(line);
        else if (m_pendingCmd.startsWith("SHA1"))
            m_shaOutput->setPlainText(line);
        else if (m_pendingCmd.startsWith("NEWTON"))
            m_newtonOut->setText(line);
        else if (m_pendingCmd.startsWith("AUDIO_EMBED")) {
            if (line.startsWith("error")) m_audioOut->setPlainText(line);
            else {
                QByteArray wav = QByteArray::fromBase64(line.toLatin1());
                QString dst = m_audioDst->text();
                if (dst.isEmpty()) m_audioOut->setPlainText("Получено, но путь сохранения пуст.");
                else {
                    QFile f(dst);
                    if (f.open(QIODevice::WriteOnly)) { f.write(wav); f.close(); m_audioOut->setPlainText("ok: " + dst); }
                    else m_audioOut->setPlainText("Ошибка сохранения");
                }
            }
        }
        else if (m_pendingCmd.startsWith("AUDIO_EXTRACT"))
            m_audioOut->setPlainText(line);
        m_pendingCmd.clear();
    }
}

void MainWindow::sendCommand(const QString& cmd, const QString& logHint)
{
    if (!TcpClient::instance().isConnected()) {
        QMessageBox::warning(this, "Нет подключения", "Подключитесь к серверу.");
        return;
    }
    m_pendingCmd = cmd;
    appendLog("[>>] " + (logHint.isEmpty() ? (cmd.size() > 150 ? cmd.left(80) + "..." : cmd) : logHint));
    TcpClient::instance().sendLine(cmd);
}

void MainWindow::appendLog(const QString& msg) { m_log->append(msg); }

void MainWindow::pushTableRow(const QString& cmd, const QString& resp)
{
    QString c = cmd.size() > 80 ? cmd.left(80) + "..." : cmd;
    QString r = resp.size() > 80 ? resp.left(80) + "..." : resp;
    int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    m_table->setItem(row, 1, new QTableWidgetItem(c));
    m_table->setItem(row, 2, new QTableWidgetItem(r));
}

void MainWindow::onRegister() { sendCommand(QString("REGISTER|%1|%2").arg(m_login->text().trimmed(), m_pass->text())); }
void MainWindow::onLogin() { sendCommand(QString("AUTH|%1|%2").arg(m_login->text().trimmed(), m_pass->text())); }
void MainWindow::onAesEncrypt() { sendCommand(QString("AES_ENCRYPT|%1|%2").arg(m_aesKey->text(), m_aesInput->text())); }
void MainWindow::onAesDecrypt() {
    QString hex = m_aesInput->text().trimmed().remove(QRegularExpression("\\s+")).toLower();
    if (hex.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Вход пуст."); return; }
    sendCommand(QString("AES_DECRYPT|%1|%2").arg(m_aesKey->text(), hex));
}
void MainWindow::onSha1() { sendCommand("SHA1|" + m_shaInput->text()); }
void MainWindow::onNewton() {
    sendCommand(QString("NEWTON|%1|%2").arg(m_newtonX0->value()).arg(m_newtonEps->value()));
}
void MainWindow::onAudioEmbed()
{
    if (m_audioSrc->text().isEmpty()) { QMessageBox::warning(this, "Ошибка", "Укажите WAV."); return; }
    QFile f(m_audioSrc->text());
    if (!f.open(QIODevice::ReadOnly)) { QMessageBox::warning(this, "Ошибка", "Не открыть WAV."); return; }
    QString b64 = QString::fromLatin1(f.readAll().toBase64());
    sendCommand(QString("AUDIO_EMBED|%1|%2").arg(b64, m_audioMsg->text()),
                QString("AUDIO_EMBED|[%1 b64]|%2").arg(b64.size()).arg(m_audioMsg->text()));
}
void MainWindow::onAudioExtract()
{
    if (m_audioExt->text().isEmpty()) { QMessageBox::warning(this, "Ошибка", "Укажите WAV."); return; }
    QFile f(m_audioExt->text());
    if (!f.open(QIODevice::ReadOnly)) { QMessageBox::warning(this, "Ошибка", "Не открыть WAV."); return; }
    QString b64 = QString::fromLatin1(f.readAll().toBase64());
    sendCommand("AUDIO_EXTRACT|" + b64, QString("AUDIO_EXTRACT|[%1 b64]").arg(b64.size()));
}

void MainWindow::setLoggedIn(const QString& user)
{
    m_user = user;
    if (user.isEmpty()) {
        m_authFlag->setText("○ Гость");
        m_authFlag->setStyleSheet("color: red;");
        m_authFlag->setToolTip("Не авторизован — нужен AUTH");
    } else {
        m_authFlag->setText("● " + user);
        m_authFlag->setStyleSheet("color: green;");
        m_authFlag->setToolTip("Авторизован как " + user);
    }
}
