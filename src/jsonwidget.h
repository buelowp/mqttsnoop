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

#pragma once

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

class JsonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JsonWidget(QString topic, QWidget *parent = nullptr);
    ~JsonWidget() override;
    
    void addJson(QJsonDocument &json);
    QString topic() { return m_topicString; }
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected slots:
    void showEvent(QShowEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    
private:
    void populateNewWidget(int localX, QJsonObject obj);
    void updateWidget(int localX, int i, QJsonObject obj);
    
    QLabel *m_topic;
    QString m_topicString;
    uint32_t m_y;
    uint32_t m_width;
    uint32_t m_origin;
    bool m_populated;
};

