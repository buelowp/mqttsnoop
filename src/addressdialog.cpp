// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "addressdialog.h"

AddressDialog::AddressDialog(QWidget *parent) : QWidget(parent)
{
    QSettings settings("mqttsnoop", "mqttsnoop");

    m_layout = new QGridLayout();
    m_serverLabel = new QLineEdit();
    m_isValid = false;
    m_cancel = new QPushButton("Cancel");
    m_ok = new QPushButton("Connect");
    setWindowTitle("MQTT Address");
    connect(m_serverLabel, &QLineEdit::textChanged, this, &AddressDialog::textChanged);
    connect(m_cancel, &QPushButton::clicked, this, &AddressDialog::cancel);
    connect(m_ok, &QPushButton::clicked, this, &AddressDialog::close);
    m_ok->setEnabled(false);

    m_layout->addWidget(m_serverLabel, 0, 0, 1, 2);
    m_layout->addWidget(m_ok, 1, 0, 1, 1);
    m_layout->addWidget(m_cancel, 1, 1, 1, 1);
    setLayout(m_layout);
}

AddressDialog::~AddressDialog()
{
}

void AddressDialog::cancel()
{
    m_isValid = false;
    close();
}

void AddressDialog::setText(QString text)
{
    m_serverLabel->setText(text);
}

void AddressDialog::textChanged(const QString &text)
{
    QHostAddress validated(text);

    qDebug() << __PRETTY_FUNCTION__ << ": m_isValid =" << m_isValid;
    m_isValid = !validated.isNull();

    if (m_isValid)
        m_ok->setEnabled(true);
}

void AddressDialog::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);

    qDebug() << __PRETTY_FUNCTION__ << ": m_isValid" << m_isValid;
    if (m_isValid) {
        QSettings settings("mqttsnoop", "mqttsnoop");

        settings.setValue("mqttserver", m_serverLabel->text());
        emit newServerValue(m_serverLabel->text());
    }
}

void AddressDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    qDebug() << __PRETTY_FUNCTION__ << ":" << geometry();
}

void AddressDialog::moveEvent(QMoveEvent* event)
{
    QWidget::moveEvent(event);
}
