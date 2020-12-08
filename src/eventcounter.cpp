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

#include "eventcounter.h"

EventCounter::EventCounter(QObject *parent) : QObject(parent)
{
    m_lastSecond = 0;
    
    m_eventTimer = new QTimer();
    m_eventTimer->setInterval(1000);
    connect(m_eventTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_eventTimer->start();
    
    for (int i = 0; i < 60; i++) {
        m_events[i] = 0;
    }
}

EventCounter::~EventCounter()
{
    m_eventTimer->stop();
}

void EventCounter::bump()
{
    m_events[m_lastSecond]++;
    emit minuteEvents(minuteEventCount());
}

uint64_t EventCounter::minuteEventCount()
{
    uint64_t count = 0;
    
    for (int i = 0; i < 60; i++) {
        count += m_events[i];
    }
    return count;
}

void EventCounter::timeout()
{
    m_lastSecond++;
    if (m_lastSecond == 60)
        m_lastSecond = 0;    

    m_events[m_lastSecond] = 0;
}

