#ifndef MQTTSNOOP_H
#define MQTTSNOOP_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <QtMqtt/QtMqtt>

#include "jsonwidget.h"
#include "tabwidget.h"
#include "eventcounter.h"
#include "addressdialog.h"

class MQTTSnoopWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MQTTSnoopWindow(QWidget *parent = nullptr);
    ~MQTTSnoopWindow() override;

public slots:
    void connected();
    void disconnected();
    void error(QMqttClient::ClientError error);
    void subscribed(const QString& topic, const quint8 qos);
    void unsubscribed(const QString& topic);
    void received(const QByteArray &message, const QMqttTopicName &topic);
    void displayMPM(uint64_t);
    void menuSubscribe();
    void menuConnect();
    void connectAddressInput();
    void connectAddressInput(QString address);

protected:
    void moveEvent(QMoveEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    
private:
    void updateTab(QString topic, QString localTopic, QJsonDocument doc, TabWidget *tab);
    void newTab(QString topic, QString localTopic, QJsonDocument doc);
    void buildStatusBar();
    void buildMenuBar();
    void updateMpmCount();
    bool compareTabText(QString, QString);
    
    QMqttClient *m_mqttClient;
    QMqttSubscription *m_subscription;
    AddressDialog *m_addressDialog;
    QTabWidget *m_tabWidget;
    QWidget *m_statusbarWidget;
    QHBoxLayout *m_statusbarLayout;
    QLabel *m_sbConnected;     
    QLabel *m_sbTopicsReceived;
    QLabel *m_sbMessagesPerMinute;
    QLabel *m_sbCurrentTopic;
    QMutex m_newTabMutex;
    QMutex m_updateTabMutex;
    QMap<int, int> m_messageCounter;
    QString m_currentTopic;
    QString m_hostName;
    QHostAddress m_mqttServer;
    QAction *m_subscribeAct;
    EventCounter *m_eventCounter;
    int m_topics;
    uint32_t m_mpm;
};

#endif // MQTTSNOOP_H
