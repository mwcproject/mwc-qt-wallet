// Copyright 2019 The MWC Developers
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

#include "TaskErrWrnInfoListener.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../core/Notification.h"

namespace wallet {

bool TaskErrWrnInfoListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
        case S_READY:
            if (!walletIsReady)
                qDebug() << "TaskErrWrnInfoListener::processTask switch to ready state";
            walletIsReady = true;
            return false;
        case S_ERROR:
        case S_GENERIC_ERROR: {
            if (walletIsReady) {
                qDebug() << "TaskErrWrnInfoListener::processTask with events: " << printEvents(events);
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL,
                                                     evt.message);
            }
            return true;
        }
        case S_GENERIC_WARNING: {
            if (walletIsReady) {
                qDebug() << "TaskErrWrnInfoListener::processTask with events: " << printEvents(events);
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING,
                                                     evt.message);
            }
            return true;
        }
        case S_GENERIC_INFO: {
            if (walletIsReady) {
                qDebug() << "TaskErrWrnInfoListener::processTask with events: " << printEvents(events);
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO,
                                                     evt.message);
            }
            return true;
        }
        default:
            return false;
    }
}

}
