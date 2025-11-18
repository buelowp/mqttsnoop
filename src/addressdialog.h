// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ADDRESSDIALOG_H
#define ADDRESSDIALOG_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>

#include "qlabelevent.h"

class AddressDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AddressDialog(QWidget *parent = nullptr);
    ~AddressDialog() override;

    void setServerText(QString text) { m_serverLabel->setText(text); }
    void setServerPort(QString port) { m_serverPortLabel->setText(port); }
    void setCACertFile(QString text) { m_caCertificateLabel->setText(text); }
    void setClientKeyFile(QString text) { m_clientKeyLabel->setText(text); }
    void setClientKeyCert(QString text) { m_clientCertificateLabel->setText(text); }

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

public slots:
    void setText(QString text);
    void textChanged(const QString &text);
    void cancel();
    void accept();
    void caCert();
    void clientCert();
    void clientKey();
    void autoConnectChange(Qt::CheckState state);

signals:
    void newServerValue(QString value, int port);
    void newClientCertificate(QString cert);
    void newCACertificate(QString cert);
    void newClientKey(QString key);
    void autoConnect(bool state);

private:
    QGridLayout *m_layout;
    QLineEdit *m_serverLabel;
    QLineEdit *m_serverPortLabel;
    QLabelEvent *m_caCertificateLabel;
    QLabelEvent *m_clientCertificateLabel;
    QLabelEvent *m_clientKeyLabel;
    QCheckBox *m_autoConnect;
    QPushButton *m_ok;
    QPushButton *m_cancel;
    bool m_isValid;
    QString m_caCert;
    QString m_clientCert;
    QString m_clientKey;
    QString m_hostName;
    int m_port;
    QString m_homeDir;
};

#endif // ADDRESSDIALOG_H
