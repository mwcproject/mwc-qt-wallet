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
#include "node/node_client.h"
#include "util/message_mapper.h"
#include "../util/Log.h"
#include "core/Notification.h"

namespace state {

AccountTransfer::AccountTransfer( StateContext * context) :
        State(context, STATE::ACCOUNT_TRANSFER) {

    // using static connection. Lock flag transferInProgress will be used to switch the processing

    connect( context->wallet, &wallet::Wallet::onSend, this, &AccountTransfer::onSend, Qt::QueuedConnection );
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
    logger::logInfo(logger::STATE, "Call AccountTransfer::transferFunds with from=" + from + " to=" + to + " sendAmount=" + sendAmount);

    if ( !context->nodeClient->isNodeHealthy() ) {
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
    // For self send, let's allow to send all coins, no confirmations
    QVector<wallet::AccountInfo> walletAccounts = context->wallet->getWalletBalance(0, true, context->appContext->getLockedOutputs() );
    for (auto & a : walletAccounts) {
        if (a.accountName == from)
            accFrom = a;
    }

    core::SendCoinsParams sendParams = context->appContext->getSendCoinsParams();

    if ( mwcAmount.second > accFrom.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, accFrom, sendParams );

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
    util::getOutputsToSend2( accFrom.accountPath, sendParams.changeOutputs, nanoCoins, context->wallet, context->appContext, outputs, &txnFee);

    // Need to show confirmation dialog similar to what send has. Point to show the fees
    if (txnFee == 0 && outputs.size() == 0) {
        txnFee = util::getTxnFee2( accFrom.accountPath, nanoCoins, context->wallet,
                                  context->appContext, sendParams.changeOutputs, outputs );
    }
    QString txnFeeStr = util::txnFeeToString(txnFee);

    if ( !core::getWndManager()->sendConfirmationDlg("Confirm Transfer Request",
                                                    "You are transferring " + (nanoCoins < 0 ? "all" : util::nano2one(nanoCoins)) +
                                                    " MWC\nfrom account '" + from + "' to account '" + to + "'",
                                                    1.0, outputs.size() ) ) {
        return false;
    }

    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();

    if (prms.changeOutputs != sendParams.changeOutputs) {
        // Recalculating the outputs and fees. There is a chance that outputs will be different.
        util::getOutputsToSend2( accFrom.accountPath, sendParams.changeOutputs, nanoCoins, context->wallet, context->appContext, outputs, &txnFee);
        if (outputs.size() == 0) {
            util::getTxnFee2( accFrom.accountPath, nanoCoins, context->wallet,
                                  context->appContext, sendParams.changeOutputs, outputs );
        }
    }

    // Expected that everything is fine, but will do operation step by step
    // 1. set-recv to accountTo
    // 3. Send funds (will switch to account).
    // 4. Wait for 'finalize' step
    // 6. Restore back receive account
    // 7. Restore back current account
    // 5. Refresh accounts balance

    transferState=1;

    bool fluff = context->appContext->isFluffSet();
    bool sent = context->wallet->sendTo(accFrom.accountPath,
            "self_send",
            nanoCoins, //  -1  - mean All
            false,
            "", // can be empty, means None
            1,
            "smallest", //  Values: all, smallest. Default: Smallest
            "self",  // Values:  http, file, slatepack, self, mwcmqs
            to, // Values depends on 'method' Send the transaction to the provided server (start with http://), destination for encrypting/proofs. For method self, dest can point to account name to move
            false,
            prms.changeOutputs,
            fluff,
            -1, // pass -1 to skip
            false,
            outputs, // Outputs to use. If None, all outputs can be used
            false,
            0);

    if (!sent) {
        core::getWndManager()->messageTextDlg("Error",
            "Another Send operation is still in the progress. Please wait until it finished and retry after.");
    }
    return sent;
}

void AccountTransfer::goBack() {
    logger::logInfo(logger::STATE, "Call AccountTransfer::goBack");
    context->stateMachine->setActionWindow( STATE::ACCOUNTS );
}

void AccountTransfer::onSend(  bool success, QString error, QString tx_uuid, int64_t amount, QString method, QString dest, QString tag ) {
    Q_UNUSED(amount);
    Q_UNUSED(method);
    Q_UNUSED(dest);

    logger::logInfo(logger::STATE, "Call AccountTransfer::onSend with success=" + QString(success ? "true" : "false") +
            " error=" + error + " tx_uuid=" + tx_uuid + " tag=" + tag);
    Q_UNUSED(tx_uuid)

    if (tag != "self_send")
        return;

    Q_ASSERT(transferState==1);
    transferState = -1;

    if  (!success) {
        for (auto b : bridge::getBridgeManager()->getAccountTransfer()) {
            b->showTransferResults(false, util::mapMessage(error) );
        }
        return;
    }

    for (auto b : bridge::getBridgeManager()->getAccountTransfer()) {
        b->updateAccounts();
        b->showTransferResults(true, "" );
    }

    notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "You successfully transfered " + util::nano2one(amount) + " MWC to " + dest + ". Transaction: " + tx_uuid );

    context->stateMachine->setActionWindow( STATE::ACCOUNTS );
}



}
