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

#include "g_Send.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../util/filedialog.h"
#include "../util/ui.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/g_send_b.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QThread>

namespace state {


QString generateAmountErrorMsg(int64_t mwcAmount, const wallet::AccountInfo &acc, const core::SendCoinsParams &sendParams) {
    QString msg2print = "You are trying to send " + util::nano2one(mwcAmount) + " MWC, but you only have " +
                        util::nano2one(acc.currentlySpendable) + " spendable MWC.";
    if (acc.awaitingConfirmation > 0)
        msg2print += " " + util::nano2one(acc.awaitingConfirmation) + " coins are awaiting confirmation.";

    if (acc.lockedByPrevTransaction > 0)
        msg2print += " " + util::nano2one(acc.lockedByPrevTransaction) + " coins are locked.";

    if (acc.awaitingConfirmation > 0 || acc.lockedByPrevTransaction > 0) {
        if (sendParams.inputConfirmationNumber != 1) {
            if (sendParams.inputConfirmationNumber < 0)
                msg2print += " You can modify settings to spend MWC with less than 10 confirmations (wallet default value).";
            else
                msg2print += " You can modify settings to spend MWC with less than " +
                             QString::number(sendParams.inputConfirmationNumber) + " confirmations.";
        }
    }
    return msg2print;
}


Send::Send(StateContext * context) :
        State(context, STATE::SEND) {

    QObject::connect(context->wallet, &wallet::Wallet::onSend,
                     this, &Send::sendRespond, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onSendFile,
                     this, &Send::respSendFile, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onSendSlatepack,
                     this, &Send::respSendSlatepack, Qt::QueuedConnection);

    // Need to update mwc node status because send can fail if node is not healthy.
    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Send::onNodeStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onRequestRecieverWalletAddress,
                     this, &Send::onRequestRecieverWalletAddress, Qt::QueuedConnection);

}

Send::~Send() {}

NextStateRespond Send::execute() {
    if ( context->appContext->getActiveWndState() != STATE::SEND )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    switchToStartingWindow();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Send::switchToStartingWindow() {
    core::getWndManager()->pageSendStarting();
    context->wallet->updateWalletBalance(true,true); // request update, respond at onWalletBalanceUpdated
}

// onlineOffline => Next step
// Process sept 1 send request.  sendAmount is a value as user input it
// return code:
//   0 - ok
//   1 - account error
//   2 - amount error
int Send::initialSendSelection( bridge::SEND_SELECTED_METHOD sendSelectedMethod, QString account, QString sendAmount ) {

    QVector<wallet::AccountInfo> balance = context->wallet->getWalletBalance();
    wallet::AccountInfo selectedAccount;
    for (const auto & a : balance) {
        if (a.accountName == account) {
            selectedAccount = a;
            break;
        }
    }
    // Note, if no acount selected, that will be this case. We are fine with that.
    if (selectedAccount.currentlySpendable == 0) {
        core::getWndManager()->messageTextDlg("Incorrect Input", "Your account doesn't have any spendable MWC to send");
        return 1;
    }

    QPair<bool, int64_t> mwcAmount;
    mwcAmount = util::one2nano(sendAmount);
    if (!mwcAmount.first || mwcAmount.second<=0) {
            core::getWndManager()->messageTextDlg("Incorrect Input", "Please specify the number of MWC to send");
            return 2;
    }

    core::SendCoinsParams sendParams = context->appContext->getSendCoinsParams();

    // init expected to be fixed, so no need to disable the message
    if ( mwcAmount.second > selectedAccount.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, selectedAccount, sendParams );

        core::getWndManager()->messageTextDlg("Incorrect Input", msg2print );
        return 2;
    }

    // Check if we have extra for fee
    QStringList txnOutputList;
    uint64_t fee = util::getTxnFee( selectedAccount.accountName, mwcAmount.second, context->wallet,
                       context->appContext, sendParams.changeOutputs,
                       txnOutputList);
    if (fee < mwc::BASE_TRANSACTION_FEE) {
        core::getWndManager()->messageTextDlg("Incorrect Input", "Your account doesn't have enough MWC to send and cover this transaction fees." );
        return 2;
    }

    if (sendSelectedMethod == bridge::SEND_SELECTED_METHOD::ONLINE_ID ) {
        core::getWndManager()->pageSendOnline(selectedAccount.accountName, mwcAmount.second);
    }
    else if (sendSelectedMethod == bridge::SEND_SELECTED_METHOD::FILE_ID ) {
        core::getWndManager()->pageSendFile(selectedAccount.accountName, mwcAmount.second);
    }
    else if (sendSelectedMethod == bridge::SEND_SELECTED_METHOD::SLATEPACK_ID ) {
        core::getWndManager()->pageSendSlatepack(selectedAccount.accountName, mwcAmount.second);
    }
    else {
        Q_ASSERT(false);
    }

    return 0;
}

