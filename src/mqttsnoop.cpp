#include "mqttsnoop.h"

MQTTSnoopWindow::MQTTSnoopWindow(QWidget *parent) : QMainWindow(parent), m_topics(0), m_mpm(0)
{
    m_tabWidget = new QTabWidget();
    
    m_mqttClient = new QMQTT::Client();
    m_hostName = QString("%1-%2").arg(QHostInfo::localHostName()).arg(QRandomGenerator::global()->generate());
    m_mqttClient->setClientId(m_hostName);
    m_mqttClient->setAutoReconnect(true);
    m_mqttClient->setAutoReconnectInterval(10000);

    m_addressDialog = new AddressDialog();
    m_addressDialog->setWindowModality(Qt::WindowModal);

    m_eventCounter = new EventCounter();
    connect(m_eventCounter, SIGNAL(minuteEvents(uint64_t)), this, SLOT(displayMPM(uint64_t)));
    
    connect(m_mqttClient, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_mqttClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_mqttClient, SIGNAL(error(const QMQTT::ClientError)), this, SLOT(error(const QMQTT::ClientError)));

    connect(m_mqttClient, SIGNAL(subscribed(const QString&, const quint8)), this, SLOT(subscribed(const QString&, const quint8)));
    connect(m_mqttClient, SIGNAL(unsubscribed(const QString&)), this, SLOT(unsubscribed(const QString&)));
    connect(m_mqttClient, SIGNAL(pingresp()), this, SLOT(pingresp()));
    connect(m_mqttClient, SIGNAL(received(const QMQTT::Message&)), this, SLOT(received(const QMQTT::Message&)));

    connect(m_addressDialog, &AddressDialog::newServerValue, this, qOverload<QString>(&MQTTSnoopWindow::connectAddressInput));
    
    setCentralWidget(m_tabWidget);
    
    buildStatusBar();
    buildMenuBar();
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::darkGray);
    setAutoFillBackground(true);
    setPalette(pal);

    m_currentTopic = "#";

    QSettings settings("mqttsnoop", "mqttsnoop");
    if (settings.contains("mqttserver")) {
        if (settings.contains("mqttport")) {
            m_mqttClient->setPort(settings.value("mqttport").toInt());
        }
        else {
            m_mqttClient->setPort(1883);
            settings.setValue("mqttport", 1883);
        }
        m_mqttServer.setAddress(settings.value("mqttserver").toString());
        m_addressDialog->setText(settings.value("mqttserver").toString());
        if (!m_mqttServer.isNull()) {
            connectAddressInput();
        }
    }
    else {
        menuConnect();
    }
}

MQTTSnoopWindow::~MQTTSnoopWindow()
{
    m_mqttClient->unsubscribe(m_currentTopic);
    m_mqttClient->disconnect();
}

void MQTTSnoopWindow::buildMenuBar()
{
    QToolBar *netToolBar = addToolBar(tr("Network"));
    const QIcon connectIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *connectAct = new QAction(connectIcon, tr("&Connect"), this);
    connectAct->setStatusTip(tr("Connect"));
    connect(connectAct, &QAction::triggered, this, &MQTTSnoopWindow::menuConnect);
    netToolBar->addAction(connectAct);

    const QIcon subscribeIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *subsribeAct = new QAction(subscribeIcon, tr("&Subscribe"), this);
    subsribeAct->setStatusTip(tr("Subscribe"));
    connect(subsribeAct, &QAction::triggered, this, &MQTTSnoopWindow::menuSubscribe);
    netToolBar->addAction(subsribeAct);
}


void MQTTSnoopWindow::menuConnect()
{
    m_addressDialog->show();
}

void MQTTSnoopWindow::menuSubscribe()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Subscribe to Topics"),
                                         tr("Topic String"), QLineEdit::Normal,
                                         m_currentTopic, &ok);


    qDebug() << __PRETTY_FUNCTION__ << ": Subscribing to" << text << ", ok is" << ok;
    if (ok && text.size()) {
        qDebug() << __PRETTY_FUNCTION__ << ": Subscribing to" << text;
        m_mqttClient->unsubscribe(m_currentTopic);
        m_mqttClient->subscribe(text);
    }
}

void MQTTSnoopWindow::displayMPM(uint64_t e)
{
    m_sbMessagesPerMinute->setText(QString("Messages Per Minute: %1").arg(e));
}

void MQTTSnoopWindow::buildStatusBar()
{
    QPalette palette;

    m_statusbarWidget = new QWidget();
    m_statusbarLayout = new QHBoxLayout();
    
    m_sbConnected = new QLabel("Disconnected");
    m_sbConnected->setAutoFillBackground(true);
    m_sbConnected->setStyleSheet("QLabel { background-color: white; color : black; }");

    m_sbTopicsReceived = new QLabel("Topics Received: 0");
    m_sbTopicsReceived->setAlignment(Qt::AlignCenter);
    m_sbTopicsReceived->setAutoFillBackground(true);
    m_sbTopicsReceived->setStyleSheet("QLabel { background-color: white; color : black; }");

    m_sbMessagesPerMinute = new QLabel("Messages Per Minute: 0");
    m_sbMessagesPerMinute->setAlignment(Qt::AlignRight);
    m_sbMessagesPerMinute->setAutoFillBackground(true);
    m_sbMessagesPerMinute->setStyleSheet("QLabel { background-color: white; color : black; }");
    
    m_sbCurrentTopic = new QLabel(QString("Topic: ") + m_currentTopic);
    m_sbCurrentTopic->setAlignment(Qt::AlignCenter);
    m_sbCurrentTopic->setAutoFillBackground(true);
    m_sbCurrentTopic->setStyleSheet("QLabel { background-color: white; color : black; }");

    m_statusbarLayout->addWidget(m_sbConnected);
    m_statusbarLayout->addStretch();
    m_statusbarLayout->addWidget(m_sbTopicsReceived);
    m_statusbarLayout->addStretch();
    m_statusbarLayout->addWidget(m_sbCurrentTopic);
    m_statusbarLayout->addStretch();
    m_statusbarLayout->addWidget(m_sbMessagesPerMinute);
    m_statusbarLayout->setSizeConstraint(QLayout::SetMaximumSize);
    
    m_statusbarWidget->setLayout(m_statusbarLayout);
    statusBar()->addWidget(m_statusbarWidget);
}

