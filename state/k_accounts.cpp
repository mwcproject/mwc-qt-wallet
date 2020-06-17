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

#include "k_accounts.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../core/Notification.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/k_accounts_b.h"

namespace state {

Accounts::Accounts(StateContext * context) :
    State(context, STATE::ACCOUNTS)
{
    connect( context->wallet, &wallet::Wallet::onLoginResult, this, &Accounts::onLoginResult, Qt::QueuedConnection );

    connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Accounts::onNodeStatus, Qt::QueuedConnection);

    startingTime = 0;

    startTimer(37000);
}

Accounts::~Accounts() {}

NextStateRespond Accounts::execute() {
    if (context->appContext->getActiveWndState() != STATE::ACCOUNTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if ( state::getStateMachine()->getCurrentStateId() != STATE::ACCOUNTS ) {
        core::getWndManager()->pageAccounts();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Accounts::doTransferFunds() {
    // Calling the next page
    context->stateMachine->setActionWindow( STATE::ACCOUNT_TRANSFER, true );
}

void Accounts::deleteAccount( QString accountName ) {
    // Delete is rename. Checking for names collision

    QVector<wallet::AccountInfo> allAccounts = context->wallet->getWalletBalance(false);

    QString newName;

    for (int t=0;t<100;t++) {
        newName = mwc::DEL_ACCONT_PREFIX + accountName + (t==0?"":("_" + QString::number(t)));
        bool collision = false;
        for ( auto & acc : allAccounts ) {
            if (acc.accountName ==  newName) {
                collision = true;
                break;
            }
        }
        if (!collision)
            break;
    }
    context->wallet->renameAccount( accountName, newName );
}

void Accounts::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);

    // Skipping first 5 seconds after start. Let's mwc-node get online
    if ( startingTime==0 || QDateTime::currentMSecsSinceEpoch() - startingTime < 5000 )
        return;

    if ( !context->wallet->isWalletRunningAndLoggedIn() ) {
        startingTime=0;
        return;
    }


    if ( isNodeHealthy() ) {
        if (!lastNodeIsHealty) {
            notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                              "MWC-Node that wallet connected to is healthy now. Wallet can validate stored data with blockchain.");
            lastNodeIsHealty = true;
        }
        else {
            context->wallet->updateWalletBalance(true, false);
        }
    }
    else {
        if (lastNodeIsHealty) {
            notify::appendNotificationMessage(notify::MESSAGE_LEVEL::WARNING,
                                              "Wallet connected to not healthy MWC-Node. Your balance, transactions and output status might be not accurate");
            lastNodeIsHealty = false;
        }
        context->wallet->updateWalletBalance(false, false);
    };

}

void Accounts::onLoginResult(bool ok) {
    Q_UNUSED(ok)
    startingTime = QDateTime::currentMSecsSinceEpoch();
}


void Accounts::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    nodeIsHealthy = online &&
                    ((config::isColdWallet() || connections > 0) && totalDifficulty > 0 && nodeHeight > peerHeight - 5);
}



}

