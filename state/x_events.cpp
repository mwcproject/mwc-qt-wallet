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
#include "../windows/x_events_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"

namespace state {

Events::Events(StateContext * context):
    State(context, STATE::EVENTS)
{
    QObject::connect( context->wallet, &wallet::Wallet::onNewNotificationMessage,
            this, &Events::onNewNotificationMessage, Qt::QueuedConnection );
}

Events::~Events() {}

NextStateRespond Events::execute() {
    if (context->appContext->getActiveWndState() != STATE::EVENTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    wnd = (wnd::Events*) context->wndManager->switchToWindowEx( mwc::PAGE_X_EVENTS,
                new wnd::Events( context->wndManager->getInWndParent(), this ) );

    emit updateNonShownWarnings(false);

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Events::deleteEventsWnd(wnd::Events * w)
{
    if (w==wnd) {
        wnd=nullptr;
        messageWaterMark = QDateTime::currentMSecsSinceEpoch();
    }
}

QVector<int> Events::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("NotifTblColWidth");
}

void Events::updateColumnsWidhts(const QVector<int> & widths) {
    context->appContext->updateIntVectorFor("NotifTblColWidth", widths);

}

QVector<wallet::WalletNotificationMessages> Events::getWalletNotificationMessages() {
    return context->wallet->getWalletNotificationMessages();
}

void Events::onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message) {
    Q_UNUSED(message);

    if (wnd!= nullptr) {
        wnd->updateShowMessages();
    }
    else {
        if ( wallet::WalletNotificationMessages::isCritical(level) )
            emit updateNonShownWarnings(true);
    }
}

// Check if some error/warnings need to be shown
bool Events::hasNonShownWarnings() const {
    QVector<wallet::WalletNotificationMessages> msgs = context->wallet->getWalletNotificationMessages();

    for ( int i = msgs.size()-1; i>=0; i-- ) {
        if (msgs[i].time.toMSecsSinceEpoch() <= messageWaterMark )
            return false;

        if ( msgs[i].isCritical() )
            return true;
    }

    return false;
}


}
