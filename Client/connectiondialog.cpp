#include "connectiondialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>

ConnectionDialog::ConnectionDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Подключение");
    m_host = new QLineEdit("127.0.0.1", this);
    m_port = new QSpinBox(this);
    m_port->setRange(1, 65535);
    m_port->setValue(34944);
    auto* form = new QFormLayout(this);
    form->addRow("Хост:", m_host);
    form->addRow("Порт:", m_port);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addWidget(btns);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString ConnectionDialog::host() const { return m_host->text().trimmed(); }
int ConnectionDialog::port() const { return m_port->value(); }
