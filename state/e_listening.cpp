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

namespace state {

Listening::Listening(StateContext * context) :
    State(context, STATE::LISTENING )
{
    // Let's establish connectoins at the beginning

    QObject::connect(context->wallet, &wallet::Wallet::onListeningStartResults,
                                         this, &Listening::onListeningStartResults, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onListeningStopResult,
                                         this, &Listening::onListeningStopResult, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onListenerMqCollision,
                     this, &Listening::onListenerMqCollision, Qt::QueuedConnection);

    QObject::connect(notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                     this, &Listening::onNewNotificationMessage, Qt::QueuedConnection);
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


// Listening, you will not be able to get a results
void Listening::onListeningStartResults( bool mqTry, bool kbTry, bool torTry, // what we try to start
                               QStringList errorMessages, bool initialStart ) // error messages, if get some
{
    Q_UNUSED(mqTry)
    Q_UNUSED(kbTry)
    Q_UNUSED(torTry)

    if ( !errorMessages.empty() && !initialStart ) {
        QString msg;
        for (auto & s : errorMessages)
            msg += s + '\n';

        if (kbTry) {

            const wallet::WalletConfig & cfg = context->wallet->getWalletConfig();
            if (!cfg.keyBasePath.isEmpty() ) {
                msg += "\nYour current keybase path:\n" + cfg.keyBasePath + "\nThe keybase path can be changed at 'Wallet Configuration' page.";
            }
        }

        if (msg.contains("mwcmq") && msg.contains("already started") ) {
            msg = "MWC MQS listener is running, but it lost connection and trying to reconnect in background to " +
                    context->wallet->getWalletConfig().getMwcMqHostFull() +".\nPlease check your network connection";
        }

        core::getWndManager()->messageTextDlg("Start listening Error", msg);
    }
}

void Listening::onListeningStopResult(bool mqTry, bool kbTry, bool torTry, // what we try to stop
                            QStringList errorMessages ) {
    Q_UNUSED(mqTry)
    Q_UNUSED(kbTry)
    Q_UNUSED(torTry)

    if (!errorMessages.empty()) {
        QString msg;
        for (auto & s : errorMessages)
            msg += s + "\n";

        core::getWndManager()->messageTextDlg("Stop listening Error", msg);
    }
}

void Listening::onListenerMqCollision() {
    core::getWndManager()->messageTextDlg("MWC MQS new login detected",
            "New login to MWC MQS detected. Only one instance of your wallet can be connected to MWC MQS.\n"
            "Listener is stopped. You can activate listener from 'Listening' page.");
}

// Looking for "Failed to start mwcmqs subscriber. Error connecting to mqs.mwc.mw:443"
void Listening::onNewNotificationMessage(notify::MESSAGE_LEVEL level, QString message) {
    Q_UNUSED(level);
    // We are not relying to the window, but checking if it is active
    if ( message.contains("Failed to start mwcmqs subscriber") ) {
        if (lastShownErrorMessage!=message) {
            core::getWndManager()->messageTextDlg("Start listening Error", message);
            lastShownErrorMessage=message;
        }
    }
}


}
