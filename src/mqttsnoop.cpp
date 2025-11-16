#include "mqttsnoop.h"

QString tabStyle(" \
    QTabBar::tab { \
        background-color: qlineargradient(x1:0.5, y1:1, x2:0.5, y2:0, stop:0 rgb(253,250,250), stop:0.2 rgb(253,250,250), stop:1 rgb(255,249,234)); \
 \
        border-top-left-radius: 7px; \
        border-top-right-radius: 7px; \
\
        min-width: 8ex; \
        padding: 5px; \
    } \
\
    QTabBar::tab:selected { \
        background-color: rgb(200,200,255); \
    } \
\
    QTabBar::tab:!selected { \
        margin-top: 5px; \
        background: qlineargradient(x1:0.5, y1:1, x2:0.5, y2:0, stop:0 rgb(253,250,250), stop:0.2 rgb(253,250,250), stop:1 rgb(250,244,229)); \
        color: rgb(93, 109, 109) \
    }"
);

MQTTSnoopWindow::MQTTSnoopWindow(QWidget *parent) : QMainWindow(parent), m_topics(0), m_mpm(0)
{
    m_tabWidget = new QTabWidget();
    
    m_isSSL = false;
    m_mqttClient = new QMqttClient();
    m_hostName = QString("%1-%2").arg(QHostInfo::localHostName()).arg(QRandomGenerator::global()->generate());
    m_mqttClient->setClientId(m_hostName);

    m_addressDialog = new AddressDialog();
    m_addressDialog->setWindowModality(Qt::WindowModal);
    connect(m_addressDialog, &AddressDialog::newCACertificate, this, &MQTTSnoopWindow::newCACert);
    connect(m_addressDialog, &AddressDialog::newClientCertificate, this, &MQTTSnoopWindow::newClientCert);
    connect(m_addressDialog, &AddressDialog::newClientKey, this, &MQTTSnoopWindow::newClientKey);

    m_eventCounter = new EventCounter();
    connect(m_eventCounter, SIGNAL(minuteEvents(uint64_t)), this, SLOT(displayMPM(uint64_t)));
    
    connect(m_mqttClient, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_mqttClient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_mqttClient, &QMqttClient::errorChanged, this, &MQTTSnoopWindow::error);
    connect(m_mqttClient, &QMqttClient::messageReceived, this, &MQTTSnoopWindow::received);
    connect(m_addressDialog, &AddressDialog::newServerValue, this, &MQTTSnoopWindow::connectAddressInput);
    
    setCentralWidget(m_tabWidget);
    
    buildStatusBar();
    buildMenuBar();

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    m_currentTopic = "#";

    m_tabWidget->setStyleSheet(tabStyle);
    QSettings settings("home", "mqttsnoop");
    if (settings.contains("mqttserver")) {
        m_addressDialog->setServerText(settings.value("mqttserver").toString());
    }
    if (settings.contains("mqttport")) {
        m_addressDialog->setServerPort(settings.value("mqttport").toString());
    }
    if (settings.contains("clientkey")) {
        m_addressDialog->setClientKeyFile(settings.value("clientkey").toString());
        newClientKey(settings.value("clientkey").toString());
    }
    if (settings.contains("cacert")) {
        m_addressDialog->setCACertFile(settings.value("cacert").toString());
        newCACert(settings.value("cacert").toString());
    }
    if (settings.contains("clientcert")) {
        m_addressDialog->setClientKeyFile(settings.value("clientcert").toString());
        newClientCert(settings.value("clientcert").toString());
    }
}

MQTTSnoopWindow::~MQTTSnoopWindow()
{
    m_mqttClient->unsubscribe(m_currentTopic);
    m_mqttClient->disconnectFromHost();
}

void MQTTSnoopWindow::newCACert(QString cert)
{
    QSettings settings("home", "mqttsnoop");
    QFile certFile(cert);

    if (certFile.open(QIODevice::ReadOnly)) {
        QSslCertificate c(&certFile, QSsl::Pem);
        m_sslConfig.addCaCertificate(c);
        settings.setValue("cacert", cert);
    }
    else {
        qDebug() << "Error opening CA cert" << cert << ":" << certFile.errorString();
    }
}

void MQTTSnoopWindow::newClientCert(QString cert)
{
    QSettings settings("home", "mqttsnoop");
    QFile certFile(cert);

    if (certFile.open(QIODevice::ReadOnly)) {
        QSslCertificate c(&certFile, QSsl::Pem);
        m_sslConfig.setLocalCertificate(c);
        settings.setValue("clientcert", cert);
    }
    else {
        qDebug() << "Error opening client cert" << cert << ":" << certFile.errorString();
    }
}

