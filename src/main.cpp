#include "mqttsnoop.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    mqttsnoop w;
    w.show();

    return app.exec();
}

