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

JsonWidget::JsonWidget(QString topic, QWidget *parent) : QWidget(parent), m_topicString(topic), m_populated(false)
{
    m_layout = new QVBoxLayout();
    m_topic = new QLabel(topic);
    m_data = new QJsonModel();
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(210, 210, 210));
    setAutoFillBackground(true);
    setPalette(pal);

    m_layout->addWidget(m_topic);
    setLayout(m_layout);

    m_minHeight = 0;
}

JsonWidget::~JsonWidget() = default;

void JsonWidget::populateNewWidget(QJsonDocument obj)
{
    QTreeView * view = new QTreeView();
    m_data->load(obj);
    view->setModel(m_data);
    setMinimumHeight(view->height());
    m_layout->addWidget(view);
}

void JsonWidget::updateWidget(QJsonDocument obj)
{
    if (m_populated) {
        m_data->load(obj);
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
        updateWidget(doc);
    }
    else {
        populateNewWidget(doc);
        m_populated = true;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    m_topic->setText(QString("Topic: %1 [%2]").arg(m_topicString).arg(now.toString("dd-MM-yyyy h:mm:ss ap")));
}
