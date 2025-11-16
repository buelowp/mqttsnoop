#include "qlabelevent.h"

QLabelEvent::QLabelEvent(QWidget* parent, Qt::WindowFlags f)
: QLabel(parent)
{

}

QLabelEvent::~QLabelEvent() {}

void QLabelEvent::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}
