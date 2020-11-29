#include "mqttsnoop.h"

MQTTSnoopWindow::MQTTSnoopWindow(QWidget *parent) : QMainWindow(parent)
{
    m_mainWidget = new QTabWidget();
    
    m_mqttClient = new QMQTT::Client("172.24.1.13", 1883, false, false);
    QString hn = QString("%1-%2").arg(QHostInfo::localHostName()).arg(QRandomGenerator::global()->generate());
    m_mqttClient->setClientId(hn);
    qDebug() << __PRETTY_FUNCTION__ << "Connecting to host with name" << hn;
    m_mqttClient->connectToHost();
    m_mqttClient->setAutoReconnect(true);
    m_mqttClient->setAutoReconnectInterval(10000);
    
    connect(m_mqttClient, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_mqttClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_mqttClient, SIGNAL(error(const QMQTT::ClientError)), this, SLOT(error(const QMQTT::ClientError)));

    connect(m_mqttClient, SIGNAL(subscribed(const QString&, const quint8)), this, SLOT(subscribed(const QString&, const quint8)));
    connect(m_mqttClient, SIGNAL(unsubscribed(const QString&)), this, SLOT(unsubscribed(const QString&)));
    connect(m_mqttClient, SIGNAL(pingresp()), this, SLOT(pingresp()));
    connect(m_mqttClient, SIGNAL(received(const QMQTT::Message&)), this, SLOT(received(const QMQTT::Message&)));
    
    setCentralWidget(m_mainWidget);
    
    m_sbConnected = new QLabel("Disconnected");
    m_sbTopicsReceived = new QLabel("Topics: 0");
    m_sbMessagesPerMinute = new QLabel("Messages Per Minute: 0");
    statusBar()->addWidget(m_sbConnected);
    statusBar()->addWidget(m_sbTopicsReceived);
    statusBar()->addWidget(m_sbMessagesPerMinute);
}

MQTTSnoopWindow::~MQTTSnoopWindow()
{
    m_mqttClient->unsubscribe("#");
}

bool MQTTSnoopWindow::populateNewLayout(QVBoxLayout *layout, QString topic, QByteArray payload)
{
    QJsonDocument json = QJsonDocument::fromJson(payload);
    
    if (json.isNull() || json.isEmpty()) {
        qDebug() << __PRETTY_FUNCTION__ << ": Returning false";
        return false;
    }
    
    qDebug() << __PRETTY_FUNCTION__ << "Populating new topic" << topic;
    JsonWidget *widget = new JsonWidget(topic);
    widget->addJson(json);
    widget->resize(widget->sizeHint());
    layout->addWidget(widget);
    return true;
}

bool MQTTSnoopWindow::populateExistingLayout(QVBoxLayout *layout, QString topic, QByteArray payload)
{
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    
    for (int i = 0; i < layout->count(); i++) {
        JsonWidget *widget = static_cast<JsonWidget*>(layout->itemAt(i)->widget());
        if (widget->topic() == topic) {
            widget->addJson(doc);
            return true;
        }
    }
    populateNewLayout(layout, topic, payload);
    return false;
}

void MQTTSnoopWindow::connected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Connection success";
    m_mqttClient->subscribe("#");   // Subscribe to EVERYTHING on purpose
    m_sbConnected->setText(QString("Connected: %1").arg(m_mqttClient->hostName()));
}

void MQTTSnoopWindow::disconnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Disconnected from MQTT server";
    m_sbConnected->setText(QString("Disconnected"));
}

void MQTTSnoopWindow::error(const QMQTT::ClientError error)
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT error:" << error;
}

void MQTTSnoopWindow::pingresp()
{
}

void MQTTSnoopWindow::published(const quint16 msgid, const quint8 qos)
{
    Q_UNUSED(msgid)
    Q_UNUSED(qos)
}

void MQTTSnoopWindow::received(const QMQTT::Message& message)
{
    QString parentTopic = message.topic().left(message.topic().indexOf("/"));
    int i = 0;
    
    for (i = 0; i < m_mainWidget->count(); i++) {
        if (m_mainWidget->tabText(i) == parentTopic) {
            QWidget *widget = m_mainWidget->widget(i);
            QVBoxLayout *layout = static_cast<QVBoxLayout*>(widget->layout());
            populateExistingLayout(layout, message.topic(), message.payload());
            m_sbTopicsReceived->setText(QString("Topics: %1").arg(m_mainWidget->count()));
            return;
        }
    }
    
    QScrollArea *widget = new QScrollArea();
    QVBoxLayout *layout = new QVBoxLayout();
    if (populateNewLayout(layout, message.topic(), message.payload())) {
        widget->setLayout(layout);
        m_mainWidget->addTab(widget, parentTopic);
    }
    m_sbTopicsReceived->setText(QString("Topics: %1").arg(m_mainWidget->count()));
}

void MQTTSnoopWindow::subscribed(const QString& topic, const quint8 qos)
{
    Q_UNUSED(topic)
    Q_UNUSED(qos)
    qDebug() << __PRETTY_FUNCTION__ << ": subscribed to:" << topic;
}

void MQTTSnoopWindow::unsubscribed(const QString& topic)
{
    qDebug() << __PRETTY_FUNCTION__ << ": Unsubscribed from topic:" << topic;
}






