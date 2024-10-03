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
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../util/address.h"
#include "../util/ui.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/k_accounttransfer_b.h"
#include "g_Send.h"

namespace state {

AccountTransfer::AccountTransfer( StateContext * context) :
        State(context, STATE::ACCOUNT_TRANSFER) {

    // using static connection. Lock flag transferInProgress will be used to switch the processing

    connect( context->wallet, &wallet::Wallet::onSend, this, &AccountTransfer::onSend, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &AccountTransfer::onWalletBalanceUpdated, Qt::QueuedConnection );
    connect( context->wallet, &wallet::Wallet::onNodeStatus, this, &AccountTransfer::onNodeStatus, Qt::QueuedConnection);
}

AccountTransfer::~AccountTransfer() {

}

NextStateRespond AccountTransfer::execute() {
    if (context->appContext->getActiveWndState() != STATE::ACCOUNT_TRANSFER)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if ( state::getStateMachine()->getCurrentStateId() != STATE::ACCOUNT_TRANSFER ) {
        core::getWndManager()->pageAccountTransfer();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


// nanoCoins < 0 - all funds
bool AccountTransfer::transferFunds(const QString & from,
                   const QString & to,
                   const QString & sendAmount ) {

    if ( !nodeIsHealthy ) {
        core::getWndManager()->messageTextDlg("Unable to transfer", "Your MWC Node, that wallet is connected to, is not ready.\n"
                                                                 "MWC Node needs to be connected to a few peers and finish block synchronization process");
        return false;
    }

    QPair<bool, int64_t> mwcAmount;
    if (sendAmount != "All") {
        mwcAmount = util::one2nano(sendAmount);
        if (!mwcAmount.first || mwcAmount.second<=0) {
            core::getWndManager()->messageTextDlg("Incorrect Input", "Please specify correct number of MWC to transfer");
            return false;
        }
    }
    else { // All
        mwcAmount = QPair<bool, int64_t>(true, -1);
    }

    int64_t nanoCoins = mwcAmount.second;

    wallet::AccountInfo accFrom;
    QVector<wallet::AccountInfo> walletAccounts = context->wallet->getWalletBalance();
    for (auto & a : walletAccounts) {
        if (a.accountName == from)
            accFrom = a;
    }

    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();

    if ( mwcAmount.second > accFrom.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, accFrom, prms );

        core::getWndManager()->messageTextDlg("Incorrect Input",
                                         msg2print );

        return false;
    }


    if ( state::getStateMachine()->getCurrentStateId() != STATE::ACCOUNT_TRANSFER ) {
        Q_ASSERT(false);
        return false;
    }

    if (transferState>=0) {
        for (auto b : bridge::getBridgeManager()->getAccountTransfer())
            b->showTransferResults(false, "Another funds transfer operation is in the progress. We can't start a new transfer.");
        return false;
    }

    QStringList outputs; // empty is valid value. Empty - mwc713 will use default algorithm.
    uint64_t txnFee = 0; // not used here yet
    // nanoCoins < 0  - All
    if (! util::getOutputsToSend( accFrom.accountName, prms.changeOutputs, nanoCoins, context->wallet, context->appContext, outputs, &txnFee) ) {
        for (auto b : bridge::getBridgeManager()->getAccountTransfer())
            b->hideProgress();
        return false; // User cancel transaction
    }

    // Need to show confirmation dialog similar to what send has. Point to show the fees
    if (txnFee == 0 && outputs.size() == 0) {
        txnFee = util::getTxnFee( accFrom.accountName, nanoCoins, context->wallet,
                                  context->appContext, prms.changeOutputs, outputs );
    }
    QString txnFeeStr = util::txnFeeToString(txnFee);

    QString hash = context->wallet->getPasswordHash();
    if ( !core::getWndManager()->sendConfirmationDlg("Confirm Transfer Request",
                                                    "You are transferring " + (nanoCoins < 0 ? "all" : util::nano2one(nanoCoins)) +
                                                    " MWC\nfrom account '" + from + "' to account '" + to + "'" +
                                                    "\n\nTransaction fee: " + txnFeeStr,
                                                    1.0, hash ) ) {
        return false;
    }

    // Expected that everything is fine, but will do operation step by step
    // 1. set-recv to accountTo
    // 3. Send funds (will switch to account).
    // 4. Wait for 'finalize' step
    // 6. Restore back receive account
    // 7. Restore back current account
    // 5. Refresh accounts balance

    trAccountFrom = from;
    trAccountTo = to;
    trNanoCoins = nanoCoins;
    outputs2use = outputs;

    transferState=1;

    bool fluff = context->appContext->isFluffSet();
    context->wallet->selfSend( trAccountFrom, trAccountTo, trNanoCoins, outputs2use, fluff );

    return true;
}

void AccountTransfer::goBack() {
    context->stateMachine->setActionWindow( STATE::ACCOUNTS );
}

void AccountTransfer::onSend( bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc ) {
    Q_UNUSED(txid);
    Q_UNUSED(slate);
    Q_UNUSED(address);
    Q_UNUSED(mwc);

    if (transferState!=1)
        return;

    context->wallet->updateWalletBalance(true, true);

    if  (!success) {
        for (auto b : bridge::getBridgeManager()->getAccountTransfer())
            b->showTransferResults(false, "Failed to transfer the funds. " + util::formatErrorMessages(errors) );
        transferState = -1;
        return;
    }
}

void AccountTransfer::onWalletBalanceUpdated() {
    if (transferState!=1) {
        for (auto b : bridge::getBridgeManager()->getAccountTransfer())
            b->updateAccounts();
        return;
    }

    for (auto b : bridge::getBridgeManager()->getAccountTransfer())
        b->showTransferResults(true, "" );

    // Done, finally
    transferState = -1;

    context->stateMachine->setActionWindow( STATE::ACCOUNTS );
}

void AccountTransfer::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    nodeIsHealthy = online &&
                    ((config::isColdWallet() || connections > 0) && totalDifficulty > 0 && nodeHeight > peerHeight - 5);
}


}
