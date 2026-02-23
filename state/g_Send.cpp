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
#include "../util/Files.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/g_send_b.h"
#include <QFileInfo>
#include <QCoreApplication>
#include <QThread>

#include "core/Notification.h"
#include "util/message_mapper.h"
#ifdef WALLET_MOBILE
#include "../core_mobile/qtandroidservice.h"
#endif
#include <QUrl>

namespace state {


QString generateAmountErrorMsg(qint64 mwcAmount, const wallet::AccountInfo &acc, const core::SendCoinsParams &sendParams) {
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
}

Send::~Send() {}

NextStateRespond Send::execute() {
    atSendInitialPage = true;

    if ( context->appContext->getActiveWndState() != STATE::SEND )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    switchToStartingWindow();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

bool Send::mobileBack() {
    if (atSendInitialPage) {
        return false;
    }
    else {
        switchToStartingWindow();
        return true;
    }
}


void Send::switchToStartingWindow() {
    core::getWndManager()->pageSendStarting();
    atSendInitialPage = true;
    if (context->wallet->isNodeAlive()) {
        context->wallet->scan(false, false);
    }
}

// onlineOffline => Next step
// Process sept 1 send request.  sendAmount is a value as user input it
// return code:
//   0 - ok
//   1 - account error
//   2 - amount error
int Send::initialSendSelection( bridge::SEND_SELECTED_METHOD sendSelectedMethod, QString accountPath, QString sendAmount, bool gotoNextPage ) {
    logger::logInfo(logger::STATE, "Call Send::initialSendSelection with sendSelectedMethod=" + QString::number(sendSelectedMethod) + " accountPath=" + accountPath + " sendAmount=" + sendAmount + " gotoNextPage=" + QString(gotoNextPage ? "true" : "false"));

    QVector<wallet::AccountInfo> balance = context->wallet->getWalletBalance( context->appContext->getSendCoinsParams().inputConfirmationNumber, true, context->appContext->getLockedOutputs() );
    wallet::AccountInfo selectedAccount;
    for (const auto & a : balance) {
        if (a.accountPath == accountPath) {
            selectedAccount = a;
            break;
        }
    }
    // Note, if no acount selected, that will be this case. We are fine with that.
    if (selectedAccount.currentlySpendable == 0) {
        core::getWndManager()->messageTextDlg("Incorrect Input", "Your account doesn't have any spendable MWC to send");
        return 1;
    }

    QPair<bool, qint64> mwcAmount;
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
    quint64 fee = util::getTxnFee( selectedAccount.accountPath, mwcAmount.second, context->wallet,
                       context->appContext, sendParams.changeOutputs,
                       txnOutputList).first;
    if (fee < mwc::BASE_TRANSACTION_FEE) {
        core::getWndManager()->messageTextDlg("Incorrect Input", "Your account doesn't have enough MWC to send and cover the transaction fees.");
        return 2;
    }

    tmpAccountPath = selectedAccount.accountPath;
    tmpAmount = mwcAmount.second;

    if (!gotoNextPage)
        return 0;

    // Switching to some dependent windows.
    atSendInitialPage = false;
    if (sendSelectedMethod == bridge::SEND_SELECTED_METHOD::ONLINE_ID ) {
        core::getWndManager()->pageSendOnline(selectedAccount.accountName, selectedAccount.accountPath, mwcAmount.second);
    }
    else if (sendSelectedMethod == bridge::SEND_SELECTED_METHOD::SLATEPACK_ID ) {
        core::getWndManager()->pageSendSlatepack(selectedAccount.accountName, selectedAccount.accountPath, mwcAmount.second);
    }
    else {
        Q_ASSERT(false);
    }

    return 0;
}

QString Send::getAccountPathByName(const QString account, bool showErrMessage) {
    QString accountPath;
    QVector<wallet::Account> accounts = context->wallet->listAccounts();
    for (const auto & a : accounts) {
        if (a.label == account) {
            accountPath = a.path;
            break;
        }
    }

    if (accountPath.isEmpty() && showErrMessage) {
        core::getWndManager()->messageTextDlg("Incorrect account name",
                                         "Internal error. Not found account to send from." );
    }
    return accountPath;
}


// Handle whole workflow to send offline
bool Send::sendMwcOffline( const QString & account, const QString & accountPath, qint64 amount, const QString & message, bool isLockLater, const QString & slatepackRecipientAddress) {
    logger::logInfo(logger::STATE, "Call Send::sendMwcOffline with account=" + accountPath + " amount=" + QString::number(amount) +
            " message=" + (message.isEmpty() ? "<empty>" : "<hidden>") + " isLockLater=" + QString(isLockLater ? "true" : "false") + " slatepackRecipientAddress=" + (slatepackRecipientAddress.isEmpty() ? "<empty>" : "<hidden>"));


    if (context->appContext->getGenerateProof() && slatepackRecipientAddress.isEmpty()) {
        core::getWndManager()->messageTextDlg("Recipient address",
                                              "Transaction proof is enabled. Please specify the recipient's Slatepack address.");

        return false;
    }

    core::SendCoinsParams sendParams = context->appContext->getSendCoinsParams();

    // !!!! NOTE.  For mobile HODL not a case, first case can be skipped, Directly can be called util->getTxnFee
    QStringList outputs;
    quint64 txnFee = 0;
    bool anyOutputLocked = util::getOutputsToSend( accountPath, sendParams.changeOutputs, amount,
                                  context->wallet, context->appContext,
                                  outputs, &txnFee);
    if (txnFee == 0 && outputs.size() == 0) {
        auto res = util::getTxnFee(accountPath, amount, context->wallet,
                                 context->appContext, sendParams.changeOutputs, outputs);
        txnFee = res.first;
        anyOutputLocked = anyOutputLocked || res.second;
    }
    QString txnFeeStr = util::txnFeeToString(txnFee);

    int ttl_blocks = 1440;
    if ( core::getWndManager()->sendConfirmationSlatepackDlg("Confirm Send Request",
                            "You are sending offline " + (amount < 0 ? "all" : util::nano2one(amount)) +
                           " MWC from account: " + account, 1.0,
                           outputs.size(), &ttl_blocks))  {

        core::SendCoinsParams prms = context->appContext->getSendCoinsParams();

        if (prms.changeOutputs != sendParams.changeOutputs) {
            // Recalculating the outputs and fees. There is a chance that outputs will be different.
            util::getOutputsToSend( accountPath, sendParams.changeOutputs, amount,
                              context->wallet, context->appContext,
                              outputs, &txnFee);
            if (outputs.size() == 0) {
                util::getTxnFee(accountPath, amount, context->wallet,
                                         context->appContext, sendParams.changeOutputs, outputs);
            }
        }

        // For offline, we don't want to specify outputs because because of the late lock.
        if (!anyOutputLocked) {
            outputs.clear();
        }

        // Check signal: sendRespond
        bool sent = context->wallet->sendTo( accountPath,
                "sp_send",
                amount, //  -1  - mean All
                false,
                message, // can be empty, means None
                prms.inputConfirmationNumber,
                "smallest", //  Values: all, smallest. Default: Smallest
                "slatepack",  // Values:  http, file, slatepack, self, mwcmqs
                slatepackRecipientAddress, // Values depends on 'method' Send the transaction to the provided server (start with http://), destination for encrypting/proofs. For method self, dest can point to account name to move
                context->appContext->getGenerateProof(),
                prms.changeOutputs,
                false,
                ttl_blocks, // pass -1 to skip
                false,
                outputs, // Outputs to use. If None, all outputs can be used
                isLockLater,
                0);

        if (!sent) {
            core::getWndManager()->messageTextDlg("Error",
                        "Another Send operation is still in the progress. Please wait until it finished and retry after.");

        }
        return sent;
    }
    else {
        return false;
    }
    return true;
}

void Send::sendRespond( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag ) {
    logger::logInfo(logger::STATE, "Call Send::sendRespond with success=" + QString(success ? "true" : "false") + " error=" + (error.isEmpty() ? "<empty>" : error) + " tx_uuid=" + tx_uuid + " tag=" + tag + " method=" + method);

    if (!success) {
        QString errMsg;
        if (!error.isEmpty()) {
            errMsg = "Your send request has failed:\n\n" + util::mapMessage(error);
        }

        for (auto b : bridge::getBridgeManager()->getSend())
            b->showSendResult(success, errMsg);
       return;
    }

    if (tag=="sp_send") {
        // It is a slatepack. We need to show it. SP data in stored in the wallet directory. Let's read it from there
        QString spFilePath = context->wallet->getWalletDataPath() + "/slatepack/" + tx_uuid + ".send_init.slatepack";
        QStringList sp_data = util::readTextFile( spFilePath );
        QString slatepack = sp_data.size()<1 ? "" : sp_data[0];

        if (slatepack.isEmpty()) {
            core::getWndManager()->messageTextDlg("Invalid data",
                                 "Unable to read the resulting slatepack data from " + spFilePath );
            return;
        }

        context->appContext->addSendSlatepack(tx_uuid, slatepack);

        notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "You successfully generated slatepack " + tx_uuid + " with " + util::nano2one(amount) + " MWC" + (dest.isEmpty() ? "" : " to " + dest) );
        // Let's show the slatepack and enable in place finalization
        core::getWndManager()->pageShowSlatepack( slatepack, tx_uuid, STATE::SEND, ".tx", true );
        return;
    }

    // It is a non slatepack case (send online)
    if (tag == "online_send") {
        if (success && state::getStateMachine()->getCurrentStateId() == STATE::SEND) {
            switchToStartingWindow();
            // Because of switch, need to show success window

            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "You successfully sent slate " + tx_uuid + " with " + util::nano2one(amount) + " MWC to " + dest );
            core::getWndManager()->messageTextDlg("Success", "Your MWC was successfully sent to recipient");
        }
    }

