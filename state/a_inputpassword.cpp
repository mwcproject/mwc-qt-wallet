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
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include <QCoreApplication>
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/a_inputpassword_b.h"
#include <QDir>
#include "../util/crypto.h"

namespace state {

InputPassword::InputPassword( StateContext * _context) :
    State(_context, STATE::INPUT_PASSWORD)
{
    // Result of the login
    QObject::connect( context->wallet, &wallet::Wallet::onLoginResult, this, &InputPassword::onLoginResult, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &InputPassword::onWalletBalanceUpdated, Qt::QueuedConnection );
}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    bool running = context->wallet->isRunning();
    QString lockStr = context->appContext->pullCookie<QString>("LockWallet");
    inLockMode = false;

    // Always try to start the wallet. State before is responsible for the first init
    if ( !running ) {
        // We are at the right place. Let's start the wallet

        // Processing all pending to clean up the processes
        QCoreApplication::processEvents();

        // As a node we can exit because no password is expected
        if (config::isOnlineNode()) {
            // Starting the wallet normally for the node
            context->wallet->start();
            return NextStateRespond( NextStateRespond::RESULT::DONE );
        }

        core::getWndManager()->pageInputPassword(mwc::PAGE_A_ACCOUNT_LOGIN, false);

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    if (!lockStr.isEmpty()) {
        inLockMode = true;
        lockedWalletPath = context->appContext->getCurrentWalletInstance(true);
        mwc::setWalletLocked(inLockMode);
        // wallet locking mode
        core::getWndManager()->pageInputPassword(mwc::PAGE_A_ACCOUNT_UNLOCK, true);
        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void InputPassword::submitPassword(const QString & password) {
    // Check if we can skip logout/login step.
    // If wallet wasn't changed, there is no reason logout and restart all listeners.
    if (inLockMode && lockedWalletPath == context->appContext->getCurrentWalletInstance(true) &&
                context->appContext->getActiveWndState() != STATE::SHOW_SEED) {
        // We can just verify the password and we should be good

        if (crypto::calcHSA256Hash(password) !=
                context->wallet->getPasswordHash() ) {
            // mwaasage match with mwc713 returns
            core::getWndManager()->messageTextDlg("Password", "Password supplied was incorrect. Please input correct password.");
            return;
        }
        // We are good here, let's unlock the wallet

        inLockMode = false;
        mwc::setWalletLocked(inLockMode);

        context->stateMachine->executeFrom(STATE::NONE);
        return;
    }

    // Check if we need to logout first. It is very valid case if we in lock mode
    if ( context->wallet->isRunning() )
        context->wallet->logout(true);

    if (inLockMode) {
        inLockMode = false;
        mwc::setWalletLocked(inLockMode);
    }

    context->wallet->start();
    context->wallet->loginWithPassword( password );

    if ( context->appContext->getActiveWndState() == STATE::SHOW_SEED ) {
        context->appContext->pushCookie<QString>("password", password);
    }

}

//static bool foreignAPIwasReported = false;

void InputPassword::onLoginResult(bool ok) {

    if (ok) {
        // Going forward by initializing the wallet
        if ( context->wallet->getStartedMode() == wallet::Wallet::STARTED_MODE::NORMAL ) { // Normall start of the wallet. Problem that now we have many cases how wallet started

            if (! config::isOnlineNode()) {
                // Updating the wallet balance and a node status
                context->wallet->updateWalletBalance(true, true);
            }

            // Starting listeners after balance to speed up the init process
            if ( config::isOnlineWallet() && context->wallet->hasPassword() ) {
                // Start listening, no feedback interested
                context->wallet->listeningStart(context->appContext->isAutoStartMQSEnabled(),
                                                context->appContext->isAutoStartTorEnabled(),
                                                true);
            }

            context->wallet->getNodeStatus();
        }

    }
}


// Account info is updated
void InputPassword::onWalletBalanceUpdated() {

    if (context->wallet->getWalletBalance().isEmpty() )
        return; // in restart mode

    // Using wnd as a flag that we are active.
    if ( !inLockMode && state::getStateMachine()->getCurrentStateId() == STATE::INPUT_PASSWORD) {
        context->stateMachine->executeFrom(STATE::INPUT_PASSWORD);
    }
}


}
