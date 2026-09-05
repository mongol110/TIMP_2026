#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

class QLineEdit;
class QSpinBox;

class ConnectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectionDialog(QWidget* parent = nullptr);
    QString host() const;
    int port() const;
private:
    QLineEdit* m_host;
    QSpinBox* m_port;
};

#endif
