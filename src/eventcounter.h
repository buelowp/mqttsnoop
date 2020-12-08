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

#ifndef EVENTCOUNTER_H
#define EVENTCOUNTER_H

#include <QtCore/QtCore>

/**
 * @todo write docs
 */
class EventCounter : public QObject
{
    Q_OBJECT

public:
    explicit EventCounter(QObject *parent = nullptr);
    ~EventCounter();
    
    void bump();
    
protected slots:
    void timeout();
    
signals:
    void secondEvents(uint64_t e);
    void minuteEvents(uint64_t e);
    void hourEvents(uint64_t e);
    
private:
    uint64_t minuteEventCount();
    
    QTimer *m_eventTimer;
    QMap<int, int> m_events;
    
    uint64_t m_eventsTotal;
    uint64_t m_eventsPerHour;
    uint64_t m_eventsPerMinute;
    uint64_t m_eventsPerSecond;
    int m_lastSecond;
};

#endif // EVENTCOUNTER_H
