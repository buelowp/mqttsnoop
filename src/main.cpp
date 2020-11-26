#include "mqttsnoop.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MQTTSnoopWindow w;
    w.setGeometry(100, 100, 1500, 1000);
    w.show();

    return app.exec();
}

