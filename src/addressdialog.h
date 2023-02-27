// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#ifndef ADDRESSDIALOG_H
#define ADDRESSDIALOG_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>

class AddressDialog : public QWidget
{
    Q_OBJECT

public:
    AddressDialog(QWidget *parent = nullptr);
    ~AddressDialog();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

public slots:
    void setText(QString text);
    void textChanged(const QString &text);
    void cancel();

signals:
    void newServerValue(QString value);

private:
    QGridLayout *m_layout;
    QLineEdit *m_serverLabel;
    QPushButton *m_ok;
    QPushButton *m_cancel;
    bool m_isValid;
};

#endif // ADDRESSDIALOG_H
