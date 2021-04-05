// Copyright 2021 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "TimerThread.h"

//////////////////////////////////////////////////////////////
// Timer thread.

namespace core {

TimerThread::TimerThread(QObject *parent, long _interval) : QThread(parent), interval(_interval) { alive = true; }

TimerThread::~TimerThread() {}

void TimerThread::stop() {
    alive = false;
}

void TimerThread::run() {
    int k = 0;
    while (alive) {
        QThread::msleep(interval / 10);
        if (k > 10) {
            emit onTimerEvent();
            k = 0;
        }
        k++;
    }
}

}