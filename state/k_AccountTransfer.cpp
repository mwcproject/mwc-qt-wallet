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

#include "k_AccountTransfer.h"
#include "../wallet/wallet.h"
#include "../windows/k_accounttransfer_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../util/address.h"
#include "../util/ui.h"

namespace state {

AccountTransfer::AccountTransfer( StateContext * context) :
        State(context, STATE::ACCOUNT_TRANSFER) {

    // using static connection. Lock flag transferInProgress will be used to switch the processing

    connect( context->wallet, &wallet::Wallet::onSetReceiveAccount, this, &AccountTransfer::onSetReceiveAccount, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onSend, this, &AccountTransfer::onSend, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onSlateSendTo, this, &AccountTransfer::onSlateSendTo, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onSlateFinalized, this, &AccountTransfer::onSlateFinalized, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &AccountTransfer::onWalletBalanceUpdated, Qt::QueuedConnection );
}

AccountTransfer::~AccountTransfer() {

}

NextStateRespond AccountTransfer::execute() {
    if (context->appContext->getActiveWndState() != STATE::ACCOUNT_TRANSFER)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd = (wnd::AccountTransfer*) context->wndManager->switchToWindowEx( mwc::PAGE_K_ACCOUNT_TRANSFER,
                new wnd::AccountTransfer( context->wndManager->getInWndParent(), this ) );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


// get balance for current account
QVector<wallet::AccountInfo> AccountTransfer::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

core::SendCoinsParams AccountTransfer::getSendCoinsParams() {
    return context->appContext->getSendCoinsParams();
}

void AccountTransfer::updateSendCoinsParams(const core::SendCoinsParams &params) {
    context->appContext->setSendCoinsParams(params);
    // Need to update the balances because number of confirmations affect the spendable balance.
    context->wallet->updateWalletBalance(true, true);
}


// nanoCoins < 0 - all funds
void AccountTransfer::transferFunds(const wallet::AccountInfo accountFrom,
                        const wallet::AccountInfo accountTo,
                        int64_t nanoCoins) {
    Q_ASSERT(wnd);
    if (!wnd)
        return;

    if (transferState>=0) {
        wnd-> showTransferResults(false, "Another funds transfer operation is in the progress. We can't start a new transfer.");
        return;
    }

    myAddress = context->wallet->getLastKnownMwcBoxAddress();

    // mwc mq expected to be online, we will use it for slate exchange
    if (myAddress.isEmpty() || !context->wallet->getListenerStatus().first) {
        wnd->showTransferResults(false, "Please turn on mwc mq listener. We can't transfer funds in offline mode");
        return;
    }

    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();

    // Check if HODL outputs will be affected
    QStringList outputs; // empty is valid value. Empty - mwc713 will use default algorithm.
    uint64_t txnFee = 0; // not used here yet
    // nanoCoins < 0  - All
    if (! util::getOutputsToSend( accountFrom.accountName, prms.changeOutputs, nanoCoins, context->wallet, getContext()->hodlStatus, context->appContext, nullptr, outputs, &txnFee) ) {
        wnd->hideProgress();
        return; // User cancel transaction
    }

    // Expected that everything is fine, but will do operation step by step
    // 1. set-recv to accountTo
    // 3. Send funds (will switch to account).
    // 4. Wait for 'finalize' step
    // 6. Restore back receive account
    // 7. Restore back current account
    // 5. Refresh accounts balance

    trAccountFrom = accountFrom;
    trAccountTo = accountTo;
    trNanoCoins = nanoCoins;
    trSlate = "";
    outputs2use = outputs;

    transferState = 0;
    recieveAccount = context->wallet->getReceiveAccount();
    context->wallet->setReceiveAccount( trAccountTo.accountName );
}

void AccountTransfer::goBack() {
    context->stateMachine->setActionWindow( STATE::ACCOUNTS );
}

// set receive account name results
void AccountTransfer::onSetReceiveAccount( bool ok, QString AccountOrMessage ) {
    if (transferState!=0)
        return;

    if  (!ok) {
        if (wnd)
            wnd->showTransferResults(false, "Failed to set receive account. " + AccountOrMessage);
        transferState = -1;
        return;
    }

    transferState=1;

    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();
    bool fluff = context->appContext->isFluffSet();
    context->wallet->sendTo( trAccountFrom, trNanoCoins, util::fullFormalAddress( util::ADDRESS_TYPE::MWC_MQ, myAddress), "", "", prms.inputConfirmationNumber, prms.changeOutputs, outputs2use, fluff );
}


void AccountTransfer::onSend( bool success, QStringList errors, QString address, int64_t txid, QString slate ) {
    Q_UNUSED(txid);
    Q_UNUSED(slate);
    Q_UNUSED(address);

    if (transferState!=1)
        return;

    if  (!success) {
        if (wnd)
            wnd->showTransferResults(false, "Failed to send the funds. " + util::formatErrorMessages(errors) );
        transferState = -1;
        return;
    }

    // Waiting for finalized slate to continue
}

void AccountTransfer::onSlateSendTo( QString slate, QString mwc, QString sendAddr ) {
    Q_UNUSED(mwc);
    Q_UNUSED(sendAddr);
    if (transferState!=1)
        return;

    trSlate = slate;
}

void AccountTransfer::onSlateFinalized( QString slate ) {
    if (transferState!=1)
        return;

    if (slate == trSlate) {
        // can go forward. Restore the state and update the balance

        transferState=2;
        if (!recieveAccount.isEmpty())
            context->wallet->setReceiveAccount( recieveAccount );
        context->wallet->updateWalletBalance(true, true);
    }
}

void AccountTransfer::onWalletBalanceUpdated() {
    if (transferState!=2) {
        if (wnd)
            wnd->updateAccounts();
        return;
    }

    if (wnd)
        wnd->showTransferResults(true, "" );

    // Done, finally
    transferState = -1;
}



}
