#ifndef MQTTSNOOP_H
#define MQTTSNOOP_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <QtQmqtt/qmqtt.h>

#include "jsonwidget.h"
#include "tabwidget.h"
#include "eventcounter.h"

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
    void displayMPM(uint64_t);
    
protected:
    void showEvent(QShowEvent *e);
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *e);
    
private:
    void updateTab(QString topic, QJsonDocument doc, TabWidget *tab);
    void newTab(QString topic, QJsonDocument doc);
    void buildStatusBar();
    void updateMpmCount();
    
    QMQTT::Client *m_mqttClient;
    QTabWidget *m_mainWidget;
    QWidget *m_statusbarWidget;
    QHBoxLayout *m_statusbarLayout;
    QLabel *m_sbConnected;     
    QLabel *m_sbTopicsReceived;
    QLabel *m_sbMessagesPerMinute;
    QMutex m_newTabMutex;
    QMutex m_updateTabMutex;
    QMap<int, int> m_messageCounter;
    EventCounter *m_eventCounter;
    int m_topics;
    uint32_t m_mpm;
};

#endif // MQTTSNOOP_H
