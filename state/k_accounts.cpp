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
#include "node/node_client.h"
#include "../util/Log.h"

namespace state {

Accounts::Accounts(StateContext * context) :
    State(context, STATE::ACCOUNTS)
{
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
    logger::logInfo(logger::STATE, "Call Accounts::doTransferFunds");
    // Calling the next page
    context->stateMachine->setActionWindow( STATE::ACCOUNT_TRANSFER, true );
}

void Accounts::deleteAccount( QString accountName ) {
    logger::logInfo(logger::STATE, "Call Accounts::deleteAccount with accountName=" + accountName);
    // Delete is rename. Checking for names collision

    QVector<wallet::Account> allAccounts = context->wallet->listAccounts();

    QString accountPath;
    for (const auto & a : allAccounts) {
        if (a.label == accountName) {
            accountPath = a.path;
            break;
        }
    }

    if (accountPath.isEmpty()) {
        core::getWndManager()->messageTextDlg("Incorrect account name",
                                         "Internal error. Not found account to delete." );
        return;
    }

    QString newName;

    for (int t=0;t<100;t++) {
        newName = mwc::DEL_ACCONT_PREFIX + accountName + (t==0?"":("_" + QString::number(t)));
        bool collision = false;
        for ( auto & acc : allAccounts ) {
            if (acc.label == newName) {
                collision = true;
                break;
            }
        }
        if (!collision)
            break;
    }
    context->wallet->renameAccountById( accountPath, newName );
}

}

