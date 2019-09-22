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
#include "../windows/e_listening_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QDebug>
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../control/messagebox.h"

namespace state {

Listening::Listening(StateContext * context) :
    State(context, STATE::LISTENING )
{
    // Let's establish connectoins at the beginning

    QObject::connect(context->wallet, &wallet::Wallet::onMwcMqListenerStatus,
                                         this, &Listening::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onKeybaseListenerStatus,
                                         this, &Listening::onKeybaseListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onListeningStartResults,
                                         this, &Listening::onListeningStartResults, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onListeningStopResult,
                                         this, &Listening::onListeningStopResult, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onMwcAddressWithIndex,
                                         this, &Listening::onMwcAddressWithIndex, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onListenerMqCollision,
                     this, &Listening::onListenerMqCollision, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onNewNotificationMessage,
                     this, &Listening::onNewNotificationMessage, Qt::QueuedConnection);
}

Listening::~Listening() {
    // disconnect will happen automatically
}

NextStateRespond Listening::execute() {
    if ( context->appContext->getActiveWndState() != STATE::LISTENING )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        context->wallet->getMwcBoxAddress();
        // will get result later and will update the window

        wnd = (wnd::Listening*) context->wndManager->switchToWindowEx( mwc::PAGE_E_LISTENING,
                new wnd::Listening( context->wndManager->getInWndParent(), this,
                       context->wallet->getListenerStatus(),
                       context->wallet->getListenerStartState(),
                       context->wallet->getLastKnownMwcBoxAddress(), -1));
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Listening::triggerMwcStartState() {
    QPair<bool,bool> lsnStatus = context->wallet->getListenerStartState();
    if ( !lsnStatus.first ) {
        context->wallet->listeningStart(true, false, false);
    }
    else {
        context->wallet->listeningStop(true, false);
    }
}

void Listening::requestNextMwcMqAddress() {
    context->wallet->nextBoxAddress();
}

void Listening::requestNextMwcMqAddressForIndex(int idx) {
    context->wallet->changeMwcBoxAddress(idx);
}

void Listening::triggerKeybaseStartState() {
    QPair<bool,bool> lsnStatus = context->wallet->getListenerStartState();
    qDebug() << "lsnStatus: " << lsnStatus.first << " " << lsnStatus.second;
    if ( !lsnStatus.second ) {
        context->wallet->listeningStart(false, true, false);
    }
    else {
        context->wallet->listeningStop(false, true);
    }
}

void Listening::onMwcMqListenerStatus(bool online) {
    Q_UNUSED(online)
    if (wnd) {
        wnd->updateStatuses(context->wallet->getListenerStatus(), context->wallet->getListenerStartState() );
    }
}
void Listening::onKeybaseListenerStatus(bool online) {
    Q_UNUSED(online)
    if (wnd) {
        wnd->updateStatuses(context->wallet->getListenerStatus(), context->wallet->getListenerStartState() );
    }
}

// Listening, you will not be able to get a results
void Listening::onListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                               QStringList errorMessages, bool initialStart ) // error messages, if get some
{
    Q_UNUSED(mqTry)
    Q_UNUSED(kbTry)
    if (wnd) {
        wnd->updateStatuses(context->wallet->getListenerStatus(), context->wallet->getListenerStartState() );
    }

    if (wnd && !errorMessages.empty() && !initialStart ) {
        QString msg;
        for (auto & s : errorMessages)
            msg += s + '\n';

        if (kbTry) {

            wallet::WalletConfig cfg = context->wallet->getWalletConfig();
            if (!cfg.keyBasePath.isEmpty() ) {
                msg += "\nYour current keybase path:\n" + cfg.keyBasePath + "\nThe keybase path can be changed at 'Wallet Configuration' page.";
            }
        }

        if (msg.contains("mwcmq") && msg.contains("already started") ) {
            msg = QString("MWC MQ") + (config::getUseMwcMqS() ? "S" : "") + " listener is running, but it lost connection and trying to reconnect in background to " +
                    context->wallet->getWalletConfig().getMwcMqHostFull() +".\nPlease check your network connection";
        }

        wnd->showMessage("Start listening Error", msg);
    }
}

void Listening::onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                            QStringList errorMessages ) {
    Q_UNUSED(mqTry)
    Q_UNUSED(kbTry)
    if (wnd) {
        wnd->updateStatuses(context->wallet->getListenerStatus(), context->wallet->getListenerStartState() );
    }

    if (wnd && !errorMessages.empty()) {
        QString msg;
        for (auto & s : errorMessages)
            msg += s + "/n";
        wnd->showMessage("Start listening Error", msg);
    }
}

void Listening::onMwcAddressWithIndex(QString mwcAddress, int idx) {
    if (wnd) {
        wnd->updateMwcMqAddress(mwcAddress, idx);
    }
}

void Listening::onListenerMqCollision() {
    control::MessageBox::message(nullptr, "MWC MQS new login detected", "New login to MWC MQS detected. Only one instance of your wallet can be connected to MWC MQS.\nListener is stopped. You can activate listener from 'Listening' page.");
}

// Looking for "Failed to start mwcmqs subscriber. Error connecting to mqs.mwc.mw:443"
void Listening::onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message) {
    Q_UNUSED(level);
    // We are not relying to the window, but checking if it is active
    if ( wnd!= nullptr && message.contains("Failed to start mwcmqs subscriber") ) {
        wnd->showMessage("Start listening Error", message);
    }
}


}
