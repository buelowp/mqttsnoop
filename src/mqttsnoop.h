#ifndef MQTTSNOOP_H
#define MQTTSNOOP_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <qmqtt.h>
#include "jsonwidget.h"

class MQTTSnoopWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MQTTSnoopWindow(QWidget *parent = nullptr);
    ~MQTTSnoopWindow() override;

public slots:
    void connected();
    void disconnected();
    void error(const QMQTT::ClientError error);
    void subscribed(const QString& topic, const quint8 qos);
    void unsubscribed(const QString& topic);
    void published(const quint16 msgid, const quint8 qos);
    void pingresp();
    void received(const QMQTT::Message& message);
    
private:
    bool populateNewLayout(QVBoxLayout *layout, QString topic, QByteArray payload);
    bool populateExistingLayout(QVBoxLayout *layout, QString topic, QByteArray payload);
    
    QMQTT::Client *m_mqttClient;
    QTabWidget *m_mainWidget;
    QLabel *m_sbConnected;
    QLabel *m_sbTopicsReceived;
    QLabel *m_sbMessagesPerMinute;
};

#endif // MQTTSNOOP_H