void MQTTSnoopWindow::newClientKey(QString key)
{
    QSettings settings("home", "mqttsnoop");
    QFile keyFile(key);

    if (keyFile.open(QIODevice::ReadOnly)) {
        QSslKey privateKey(&keyFile, QSsl::Rsa);
        m_sslConfig.setPrivateKey(privateKey);
        settings.setValue("clientkey", key);
    }
    else {
        qDebug() << "Error opening keyfile" << key << ":" << keyFile.errorString();
    }
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
    m_subscribeAct = new QAction(subscribeIcon, tr("&Subscribe"), this);
    m_subscribeAct->setStatusTip(tr("Subscribe"));
    connect(m_subscribeAct, &QAction::triggered, this, &MQTTSnoopWindow::menuSubscribe);
    netToolBar->addAction(m_subscribeAct);

    m_subscribeAct->setDisabled(true);
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

    if (ok && text.size()) {
        m_mqttClient->unsubscribe(m_currentTopic);
        m_subscription = m_mqttClient->subscribe(QMqttTopicFilter(text));
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

    m_sbTopicsReceived = new QLabel("Topics Received: 0");
    m_sbTopicsReceived->setAlignment(Qt::AlignCenter);
    m_sbTopicsReceived->setAutoFillBackground(true);

    m_sbMessagesPerMinute = new QLabel("Messages Per Minute: 0");
    m_sbMessagesPerMinute->setAlignment(Qt::AlignRight);
    m_sbMessagesPerMinute->setAutoFillBackground(true);
    
    m_sbCurrentTopic = new QLabel(QString("Topic: ") + m_currentTopic);
    m_sbCurrentTopic->setAlignment(Qt::AlignCenter);
    m_sbCurrentTopic->setAutoFillBackground(true);

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
    m_addressDialog->move(m_addressDialog->geometry().x() + (e->pos().x() - e->oldPos().x()), m_addressDialog->geometry().y() + (e->pos().y() - e->oldPos().y()));
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
}

void MQTTSnoopWindow::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
    
    m_sbConnected->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_sbTopicsReceived->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_sbCurrentTopic->setMinimumWidth((statusBar()->width() / 4) - 15);
    m_sbMessagesPerMinute->setMinimumWidth((statusBar()->width() / 4) - 15);
}

void MQTTSnoopWindow::newTab(QString topic, QString localTopic, QJsonDocument json)
{
    QMutexLocker locker(&m_newTabMutex);
    QWidget *parentWidget = new QWidget(m_tabWidget);
    QHBoxLayout *parentLayout = new QHBoxLayout();
    parentWidget->setLayout(parentLayout);
    QScrollArea *parentScroll = new QScrollArea();
    parentLayout->addWidget(parentScroll);
    TabWidget *tab = new TabWidget();
    parentScroll->setWidgetResizable(true);
    parentScroll->setWidget(tab);
    
    tab->addJson(topic, json);
    m_tabWidget->addTab(parentWidget, localTopic);
    m_topics++;
}

void MQTTSnoopWindow::updateTab(QString topic, QString localTopic, QJsonDocument doc, TabWidget* tab)
{
    QMutexLocker locker(&m_updateTabMutex);
    QString parentTopic = localTopic;
    if (tab->addJson(topic, doc))
        m_topics++;
}

void MQTTSnoopWindow::connected()
{
    if (m_isSSL) {
        m_sbConnected->setText(QString("Connected: mqtts://%1:%2").arg(m_mqttClient->hostname()).arg(m_mqttClient->port()));
    }
    else {
        m_sbConnected->setText(QString("Connected: mqtt://%1:%2").arg(m_mqttClient->hostname()).arg(m_mqttClient->port()));
    }
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT connected to" << m_mqttClient->hostname();
    m_subscribeAct->setDisabled(false);
}

void MQTTSnoopWindow::disconnected()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Disconnected from MQTT server";
    m_sbConnected->setText(QString("Disconnected"));
    m_subscribeAct->setDisabled(true);
}

void MQTTSnoopWindow::error(QMqttClient::ClientError error)
{
    qDebug() << __PRETTY_FUNCTION__ << ": MQTT error:" << error;
    m_sbConnected->setText(QString("MQTT Error: %1").arg(error));
    if (error == QMqttClient::ServerUnavailable || error == QMqttClient::NotAuthorized) {
        m_mqttClient->disconnectFromHost();
        m_addressDialog->show();
    }
}

bool MQTTSnoopWindow::compareTabText(QString tabText, QString text)
{
    return text == tabText.remove(QChar('&'));
}

void MQTTSnoopWindow::received(const QByteArray &message, const QMqttTopicName &topic)
{
    QString parentTopic = topic.name().left(topic.name().indexOf(QString("/")));
    QJsonDocument json = QJsonDocument::fromJson(message);
    int i = 0;
    
    if (json.isNull() || json.isEmpty()) {
        return;
    }
    m_eventCounter->bump();

    for (i = 0; i < m_tabWidget->count(); i++) {
        if (compareTabText(m_tabWidget->tabText(i), parentTopic)) {
            QWidget *top = static_cast<QWidget*>(m_tabWidget->widget(i));
            QHBoxLayout *topLayout = static_cast<QHBoxLayout*>(top->layout());
            QScrollArea *scroller = static_cast<QScrollArea*>(topLayout->itemAt(0)->widget());
            TabWidget *widget = static_cast<TabWidget*>(scroller->widget());
            updateTab(topic.name(), parentTopic, json, widget);
            m_sbTopicsReceived->setText(QString("Topics Received: %1").arg(m_topics));
            return;
        }
    }

    newTab(topic.name(), parentTopic, json);
    
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

void MQTTSnoopWindow::connectAddressInput(QString address, int port)
{
    m_mqttServer.setAddress(address);
    m_mqttClient->setPort(port);
    m_mqttClient->setHostname(address);
    if (m_sslConfig.isNull()) {
        m_isSSL = false;
        qDebug() << "Connecting to mqtt://" << m_mqttClient->hostname() << ":" << m_mqttClient->port();
        m_mqttClient->connectToHost();
    }
    else {
        qDebug() << "Connecting to mqtts://" << m_mqttClient->hostname() << ":" << m_mqttClient->port();
        m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        m_mqttClient->connectToHostEncrypted(m_sslConfig);
        m_isSSL = true;
    }
    m_sbConnected->setText(QString("Connecting to %1").arg(m_mqttServer.toString()));
}
