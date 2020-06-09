// Copyright 2020 The MWC Developers
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

#include "x_events_b.h"
#include "../BridgeManager.h"
#include "../../state/state.h"
#include "../../state/statemachine.h"
#include "../../state/x_events.h"

namespace bridge {
// Home state accessor
static state::Events * getEvents() {return (state::Events *) state::getState(state::STATE::EVENTS); }

Events::Events(QObject *parent) : QObject(parent) {
    getBridgeManager()->addEvents(this);
}

Events::~Events() {
    getBridgeManager()->removeEvents(this);
}

// New message arrive, need to emit the signal about that
void Events::updateShowMessages() {
    emit sgnUpdateShowMessages();
}

// Update status that New Warning message has arrived
void Events::updateNonShownWarnings(bool hasNonShownWarns) {
    emit sgnUpdateNonShownWarnings(hasNonShownWarns);
}

// Request is any new critical warnings arrived
bool Events::hasNonShownWarnings() {
    return getEvents()->hasNonShownWarnings();
}

// Notify thet events window is closed. Last seen even waterwart will be updated
void Events::eventsWndIsDeleted() {
    return getEvents()->eventsWndIsDeleted();
}

QVector<QString> Events::getWalletNotificationMessages() {
    QVector<notify::NotificationMessage> messages = getEvents()->getWalletNotificationMessages();
    QVector<QString> res;
    for (auto & msg : messages) {
        res.push_back(msg.time.toString("HH:mm:ss"));
        res.push_back(msg.time.toString("ddd MMMM d yyyy HH:mm:ss"));
        res.push_back(msg.getLevelStr());
        res.push_back(msg.getLevelLongStr());
        res.push_back(msg.message);
    }
    return res;
}




}