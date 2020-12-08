#include "mqttsnoop.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":images/appicon.png"));
    
    MQTTSnoopWindow w;
    w.setGeometry(100, 100, 1500, 1000);
    w.show();

    return app.exec();
}