    // Alos we have self send, that will be skipped
}

bool Send::sendMwcOnline( const QString & account, const QString & accountPath, qint64 amount, QString address, const QString & message) {
    logger::logInfo(logger::STATE, "Call Send::sendMwcOnline with accountPath=" + accountPath + " amount=" + QString::number(amount) +
                " address=" + address + " message=" + (message.isEmpty() ? "<empty>" : "<hidden>"));

    while(address.endsWith("/"))
        address = address.left(address.length()-1);

    // Let's  verify address first
    QPair< QString, util::ADDRESS_TYPE > addressRes = util::verifyAddress(address);
    if ( !addressRes.first.isEmpty() ) {
        core::getWndManager()->messageTextDlg("Incorrect Input",
                                         "Please specify a correct address to send your MWC.\n" + addressRes.first );
        return false;
    }

    const wallet::ListenerStatus listenerStatus = context->wallet->getListenerStatus();

    QString send_method;

    switch (addressRes.second) {
        case util::ADDRESS_TYPE::MWC_MQ: {
            if (!context->appContext->isFeatureMWCMQS()) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                        "You are trying to connect to another wallet with MWCMQS, but MWCMQS is disabled for your wallet. Please activate the MWCMQS feature on the wallet configuration page." );
                return false;
            }
            if (!listenerStatus.isMqsHealthy()) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                             "MWCMQS listener is offline. Please check your network connection and firewall settings." );
                return false;
            }
            send_method = "mwcmqs";
            break;
        }
        case util::ADDRESS_TYPE::TOR: {
            if (!context->appContext->isFeatureTor()) {
                    core::getWndManager()->messageTextDlg("Listener is Offline",
                                                          "You are trying to connect to another wallet with Tor, but Tor is disabled for your wallet. Please activate the Tor feature on the wallet configuration page." );
                return false;
            }

            // Let's convert the Tor address to the standard notation that the wallet understands.
            QString tor_pk = util::extractPubKeyFromAddress(address);
            address = "http://" + tor_pk + ".onion";

             if (!listenerStatus.isTorHealthy()) {
                core::getWndManager()->messageTextDlg("Listener is Offline",
                                                      "Tor listener is not healthy. Please check your network connection and firewall settings.");
                return false;
            }

            // Note, legacy naming convention. http&tor are both HTTP. The protocol will be decided from destination address
            send_method = "http";
            break;
        }
        default: // Http is fine.
            send_method = "http";
            break;
    }

    Q_ASSERT(!send_method.isEmpty());

    core::SendCoinsParams sendParams = context->appContext->getSendCoinsParams();

    QStringList outputs;
    quint64 txnFee = 0;
    util::getOutputsToSend( accountPath, sendParams.changeOutputs, amount,
                                  context->wallet, context->appContext,
                                  outputs, &txnFee );
    if (txnFee == 0 && outputs.size() == 0) {
        txnFee = util::getTxnFee( accountPath, amount, context->wallet,
                                  context->appContext, sendParams.changeOutputs, outputs ).first;
    }
    QString txnFeeStr = util::txnFeeToString(txnFee);

    if ( core::getWndManager()->sendConfirmationDlg("Confirm Send Request",
                                        "You are sending " + (amount < 0 ? "all" : util::nano2one(amount)) + " MWC from account: " + account +
                                        "\n\nTo: " + address,
                                        1.0,
                                        outputs.size()) ) {

        core::SendCoinsParams prms = context->appContext->getSendCoinsParams();

        if (prms.changeOutputs != sendParams.changeOutputs) {
            // Recalculating the outputs and fees. There is a chance that outputs will be different.
            util::getOutputsToSend( accountPath, sendParams.changeOutputs, amount,
                              context->wallet, context->appContext,
                              outputs, &txnFee);
            if (outputs.size() == 0) {
                util::getTxnFee(accountPath, amount, context->wallet,
                                         context->appContext, sendParams.changeOutputs, outputs);
            }
        }

        // Check signal: sendRespond
        bool sent = context->wallet->sendTo( accountPath,
                "online_send",
                amount, //  -1  - mean All
                false,
                message, // can be empty, means None
                prms.inputConfirmationNumber,
                "smallest", //  Values: all, smallest. Default: Smallest
                send_method,  // Values:  http, file, slatepack, self, mwcmqs
                util::fullFormalAddress( addressRes.second, address), // Values depends on 'method' Send the transaction to the provided server (start with http://), destination for encrypting/proofs. For method self, dest can point to account name to move
                context->appContext->getGenerateProof(),
                prms.changeOutputs,
                context->appContext->isFluffSet(),
                -1, // pass -1 to skip
                false,
                outputs, // Outputs to use. If None, all outputs can be used
                false, // For online doesn't make sense use late lock feature. Also it will require higher slate version
                0);

        if (!sent) {
            core::getWndManager()->messageTextDlg("Error",
                      "Another Send operation is still in the progress. Please wait until it finished and retry after.");

        }

        return sent;
    }
    return false;
}

QString Send::getSpendAllAmount(QString accountPath) {
    logger::logInfo(logger::STATE, "Call Send::getSpendAllAmount with accountPath=" + accountPath);
    return util::getAllSpendableAmount2(accountPath, context->wallet, context->appContext);
}



QString Send::getHelpDocName() {
    bool isSp = context->appContext->isFeatureSlatepack();
    bool isOnline = context->appContext->isFeatureMWCMQS() || context->appContext->isFeatureTor();

    if (isSp & isOnline)
        return "send_sp_online.html";
    else if (isSp)
        return "send_sp.html";
    else
        return "send_online.html";
}

}