// Handle whole workflow to send offline
bool Send::sendMwcOffline( QString account, int64_t amount, QString message, bool isSlatepack, bool isLockLater, QString slatepackRecipientAddress) {

    core::SendCoinsParams sendParams = context->appContext->getSendCoinsParams();

    // !!!! NOTE.  For mobile HODL not a case, first case can be skipped, Directly can be called util->getTxnFee
    QStringList outputs;
    uint64_t txnFee = 0;
    if (! util::getOutputsToSend( account, sendParams.changeOutputs, amount,
                                  context->wallet, context->appContext,
                                  outputs, &txnFee) )
        return false; // User reject something

    if (txnFee == 0 && outputs.size() == 0) {
        txnFee = util::getTxnFee(account, amount, context->wallet,
                                 context->appContext, sendParams.changeOutputs, outputs);
    }
    QString txnFeeStr = util::txnFeeToString(txnFee);

    QString hash = context->wallet->getPasswordHash();
    int ttl_blocks = 1440;
    if ( core::WndManager::RETURN_CODE::BTN2 != core::getWndManager()->questionTextDlg("Confirm Send Request",
                       "You are sending offline " + (amount < 0 ? "all" : util::nano2one(amount)) +
                       " MWC from account: " + account + "\n\nTransaction fee: " + txnFeeStr +
                       "\n\nYour initial transaction slate will be stored in a file.",
                       "Decline", "Confirm",
                       "Don't send, cancel this operation",
                       "Everything is good, continue and send",
                       false, true, 1.0,
                       hash, core::WndManager::RETURN_CODE::BTN2, &ttl_blocks) )
        return false;

    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();

    // Note, Send by file and slatepack responding with different signals

    if ( isSlatepack) {
        // Init send transaction with file output
        // Check signal:  onSendSlatepack
        context->wallet->sendSlatepack( account, amount, message,
                                        prms.inputConfirmationNumber, prms.changeOutputs, outputs, ttl_blocks, context->appContext->getGenerateProof(),
                                        slatepackRecipientAddress, // optional. Encrypt SP if it is defined.
                                        isLockLater, "");
    }
    else {
        // send as a file
        QString fileName = util::getSaveFileName("Create Initial Transaction Slate File", "fileGen",
                                                 "MWC init transaction (*.tx)", ".tx");
        if (fileName.isEmpty())
            return false;

        context->wallet->sendFile(account, amount, message, fileName, prms.inputConfirmationNumber, prms.changeOutputs,
                                  outputs, ttl_blocks, context->appContext->getGenerateProof());
    }

    return true;
}

void Send::respSendFile( bool success, QStringList errors, QString fileName ) {
    QString message;
    if (success)
        message = "Transaction file was successfully generated at " + fileName;
    else
        message = "Unable to generate transaction file.\n" + util::formatErrorMessages(errors);

    if ( state::getStateMachine()->getCurrentStateId() == STATE::SEND ) {
        for (auto b : bridge::getBridgeManager()->getSend())
            b->showSendResult(success, message);

        if (success) {
            switchToStartingWindow();
        }
    }
}

void Send::respSendSlatepack( QString tagId, QString error, QString slatepack ) {
    Q_UNUSED(tagId)
    if (!error.isEmpty()) {
        // show error...
        if ( state::getStateMachine()->getCurrentStateId() == STATE::SEND ) {
            for (auto b : bridge::getBridgeManager()->getSend())
                b->showSendResult(false, error);
        }
        return;
    }

    // Let's show the slatepack and enable in place finalization
    core::getWndManager()->pageShowSlatepack( slatepack, STATE::SEND, ".tx", true );
}


