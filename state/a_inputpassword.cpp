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
#include <control/messagebox.h>

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

    QObject::connect(context->wallet, &wallet::Wallet::onHttpListeningStatus,
                     this, &InputPassword::onHttpListeningStatus, Qt::QueuedConnection);
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
        context->wallet->start();

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
        context->wallet->start();
        inLockMode = false;
    }

    context->wallet->loginWithPassword( password );

    if ( context->appContext->getActiveWndState() == STATE::SHOW_SEED ) {
        context->appContext->pushCookie<QString>("password", password);
    }

}

QPair<bool,bool> InputPassword::getWalletListeningStatus() {
    return context->wallet->getListenerStatus();
}

QPair<bool,QString> InputPassword::getWalletHttpListeningStatus() {
    return context->wallet->getHttpListeningStatus();
}

bool InputPassword::getWalletTls() {
    return context->wallet->hasTls();
}

//static bool foreignAPIwasReported = false;

void InputPassword::onLoginResult(bool ok) {

    if (wnd)
        wnd->onLoginResult(ok);

    if (!ok) {
        if (wnd) {
            wnd->stopWaiting();
            wnd->reportWrongPassword();
        }
    }
    else {
        // Going forward by initializing the wallet
        if ( context->wallet->getStartedMode() == wallet::Wallet::STARTED_MODE::NORMAL ) { // Normall start of the wallet. Problem that now we have many cases how wallet started

            if ( config::isOnlineWallet() && context->wallet->hasPassword() ) {
                // Start listening, no feedback interested
                if (context->appContext->isAutoStartMQSEnabled())
                    context->wallet->listeningStart(true, false, true);
                if (context->appContext->isAutoStartKeybaseEnabled())
                    context->wallet->listeningStart(false, true, true);
            }

            if (! config::isOnlineNode()) {
                // Updating the wallet balance and a node status
                context->wallet->updateWalletBalance(true, true);
            }

            context->wallet->getNodeStatus();

            /*  Users don't want that. It is too much to show that every start
              if (! config::isOnlineNode()) {
                if (!foreignAPIwasReported) {
                    // Check if foregn API is activated and it is not safe
                    wallet::WalletConfig config = context->wallet->getWalletConfig();


                    if (config.hasForeignApi()) {
                        QString message;

                        if (config.foreignApiSecret.isEmpty())
                            message += "without any authorization";

                        if (!config.hasTls()) {
                            if (!message.isEmpty())
                                message += " and ";

                            message += "with non secure HTTP connection. Please consider to setup TLS certificates for your security.";
                        }

                        if (!message.isEmpty()) {
                            message = "Your wallet has activated foreign API " + message;
                            message += "\n\nIt is reccomended that you use standard security guidelines.";

                            control::MessageBox::messageText(nullptr, "WARNING", message);

                            context->stateMachine->setActionWindow( state::STATE::LISTENING );
                        }
                    }
                    foreignAPIwasReported = true;
                }
            }*/
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

void InputPassword::onHttpListeningStatus(bool listening, QString additionalInfo) {
    Q_UNUSED(additionalInfo)
    if (wnd) {
        wnd->updateHttpState(listening);
    }
}


}
