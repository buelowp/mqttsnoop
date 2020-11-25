#include "mqttsnoop.h"
#include "ui_mqttsnoop.h"

mqttsnoop::mqttsnoop(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::mqttsnoop)
{
    m_ui->setupUi(this);
}

mqttsnoop::~mqttsnoop() = default;