bool Send::sendMwcOnline( QString account, int64_t amount, QString address, QString apiSecret, QString message) {
    while(address.endsWith("/"))
        address = address.left(address.length()-1);

    // Let's  verify address first
    QPair< bool, util::ADDRESS_TYPE > addressRes = util::verifyAddress(address);
    if ( !addressRes.first ) {
        core::getWndManager()->messageTextDlg("Incorrect Input",
                                         "Please specify correct address to send your MWC" );
        return false;
    }

    const wallet::ListenerStatus listenerStart = context->wallet->getListenerStartState();
    const wallet::ListenerStatus listenerStatus = context->wallet->getListenerStatus();

    bool genProof = context->appContext->getGenerateProof();

    switch (addressRes.second) {
        case util::ADDRESS_TYPE::MWC_MQ: {
            genProof = false; // for MQS we are getting proof legacy way. It will help with backward compability
            if (!listenerStart.mqs) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                                                      "MQS listener is not started. Please start MQS listener first." );
                return false;
            }
            if (!listenerStatus.mqs) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                                                      "MQS listener is not online even it was started. Please check your network connection and firewall settings." );
                return false;
            }
            break;
        }
        case util::ADDRESS_TYPE::TOR: {
            if (!listenerStart.tor) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                                                      "Tor listener is not started. Please start Tor listener first." );
                return false;
            }

            // Let's convert tor address to the standard noration that mwc713 undertands
            QString tor_pk = util::extractPubKeyFromAddress(address);
            address = "http://" + tor_pk + ".onion";

            /*  Not checking offline because there is a high chance that send will work
             if (!listenerStatus.tor) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                                                      "Tor listener is not online even it was started. Please check your network connection and firewall settings.");
                return false;
            }*/
            break;
        }
        default: // Http is fine.
            break;
    }

    core::SendCoinsParams sendParams = context->appContext->getSendCoinsParams();

    QStringList outputs;
    uint64_t txnFee = 0;
    if (! util::getOutputsToSend( account, sendParams.changeOutputs, amount,
                                  context->wallet, context->appContext,
                                  outputs, &txnFee ) )
        return false; // User reject something

    if (txnFee == 0 && outputs.size() == 0) {
        txnFee = util::getTxnFee( account, amount, context->wallet,
                                  context->appContext, sendParams.changeOutputs, outputs );
    }
    QString txnFeeStr = util::txnFeeToString(txnFee);

    respProofAddress = "";
    restProofError = "";

    if (genProof && addressRes.second == util::ADDRESS_TYPE::HTTPS ) {
        // Requesting proof address
        context->wallet->requestRecieverWalletAddress(address, apiSecret);

        // waiting for the respose in sync mode...
        // It is not good, but other solutions are more wierd.
        while(respProofAddress.isEmpty() && restProofError.isEmpty()) {
            QCoreApplication::processEvents();
            QThread::usleep(50);
        }

        if (!restProofError.isEmpty()) {
            core::getWndManager()->messageTextDlg("Error", "Unable to get a wallet proof address for the " + address);
            return false;
        }
        Q_ASSERT(!respProofAddress.isEmpty());
    }

    // Ask for confirmation
    QString hash = context->wallet->getPasswordHash();
    if ( core::getWndManager()->sendConfirmationDlg("Confirm Send Request",
                                        "You are sending " + (amount < 0 ? "all" : util::nano2one(amount)) + " MWC from account: " + account +
                                        "\nTo: " + address +
                                        (respProofAddress.isEmpty() ? "" : "\n\nReceiver wallet proof address:\n" + respProofAddress) +
                                        "\n\nTransaction fee: " + txnFeeStr,
                                        1.0, hash ) ) {

        context->wallet->sendTo( account, amount, util::fullFormalAddress( addressRes.second, address), apiSecret, message,
                                 sendParams.inputConfirmationNumber, sendParams.changeOutputs,
                                 outputs, context->appContext->isFluffSet(), -1 /* Not used for online sends */,
                                 genProof, respProofAddress);

        return true;
    }
    return false;
}

// Response from requestRecieverWalletAddress(url)
void Send::onRequestRecieverWalletAddress(QString url, QString proofAddress, QString error) {
    Q_UNUSED(url)
    respProofAddress = proofAddress;
    restProofError = error;
}


QString Send::getSpendAllAmount(QString account) {
    return util::getAllSpendableAmount(account, context->wallet, context->appContext);
}


void Send::sendRespond( bool success, QStringList errors, QString address, int64_t txid, QString slate ) {
    Q_UNUSED(address)
    Q_UNUSED(txid)
    Q_UNUSED(slate)

    QString errMsg;

    if (!success) {
        errMsg = util::formatErrorMessages(errors);
        if (errMsg.isEmpty())
            errMsg = "Your send request has failed by some reasons";
        else
            errMsg = "Your send request has failed:\n\n" + errMsg;
    }

    for (auto b : bridge::getBridgeManager()->getSend())
        b->showSendResult(success, errMsg);

    if (success && state::getStateMachine()->getCurrentStateId() == STATE::SEND)
        switchToStartingWindow();
}

void Send::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    nodeIsHealthy = online &&
                    ((config::isColdWallet() || connections > 0) && totalDifficulty > 0 && nodeHeight > peerHeight - 5);
}


}

