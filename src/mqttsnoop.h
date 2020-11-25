#ifndef MQTTSNOOP_H
#define MQTTSNOOP_H

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui {
class mqttsnoop;
}

class mqttsnoop : public QMainWindow
{
    Q_OBJECT

public:
    explicit mqttsnoop(QWidget *parent = nullptr);
    ~mqttsnoop() override;

private:
    QScopedPointer<Ui::mqttsnoop> m_ui;
};

#endif // MQTTSNOOP_H
