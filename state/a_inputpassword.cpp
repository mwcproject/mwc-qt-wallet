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

#include "state/a_inputpassword.h"
#include "windows/a_inputpassword_w.h"
#include "../wallet/wallet.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include <QCoreApplication>

namespace state {

InputPassword::InputPassword( StateContext * _context) :
    State(_context, STATE::INPUT_PASSWORD)
{
    // Result of the login
    QObject::connect( context->wallet, &wallet::Wallet::onLoginResult, this, &InputPassword::onLoginResult, Qt::QueuedConnection );


    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &InputPassword::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect(context->wallet, &wallet::Wallet::onMwcMqListenerStatus,
                                         this, &InputPassword::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onKeybaseListenerStatus,
                                         this, &InputPassword::onKeybaseListenerStatus, Qt::QueuedConnection);

}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    bool running = context->wallet->isRunning();
    QString lockStr = context->appContext->pullCookie<QString>("LockWallet");
    inLockMode = false;

    // Allways try to start the wallet. State before is responsible for the first init
    if ( !running ) {
        // We are at the right place. Let's start the wallet

        // Processing all pending to clean up the processes
        QCoreApplication::processEvents();

        // Starting the wallet normally. The password is needed and it will be provided.
        // It is a first run, just need to login
        context->wallet->start(false);

        // As a node we can exit becuase no password is expected
        if (config::isOnlineNode()) {
            return NextStateRespond( NextStateRespond::RESULT::DONE );
        }

        wnd = (wnd::InputPassword*)context->wndManager->switchToWindowEx( mwc::PAGE_A_ACCOUNT_LOGIN,
                new wnd::InputPassword( context->wndManager->getInWndParent(), this,
                (state::WalletConfig *) context->stateMachine->getState(STATE::WALLET_CONFIG),
                false ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    if (!lockStr.isEmpty()) {
        inLockMode = true;
                // wallet locking mode
        wnd = (wnd::InputPassword*)context->wndManager->switchToWindowEx( mwc::PAGE_A_ACCOUNT_UNLOCK,
                       new wnd::InputPassword( context->wndManager->getInWndParent(), this,
                      (state::WalletConfig *) context->stateMachine->getState(STATE::WALLET_CONFIG),
                      true ) );
        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void InputPassword::submitPassword(const QString & password) {
    Q_ASSERT(wnd != nullptr);
    if (wnd) {
        wnd->startWaiting();
    }

    // Check if we need to logout first. It is very valid case if we in lock mode
    if ( inLockMode ) {
        context->wallet->logout(true);
        context->wallet->start(false);
        inLockMode = false;
    }

    context->wallet->loginWithPassword( password );
}

QPair<bool,bool> InputPassword::getWalletListeningStatus() {
    return context->wallet->getListenerStatus();
}

void InputPassword::onLoginResult(bool ok) {
    if (!ok) {
        if (wnd) {
            wnd->stopWaiting();
            wnd->reportWrongPassword();
        }
    }
    else {
        // Going forward by initializing the wallet
        if ( context->wallet->getStartedMode() == wallet::Wallet::STARTED_MODE::NORMAL ) { // Normall start of the wallet. Problem that now we have many cases how wallet started

            if ( config::isOnlineWallet() ) {
                // Start listening, no feedback interested
                context->wallet->listeningStart(true, false, true);
                context->wallet->listeningStart(false, true, true);
            }

            if (! config::isOnlineNode()) {
                // Set current receive account
                context->wallet->setReceiveAccount(context->appContext->getReceiveAccount());

                // Updating the wallet balance and a node status
                context->wallet->updateWalletBalance(true, true);
            }

            context->wallet->getNodeStatus();
        }

    }
}


// Account info is updated
void InputPassword::onWalletBalanceUpdated() {

    if (context->wallet->getWalletBalance().isEmpty() )
        return; // in restart mode

    // Using wnd as a flag that we are active
    if ( !inLockMode && wnd) {
        context->stateMachine->executeFrom(STATE::INPUT_PASSWORD);
    }
}

void InputPassword::onMwcMqListenerStatus(bool online) {
    if (wnd) {
        wnd->updateMwcMqState(online);
    }
}
void InputPassword::onKeybaseListenerStatus(bool online) {
    if (wnd) {
        wnd->updateKeybaseState(online);
    }
}


}
