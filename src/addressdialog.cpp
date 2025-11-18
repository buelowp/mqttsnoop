// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "addressdialog.h"

AddressDialog::AddressDialog(QWidget *parent) : QWidget(parent)
{
    QSettings settings("home", "mqttsnoop");

    m_layout = new QGridLayout();
    m_serverLabel = new QLineEdit();
    m_serverPortLabel = new QLineEdit();
    m_autoConnect = new QCheckBox();
    QLabel *ccl = new QLabel("Client Certificate");
    QLabel *ckl = new QLabel("Client Key");
    QLabel *cal = new QLabel("CA Certificate");
    QLabel *hnp = new QLabel("Hostname:Port");
    QLabel *mac = new QLabel("Auto Reconnect on Start");

    qDebug() << "Client Cert:" << settings.value("clientcert").toString();
    qDebug() << "Client Key:" << settings.value("clientkey").toString();
    qDebug() << "CA Cert:" << settings.value("cacert").toString();
    m_clientCertificateLabel = new QLabelEvent();
    m_clientCertificateLabel->setText(settings.value("clientcert").toString());
    connect(m_clientCertificateLabel, &QLabelEvent::clicked, this, &AddressDialog::clientCert);
    m_clientKeyLabel = new QLabelEvent();
    m_clientKeyLabel->setText(settings.value("clientkey").toString());
    connect(m_clientKeyLabel, &QLabelEvent::clicked, this, &AddressDialog::clientKey);
    m_caCertificateLabel = new QLabelEvent();
    m_caCertificateLabel->setText(settings.value("cacert").toString());
    connect(m_caCertificateLabel, &QLabelEvent::clicked, this, &AddressDialog::caCert);

    m_isValid = false;
    m_cancel = new QPushButton("Cancel");
    m_ok = new QPushButton("Connect");
    setWindowTitle("Host Connection Dialog");
    connect(m_serverLabel, &QLineEdit::textChanged, this, &AddressDialog::textChanged);
    connect(m_cancel, &QPushButton::clicked, this, &AddressDialog::cancel);
    connect(m_ok, &QPushButton::clicked, this, &AddressDialog::accept);
    connect(m_autoConnect, &QCheckBox::checkStateChanged, this, &AddressDialog::autoConnectChange);

    m_layout->addWidget(hnp, 0, 0, 1, 1);
    m_layout->addWidget(m_serverLabel, 0, 1, 1, 2);
    m_layout->addWidget(m_serverPortLabel, 0, 3, 1, 1);
    m_layout->addWidget(mac, 1, 0, 1, 3);
    m_layout->addWidget(m_autoConnect, 1, 2, 1, 1);
    m_layout->addWidget(ccl, 2, 0, 1, 1);
    m_layout->addWidget(m_clientCertificateLabel, 2, 1, 1, 3);
    m_layout->addWidget(ckl, 3, 0, 1, 1);
    m_layout->addWidget(m_clientKeyLabel, 3, 1, 1, 3);
    m_layout->addWidget(cal, 4, 0, 1, 1);
    m_layout->addWidget(m_caCertificateLabel, 4, 1, 1, 3);
    m_layout->addWidget(m_ok, 5, 0, 1, 2);
    m_layout->addWidget(m_cancel, 5, 2, 1, 2);
    setLayout(m_layout);

    m_homeDir = QString::fromUtf8(qgetenv("HOME"));
}

AddressDialog::~AddressDialog()
{
}

void AddressDialog::autoConnectChange(Qt::CheckState state)
{
    if (state == Qt::Unchecked)
        emit autoConnect(false);
    else
        emit autoConnect(true);
}

void AddressDialog::caCert()
{
    if (m_homeDir.size())
        m_caCert = QFileDialog::getOpenFileName(this, tr("Open CA File"), m_homeDir, tr("Certificates (*.crt *.pem)"));
    else
        m_caCert = QFileDialog::getOpenFileName(this, tr("Open CA File"), "/tmp", tr("Certificates (*.crt *.pem)"));

    if (m_caCert.size()) {
        qDebug() << "new CACert:" << m_caCert;
        m_caCertificateLabel->setText(m_caCert);
        emit newCACertificate(m_caCert);
    }
}

void AddressDialog::clientKey()
{
    if (m_homeDir.size())
        m_clientKey = QFileDialog::getOpenFileName(this, tr("Open Client Key File"), m_homeDir, tr("Keys (*.key)"));
    else
        m_clientKey = QFileDialog::getOpenFileName(this, tr("Open Client Key File"), "/tmp", tr("Keys (*.key)"));

    if (m_clientKey.size()) {
        qDebug() << "new client key:" << m_clientKey;
        m_clientKeyLabel->setText(m_clientKey);
        emit newClientKey(m_clientKey);
    }
}

void AddressDialog::clientCert()
{
    if (m_homeDir.size())
        m_clientCert = QFileDialog::getOpenFileName(this, tr("Open Client Cert File"), m_homeDir, tr("Certificates (*.crt *.pem)"));
    else
        m_clientCert = QFileDialog::getOpenFileName(this, tr("Open Client Cert File"), "/tmp", tr("Certificates (*.crt *.pem)"));

    if (m_clientCert.size()) {
        qDebug() << "new client cert:" << m_clientCert;
        m_clientCertificateLabel->setText(m_clientCert);
        emit newClientCertificate(m_clientCert);
    }
}

void AddressDialog::cancel()
{
    qDebug() << __PRETTY_FUNCTION__;
    close();
}

void AddressDialog::accept()
{
    QSettings settings("home", "mqttsnoop");

    settings.setValue("mqttserver", m_serverLabel->text());
    settings.setValue("mqttport", m_serverPortLabel->text().toInt());
    emit newServerValue(m_serverLabel->text(), m_serverPortLabel->text().toInt());
    close();
}

void AddressDialog::setText(QString text)
{
    m_serverLabel->setText(text);
}

void AddressDialog::textChanged(const QString &text)
{
    m_hostName = text;
}

void AddressDialog::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
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
