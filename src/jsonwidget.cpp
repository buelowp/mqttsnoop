/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2020  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jsonwidget.h"

JsonWidget::JsonWidget(QString topic, QWidget *parent) : QWidget(parent), m_topicString(topic), m_y(0), m_width(0), m_populated(false)
{
    m_topic = new QLabel(topic, this);
    m_topic->move(5, m_y + 10);
    m_origin = m_topic->height();
    m_y = m_topic->height() + 10;
//    setStyleSheet("background-color:white;");
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
}

JsonWidget::~JsonWidget() = default;

QSize JsonWidget::minimumSizeHint() const
{
    return QSize(m_width, m_y);
}

QSize JsonWidget::sizeHint() const
{
    return QSize(m_width, m_y);
}

void JsonWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);

    painter.drawRoundedRect(0, 5, width() - 7, height() - 7, 3, 3);
    QWidget::paintEvent(e);
}

void JsonWidget::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
}

void JsonWidget::populateNewWidget(int localX, QJsonObject obj)
{
    localX += 10;
    
    foreach(const QString& key, obj.keys()) {
        QJsonValue value = obj.value(key);
            QLabel *label;
            switch (value.type()) {
                case QJsonValue::Bool:
                    label = new QLabel(QString("%1 : %2").arg(key).arg(value.toBool()), this);
                    label->move(localX, m_y);
                    label->setObjectName(key);
                    m_y += label->height();
                    break;
                case QJsonValue::Double:
                    label = new QLabel(QString("%1 : %2").arg(key).arg(value.toDouble()), this);
                    label->move(localX, m_y);
                    label->setObjectName(key);
                    m_y += label->height();
                    break;
                case QJsonValue::String:
                    label = new QLabel(QString("%1 : %2").arg(key).arg(value.toString()), this);
                    label->move(localX, m_y);
                    label->setObjectName(key);
                    m_y += label->height();
                    break;
                case QJsonValue::Array:
                    label = new QLabel(QString("%1 : %2").arg(key).arg("NOT DONE YET"), this);
                    label->move(localX, m_y);
                    m_y += label->height();
                    label->setObjectName(key);
                    break;
                case QJsonValue::Object:
                    label = new QLabel(QString("%1").arg(key), this);
                    label->move(localX, m_y);
                    m_y += label->height();
                    label->setObjectName("object");
                    populateNewWidget(localX, value.toObject());
                    break;
                case QJsonValue::Undefined:
                case QJsonValue::Null:
                    label = new QLabel(QString("%1 : %2").arg(key).arg("UNDEFINED"), this);
                    label->move(localX, m_y);
                    m_y += label->height();
                    label->setObjectName(key);
                    break;
            }
            uint32_t newWidth = label->width() + localX;
            if (newWidth > m_width)
                m_width = newWidth;
    }
}

void JsonWidget::updateWidget(int localX, int i, QJsonObject obj)
{
    QList<QLabel*> labels = findChildren<QLabel*>();
    bool moveon = false;
    int index = i + 1;
    localX += 10;
    
    foreach(const QString &key, obj.keys()) {
        QJsonValue value = obj.value(key);
        for (auto label : labels) {
            moveon = false;
            switch (value.type()) {
                case QJsonValue::Bool:
                    if (label->objectName() == key) {
                        label->setText(QString("%1 : %2").arg(key).arg(value.toBool()));
                        moveon = true;
                    }
                    break;
                case QJsonValue::Double:
                    if (label->objectName() == key) {
                        label->setText(QString("%1 : %2").arg(key).arg(value.toDouble()));
                        moveon = true;
                    }
                    break;
                case QJsonValue::String:
                    if (label->objectName() == key) {
                        label->setText(QString("%1 : %2").arg(key).arg(value.toString()));
                        moveon = true;
                    }
                    break;
                case QJsonValue::Array:
                    if (label->objectName() == key) {
                        label->setText("ARRAY UNFINISHED");
                        moveon = true;
                    }
                    break;
                case QJsonValue::Object:
                    updateWidget(localX, index, value.toObject());
                    moveon = true;
                    break;
                case QJsonValue::Undefined:
                case QJsonValue::Null:
                    if (label->objectName() == key) {
                        label->setText("UNDEFINED");
                        moveon = true;
                    }
                    break;
            }
            if (moveon)
                break;
        }
    }
}

void JsonWidget::addJson(QJsonDocument& doc)
{
    if (doc.isEmpty() || doc.isNull()) {
        qWarning() << __PRETTY_FUNCTION__ << ": Bad JSON document passed in";
        return;
    }
    
    QJsonObject json = doc.object();
    if (m_populated) {
        updateWidget(5, 0, json);
    }
    else {
        populateNewWidget(5, json);
        m_populated = true;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    m_topic->setText(QString("Topic: %1 [%2]").arg(m_topicString).arg(now.toString("dd-MM-yyyy h:mm:ss ap")));
}
