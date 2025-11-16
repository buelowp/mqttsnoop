#pragma once

#include <QtWidgets/QtWidgets>
#include <QtCore/QtCore>

class QLabelEvent : public QLabel
{
    Q_OBJECT

public:
    explicit QLabelEvent(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~QLabelEvent();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);

};
