#include "mqttsnoop.h"

MQTTSnoopWindow::MQTTSnoopWindow(QWidget *parent) : QMainWindow(parent), m_topics(0), m_mpm(0)
{
    m_mainWidget = new QTabWidget();
    
    m_mqttClient = new QMQTT::Client("172.24.1.12", 1883, false, false);
    QString hn = QString("%1-%2").arg(QHostInfo::localHostName()).arg(QRandomGenerator::global()->generate());
    m_mqttClient->setClientId(hn);
    qDebug() << __PRETTY_FUNCTION__ << "Connecting to host with name" << hn;
    m_mqttClient->connectToHost();
    m_mqttClient->setAutoReconnect(true);
    m_mqttClient->setAutoReconnectInterval(10000);
    
    m_eventCounter = new EventCounter();
    connect(m_eventCounter, SIGNAL(minuteEvents(uint64_t)), this, SLOT(displayMPM(uint64_t)));
    
    connect(m_mqttClient, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_mqttClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_mqttClient, SIGNAL(error(const QMQTT::ClientError)), this, SLOT(error(const QMQTT::ClientError)));

    connect(m_mqttClient, SIGNAL(subscribed(const QString&, const quint8)), this, SLOT(subscribed(const QString&, const quint8)));
    connect(m_mqttClient, SIGNAL(unsubscribed(const QString&)), this, SLOT(unsubscribed(const QString&)));
    connect(m_mqttClient, SIGNAL(pingresp()), this, SLOT(pingresp()));
    connect(m_mqttClient, SIGNAL(received(const QMQTT::Message&)), this, SLOT(received(const QMQTT::Message&)));
    
    setCentralWidget(m_mainWidget);
    
    buildStatusBar();
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::lightGray);
    setAutoFillBackground(true);
    setPalette(pal);
}

MQTTSnoopWindow::~MQTTSnoopWindow()
{
    m_mqttClient->unsubscribe("#");
    m_mqttClient->disconnect();
}

void MQTTSnoopWindow::displayMPM(uint64_t e)
{
    m_sbMessagesPerMinute->setText(QString("Messages Per Minute: %1").arg(e));
}

void MQTTSnoopWindow::buildStatusBar()
{
    QPalette p;
    p.setColor(QPalette::Window, Qt::white);
    
    m_statusbarWidget = new QWidget();
    m_statusbarLayout = new QHBoxLayout();
    
    m_sbConnected = new QLabel("Disconnected");
    m_sbConnected->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_sbConnected->setAutoFillBackground(true);
//     m_sbConnected->setPalette(p);
    
    m_sbTopicsReceived = new QLabel("Topics: 0");
    m_sbTopicsReceived->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_sbTopicsReceived->setAlignment(Qt::AlignCenter);
    m_sbTopicsReceived->setAutoFillBackground(true);
//     m_sbTopicsReceived->setPalette(p);
    
    m_sbMessagesPerMinute = new QLabel("Messages Per Minute: 0");
    m_sbMessagesPerMinute->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_sbMessagesPerMinute->setAlignment(Qt::AlignRight);
    m_sbMessagesPerMinute->setAutoFillBackground(true);
//    m_sbMessagesPerMinute->setPalette(p);
    
    m_statusbarLayout->addWidget(m_sbConnected);
    m_statusbarLayout->addWidget(m_sbTopicsReceived);
    m_statusbarLayout->addWidget(m_sbMessagesPerMinute);
    m_statusbarLayout->addStretch();
    m_statusbarLayout->setSizeConstraint(QLayout::SetMaximumSize);
    
    m_statusbarWidget->setLayout(m_statusbarLayout);

    statusBar()->addWidget(m_statusbarWidget);
}

void MQTTSnoopWindow::closeEvent(QCloseEvent* e)
{
}

void MQTTSnoopWindow::showEvent(QShowEvent* e)
{
    Q_UNUSED(e)
    
    m_sbConnected->setMinimumWidth((statusBar()->width() / 3) - 15);
    m_sbTopicsReceived->setMinimumWidth((statusBar()->width() / 3) - 15);
    m_sbMessagesPerMinute->setMinimumWidth((statusBar()->width() / 3) - 15);
}

void MQTTSnoopWindow::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
    
    m_sbConnected->setMinimumWidth((statusBar()->width() / 3) - 15);
    m_sbTopicsReceived->setMinimumWidth((statusBar()->width() / 3) - 15);
    m_sbMessagesPerMinute->setMinimumWidth((statusBar()->width() / 3) - 15);    
}

void MQTTSnoopWindow::newTab(QString topic, QJsonDocument json)
{
    QMutexLocker locker(&m_newTabMutex);
    QString parentTopic = topic.left(topic.indexOf("/"));
    QWidget *parentWidget = new QWidget(m_mainWidget);
    QHBoxLayout *parentLayout = new QHBoxLayout();
    parentWidget->setLayout(parentLayout);
    QScrollArea *parentScroll = new QScrollArea();
    parentLayout->addWidget(parentScroll);
    TabWidget *tab = new TabWidget();
    parentScroll->setWidgetResizable(true);
    parentScroll->setWidget(tab);
    
    tab->addJson(topic, json);
    m_mainWidget->addTab(parentWidget, parentTopic);
    m_topics++;
}

void MQTTSnoopWindow::updateTab(QString topic, QJsonDocument doc, TabWidget* tab)
{
    QMutexLocker locker(&m_updateTabMutex);
    QString parentTopic = topic.left(topic.indexOf("/"));
    if (tab->addJson(topic, doc))
        m_topics++;
    
//     qDebug() << __PRETTY_FUNCTION__ << "Updated a tab";
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
    QJsonDocument json = QJsonDocument::fromJson(message.payload());
    int i = 0;
    
    m_eventCounter->bump();
    
    if (json.isNull() || json.isEmpty()) {
        return;
    }
    
    for (i = 0; i < m_mainWidget->count(); i++) {
        if (m_mainWidget->tabText(i) == parentTopic) {
            QWidget *top = static_cast<QWidget*>(m_mainWidget->widget(i));
            QHBoxLayout *topLayout = static_cast<QHBoxLayout*>(top->layout());
            QScrollArea *scroller = static_cast<QScrollArea*>(topLayout->itemAt(0)->widget());
            TabWidget *widget = static_cast<TabWidget*>(scroller->widget());
            updateTab(message.topic(), json, widget);
            m_sbTopicsReceived->setText(QString("Topics: %1").arg(m_topics));
            return;
        }
    }

    newTab(message.topic(), json);
    
    m_sbTopicsReceived->setText(QString("Topics: %1").arg(m_topics));
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