void MQTTSnoopWindow::closeEvent(QCloseEvent* e)
{
}

void MQTTSnoopWindow::moveEvent(QMoveEvent* e)
{
    int deltaX = e->pos().x() - e->oldPos().x();
    int deltaY = e->pos().y() - e->oldPos().y();
    m_addressDialog->move(m_addressDialog->geometry().x() + (e->pos().x() - e->oldPos().x()), m_addressDialog->geometry().y() + (e->pos().y() - e->oldPos().y()));
    qDebug() << __PRETTY_FUNCTION__ << ": deltaX  :" << deltaX;
    qDebug() << __PRETTY_FUNCTION__ << ": deltaY  :" << deltaY;
    qDebug() << __PRETTY_FUNCTION__ << ": geometry:" << m_addressDialog->geometry();
    qDebug() << __PRETTY_FUNCTION__ << ": frame   :" << frameGeometry();
}

void MQTTSnoopWindow::showEvent(QShowEvent* e)
{
    Q_UNUSED(e)
    statusBar()->showMaximized();
    m_sbConnected->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_sbTopicsReceived->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_sbMessagesPerMinute->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_sbCurrentTopic->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_addressDialog->move((frameGeometry().width() / 2), (frameGeometry().height() / 2));
    qDebug() << __PRETTY_FUNCTION__ << ": geometry:" << m_addressDialog->geometry();
    qDebug() << __PRETTY_FUNCTION__ << ": frame   :" << frameGeometry();
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
    QWidget *parentWidget = new QWidget(m_tabWidget);
    QHBoxLayout *parentLayout = new QHBoxLayout();
    parentLayout->setSpacing(5);
    parentWidget->setLayout(parentLayout);
    QScrollArea *parentScroll = new QScrollArea();
    parentLayout->addWidget(parentScroll);
    TabWidget *tab = new TabWidget();
    parentScroll->setWidgetResizable(true);
    parentScroll->setWidget(tab);
    
    tab->addJson(topic, json);
    m_tabWidget->addTab(parentWidget, parentTopic);
    m_topics++;
}

void MQTTSnoopWindow::updateTab(QString topic, QJsonDocument doc, TabWidget* tab)
{
    QMutexLocker locker(&m_updateTabMutex);
    QString parentTopic = topic.left(topic.indexOf("/"));
    if (tab->addJson(topic, doc))
        m_topics++;
}

void MQTTSnoopWindow::connected()
{
    m_sbConnected->setText(QString("Connected: %1").arg(m_mqttClient->host().toString()));
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT connected to" << m_mqttClient->host();
}

void MQTTSnoopWindow::disconnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Disconnected from MQTT server";
    m_sbConnected->setText(QString("Disconnected"));
}

void MQTTSnoopWindow::error(const QMQTT::ClientError error)
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT error:" << error;
    m_sbConnected->setText(QString("MQTT Error: %1").arg(error));
    if (error == QMQTT::ClientError::SocketHostNotFoundError || error == QMQTT::ClientError::SocketTimeoutError) {
        m_mqttClient->disconnectFromHost();
        m_addressDialog->show();
    }
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
    
    for (i = 0; i < m_tabWidget->count(); i++) {
        if (m_tabWidget->tabText(i) == parentTopic) {
            QWidget *top = static_cast<QWidget*>(m_tabWidget->widget(i));
            QHBoxLayout *topLayout = static_cast<QHBoxLayout*>(top->layout());
            QScrollArea *scroller = static_cast<QScrollArea*>(topLayout->itemAt(0)->widget());
            TabWidget *widget = static_cast<TabWidget*>(scroller->widget());
            updateTab(message.topic(), json, widget);
            m_sbTopicsReceived->setText(QString("Topics Received: %1").arg(m_topics));
            return;
        }
    }

    newTab(message.topic(), json);
    
    m_sbTopicsReceived->setText(QString("Topics: %1").arg(m_topics));
}

void MQTTSnoopWindow::subscribed(const QString& topic, const quint8 qos)
{
    Q_UNUSED(qos)
    m_sbCurrentTopic->setText(QString("Topic: %1, QOS(%2)").arg(topic).arg(qos));
    m_currentTopic = topic;
}

void MQTTSnoopWindow::unsubscribed(const QString& topic)
{
    Q_UNUSED(topic)
    m_sbCurrentTopic->clear();
}

void MQTTSnoopWindow::connectAddressInput()
{
    m_mqttClient->setHost(m_mqttServer);
    m_mqttClient->connectToHost();
    qDebug() << __PRETTY_FUNCTION__ << "Connecting to" << m_mqttClient->host() << "with name" << m_hostName;
    m_sbConnected->setText(QString("Connecting to %1").arg(m_mqttServer.toString()));
}

void MQTTSnoopWindow::connectAddressInput(QString address)
{
    m_mqttServer.setAddress(address);
    if (!m_mqttServer.isNull())
        connectAddressInput();
}

