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

#include <core/global.h>
#include "k_accounts.h"
#include "../wallet/wallet.h"
#include "../windows/k_accounts_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {


Accounts::Accounts(StateContext * context) :
    State(context, STATE::ACCOUNTS)
{
    connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Accounts::onWalletBalanceUpdated, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onAccountCreated, this, &Accounts::onAccountCreated, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onAccountRenamed, this, &Accounts::onAccountRenamed, Qt::QueuedConnection );
}

Accounts::~Accounts() {}

NextStateRespond Accounts::execute() {
    if (context->appContext->getActiveWndState() != STATE::ACCOUNTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd  = (wnd::Accounts*)context->wndManager->switchToWindowEx(new wnd::Accounts( context->wndManager->getInWndParent(), this ));
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// get balance for current account
QVector<wallet::AccountInfo> Accounts::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

void Accounts::updateWalletBalance() {
    context->wallet->updateWalletBalance();
}

void Accounts::doTransferFunds() {
    // Calling the next page
    context->stateMachine->setActionWindow( STATE::ACCOUNT_TRANSFER, true );
}

// add new account
void Accounts::createAccount(QString account) {
    context->wallet->createAccount(account);
}

void Accounts::onAccountCreated( QString newAccountName) {
    Q_UNUSED(newAccountName);
    if (wnd) {
        wnd->refreshWalletBalance();
    }
}

void Accounts::onWalletBalanceUpdated() {
    if (wnd) {
        wnd->refreshWalletBalance();
    }
}

void Accounts::onAccountRenamed(bool success, QString errorMessage) {
    if (wnd) {
         wnd->onAccountRenamed(success, errorMessage);
    }
}

QVector<int> Accounts::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("AccountTblColWidth");
}

void Accounts::updateColumnsWidhts(const QVector<int> & widths) {
    context->appContext->updateIntVectorFor("AccountTblColWidth", widths);
}

// ui caller must be at waiting state
void Accounts::renameAccount( const wallet::AccountInfo & account, QString newName ) {
    context->wallet->renameAccount( account.accountName, newName );
}

void Accounts::deleteAccount( const wallet::AccountInfo & account ) {
    // Delete is rename. Checking for names collision

    QVector<wallet::AccountInfo> allAccounts = context->wallet->getWalletBalance(false);

    QString newName;

    for (int t=0;t<100;t++) {
        newName = mwc::DEL_ACCONT_PREFIX + account.accountName + (t==0?"":("_" + QString::number(t)));
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

    renameAccount( account, newName );
}



}

