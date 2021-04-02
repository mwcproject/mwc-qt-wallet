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

#include "x_events.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/x_events_b.h"

namespace state {

Events::Events(StateContext * context):
    State(context, STATE::EVENTS)
{
    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
            this, &Events::onNewNotificationMessage, Qt::QueuedConnection );
}

Events::~Events() {}

NextStateRespond Events::execute() {
    if (context->appContext->getActiveWndState() != STATE::EVENTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    core::getWndManager()->pageEvents();

    for (auto b : bridge::getBridgeManager()->getEvents())
        b->updateNonShownWarnings(false);

    messageWaterMark = QDateTime::currentMSecsSinceEpoch();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Events::eventsWndIsDeleted()
{
    messageWaterMark = QDateTime::currentMSecsSinceEpoch();
}

// Historical design. UI can call this method now, but not in the past
QVector<notify::NotificationMessage> Events::getWalletNotificationMessages() {
    return notify::getNotificationMessages();
}

void Events::onNewNotificationMessage(bridge::MESSAGE_LEVEL  level, QString message) {
    Q_UNUSED(message);

    for (auto b : bridge::getBridgeManager()->getEvents())
        b->updateShowMessages();

    if ( state::getStateMachine()->getCurrentStateId() != STATE::EVENTS ) {
        if ( notify::NotificationMessage::isCritical(level) ) {
            for (auto b : bridge::getBridgeManager()->getEvents())
                b->updateNonShownWarnings(true);
        }
    }
}

// Check if some error/warnings need to be shown
bool Events::hasNonShownWarnings() const {
    QVector<notify::NotificationMessage> msgs = notify::getNotificationMessages();

    for ( int i = msgs.size()-1; i>=0; i-- ) {
        if (msgs[i].time.toMSecsSinceEpoch() <= messageWaterMark )
            return false;

        if ( msgs[i].isCritical() )
            return true;
    }

    return false;
}


}
