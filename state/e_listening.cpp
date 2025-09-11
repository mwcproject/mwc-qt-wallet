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

#include "e_listening.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QDebug>
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/notification_b.h"

namespace state {

Listening::Listening(StateContext * context) :
    State(context, STATE::LISTENING )
{
    // Let's establish connectoins at the beginning
    QObject::connect(context->wallet, &wallet::Wallet::onListenerMqCollision,
                 this, &Listening::onListenerMqCollision, Qt::QueuedConnection);
}

Listening::~Listening() {
    // disconnect will happen automatically
}

NextStateRespond Listening::execute() {
    if ( context->appContext->getActiveWndState() != STATE::LISTENING )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    // will get result later and will update the window
    core::getWndManager()->pageListening();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Listening::onListenerMqCollision() {
    core::getWndManager()->messageTextDlg("MWC MQS new login detected",
            "New login to MWC MQS detected. Only one instance of your wallet can be connected to MWC MQS.\n"
            "Listener is stopped. Please close your other instance and restart this wallet to start use MWCMQS.");
}


}
