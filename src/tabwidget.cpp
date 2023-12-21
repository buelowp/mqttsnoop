/*
 * Copyright 2020 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tabwidget.h"

TabWidget::TabWidget(QWidget *parent) : QWidget(parent)
{
    m_layout = new QGridLayout();
    setLayout(m_layout);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(190, 190, 190));
    setAutoFillBackground(true);
    setPalette(pal);
    m_layout->setSpacing(5);
}

TabWidget::~TabWidget() = default;

bool TabWidget::addNewWidget(int row, int col, QString topic, QJsonDocument doc)
{
    JsonWidget *widget = new JsonWidget(topic);
    widget->addJson(doc);
    m_layout->addWidget(widget, row, col);
    return true;
}

bool TabWidget::addJson(QString topic, QJsonDocument doc)
{
    for (int i = 0; i <= m_layout->rowCount(); i++) {
        for (int j = 0; j < 2; j++) {
            QLayoutItem *item = m_layout->itemAtPosition(i, j);
            if (item == nullptr) {
//                 qDebug() << __PRETTY_FUNCTION__ << "No widget at row" << i << ", column" << j << ", creating a new topic [" << topic << "]";
                return addNewWidget(i, j, topic, doc);
            }
            
            JsonWidget *jw = static_cast<JsonWidget*>(item->widget());
            if (jw->topic() == topic) {
//                 qDebug() << __PRETTY_FUNCTION__ << "Updating existing [" << topic << "] at row" << i << ", column" << j;
                jw->addJson(doc);
                return false;
            }
        }
    }
    
//     qDebug() << __PRETTY_FUNCTION__ << "Did not find [" << topic << "] in the widget set";
    return false;
}
