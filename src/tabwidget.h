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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "jsonwidget.h"

/**
 * @todo write docs
 */
class TabWidget : public QWidget
{
    Q_OBJECT

public:
    TabWidget(QWidget *parent = nullptr);
    ~TabWidget() override;
    
    bool addJson(QString topic, QJsonDocument doc);
    
private:
    bool addNewWidget(int row, int col, QString topic, QJsonDocument doc);
    
    QGridLayout *m_layout;
};

#endif // TABWIDGET_H
