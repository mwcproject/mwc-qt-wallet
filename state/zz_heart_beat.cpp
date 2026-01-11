// Copyright 2025 The MWC Developers
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

#include "zz_heart_beat.h"

#include <QJsonArray>
#include "statemachine.h"
#include "bridge/BridgeManager.h"
#include "bridge/heartbeat_b.h"
#include "bridge/wnd/u_nodeInfo_b.h"
#include "core/appcontext.h"
#include "core/Notification.h"
#include "core/WndManager.h"
#include "node/node_client.h"
#include "../util/Log.h"
#include "bridge/wnd/z_progresswnd_b.h"
#include "core/WalletApp.h"
#include "core/global.h"
#include "core_desktop/windowmanager.h"

namespace state {

HeartBeat::HeartBeat(StateContext * _context) :
            State(_context, STATE::HEART_BEAT ) {

    lastNodeIsHealty = true;

    QObject::connect(context->wallet, &wallet::Wallet::onLogin,
                     this, &HeartBeat::onLogin, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onLogout,
                     this, &HeartBeat::onLogout, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onSlateReceivedFrom,
                 this, &HeartBeat::onSlateReceivedFrom, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onScanProgress,
                 this, &HeartBeat::onScanProgress, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onScanDone,
                 this, &HeartBeat::onScanDone, Qt::QueuedConnection);

    // Checking/update node status every 20 seconds...
    startTimer(3000); // Let's update node info every 60 seconds. By some reasons it is slow operation...
}

HeartBeat::~HeartBeat() {

}

NextStateRespond HeartBeat::execute() {
    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}


void HeartBeat::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);

    if (core::WalletApp::isExiting())
        return;

    // nothing is running, nothing to update
    if (!context->wallet->isInit() || context->wallet->isBusy())
        return;

    updateNodeStatus();

    if ( context->wallet->getStartStatus() != wallet::Wallet::STARTED_MODE::NORMAL ) {
           lastNodeIsHealty = true;
           return;
    }

    bool pubNode = context->nodeClient->isUsePublicNode();
    if (lastUsePubNode != pubNode) {
        lastUsePubNode = pubNode;
        notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                              QString("Wallet switched to ") + (pubNode ? "public" : "embedded") + " MWC-Node");
    }

    if ( context->nodeClient->isNodeHealthy() ) {
        if (!lastNodeIsHealty) {
            if (config::isOnlineWallet()) {
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                              "MWC-Node is healthy. Wallet can validate stored data with blockchain.");
            }
            lastNodeIsHealty = true;
        }
    }
    else {
        if (lastNodeIsHealty) {
            if (config::isOnlineWallet()) {
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::WARNING,
                                                  "MWC-Node is offline. Your balance, transactions and output status might be not accurate");
            }
            lastNodeIsHealty = false;
        }
    };

    // Tor/Mqs online status
    wallet::ListenerStatus listeners = context->wallet->getListenerStatus();

    if (listeners.isMqsHealthy() != mqsOnline) {
        if (listeners.isMqsHealthy()) {
            notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO, "Start listening on MWCMQS address");
        }
        else {
            notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO, "Stop listening on MWCMQS address");
        }
    }

    if (listeners.isTorHealthy() != torOnline) {
        if (listeners.isTorHealthy()) {
            notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO, "Start listening on TOR address");
        }
        else {
            notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO, "Stop listening on TOR address");
        }
    }

    if (listeners.isMqsHealthy() != mqsOnline || listeners.isTorHealthy()!=torOnline) {
        mqsOnline = listeners.isMqsHealthy();
        torOnline = listeners.isTorHealthy();
        emitTorMqsStateChange();
    }
}

void HeartBeat::updateNodeStatus() {
    if (context->nodeClient==nullptr)
        return;

    wallet::NodeStatus nodeStatus = context->nodeClient->requestNodeStatus();

    if (justLogin) {
        justLogin = false;
        // Let's consider 5 blocks (5 minutes) unsync be critical issue
        if ( !nodeStatus.isHealthy() ) {
            if ( state::getStateMachine()->getCurrentStateId() != STATE::NODE_INFO) {
                // Switching to this Node Info state. State switch will take care about the rest workflow
                context->stateMachine->setActionWindow( state::STATE::NODE_INFO );
            }
        }
    }

    QString localNodeProgress =  context->nodeClient->getLastInternalNodeState();
    for (auto b : bridge::getBridgeManager()->getHeartBeat())
        b->emitNodeStatus( localNodeProgress, nodeStatus );

    if (lastNodeDifficulty != nodeStatus.totalDifficulty ) {
        lastNodeDifficulty = nodeStatus.totalDifficulty;
        // trigger update because the height was changed
        if (context->wallet->getStartStatus() == wallet::Wallet::STARTED_MODE::NORMAL) {
            context->wallet->update_wallet_state();
        }
        lastNodeHeight = nodeStatus.nodeHeight;
    }
}

void HeartBeat::onLogin() {
    logger::logInfo(logger::STATE, "Call HeartBeat::onLogin");
    lastNodeIsHealty = true;
    justLogin = true;
    mqsOnline = false;
    torOnline = false;
    emitTorMqsStateChange();
}

void HeartBeat::onLogout() {
    logger::logInfo(logger::STATE, "Call HeartBeat::onLogout");
    lastNodeIsHealty = true;
    mqsOnline = false;
    torOnline = false;
    emitTorMqsStateChange();
}

void HeartBeat::onSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message ) {
    logger::logInfo(logger::STATE, "Call HeartBeat::onSlateReceivedFrom with slate=<hidden> mwc=" + QString::number(mwc) + " fromAddr=" + fromAddr + " message=" + message);
    recievedTxs.insert(slate, ReceivedTxInfo(  mwc, fromAddr, message) );
    if (fromAddr == "http listener" || fromAddr == "self") {
        // It is self('self') or slate pack ('http listener') transaction, the real http listener is not supported by QT wallet
        // We don't want to show congrats message to that.
        context->appContext->setShowCongratsForTx(slate);

        if (fromAddr == "http listener") {
            notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                      "You received slate " + slate + " with " + util::nano2one(mwc) + " MWC");
        }
    }
    else {
        notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                  "You received slate " + slate + " with " + util::nano2one(mwc) + " MWC from " + fromAddr);
    }
}

// Waiting for
void HeartBeat::onScanProgress( QString responseId, QJsonObject statusMessage ) {

    if (config::isColdWallet() && statusMessage.contains("Scanning"))
    {
        if (coldWalletSyncState == state::STATE::NONE) {
            coldWalletSyncState = context->stateMachine->getCurrentStateId();
            context->stateMachine->resetLogoutLimit(true);
            // Need reset state because we are messaging with windows. State is not valid any more
            context->stateMachine->resetCurrentState();
            core::getWndManager()->pageProgressWnd(mwc::PAGE_X_RESYNC, responseId,
                                                       "Re-sync with a node", "Preparing to re-sync", "", false);
        }
        else {
            QJsonArray vals = statusMessage["Scanning"].toArray();
            int percent_progress = vals[2].toInt();
            for (auto b: bridge::getBridgeManager()->getProgressWnd()) {
                if (b->getCallerId() == responseId) {
                    b->initProgress(responseId, 0, 100);
                    QString msgProgress = "Re-sync in progress...  " + QString::number(percent_progress) + "%";
                    b->updateProgress(responseId, percent_progress, msgProgress);
                    b->setMsgPlus(responseId, "");
                }
            }
        }
    }

    Q_UNUSED(responseId);
    // {"response_id":"upd_6","status":{"Info":"Changing transaction a2df1fde-ea4d-4cb8-b5fc-1c7384bf7a76 state to confirmed"}}
    if (statusMessage.contains("Info")) {
        QString message = statusMessage["Info"].toString();

        // Tracking events when transactions was confirmed for the first time
        if (message.contains("Changing transaction")) {
            // Message:  Changing transaction c5d7fa2c-6463-4cbb-b42c-c22c195e3ec8 state to confirmed
            if (message.endsWith("state to confirmed")) {
                // Extracting tx UUID: 8 hex digits - 4 hex digits - 4 hex digits - 4 hex digits - 12 hex digits
                QRegularExpression uuidRe(R"(\b[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\b)");

                QRegularExpressionMatch match = uuidRe.match(message);
                if (match.hasMatch()) {
                    // use lowcase for safety. Standard doesn't say the case.
                    QString uuid = match.captured(0).toLower();
                    // Checking is it was reported
                    if (!context->appContext->isShowCongratsForTx(uuid)) {
                        // If it is known tx - we can show the congrats message now.
                        if (recievedTxs.contains(uuid)) {
                            ReceivedTxInfo txInfo = recievedTxs[uuid];
                            recievedTxs.remove(uuid);
                            context->appContext->setShowCongratsForTx(uuid);
                            // Just in case it was a receive slatepack, let's clean it up. Otherwise delete will handle not exist data well.
                            context->appContext->deleteReceiveSlatepack(uuid);
                            core::getWndManager()->messageHtmlDlg("Congratulations!",
                                                  "You received <b>" + util::nano2one(txInfo.mwc) + "</b> MWC<br>" +
                                                  (txInfo.message.isEmpty() ? "" : "Description: " + txInfo.message + "<br>") +
                                                  "<br>From: " + txInfo.fromAddr +
                                                  "<br>Transaction: " + uuid);
                        }
                        else {
                            // Need to request the transaction details to see the amounts...
                            wallet::WalletTransaction transaction = context->wallet->getTransactionByUUID(uuid);
                            if (!transaction.txid.isEmpty()) {
                                context->appContext->setShowCongratsForTx(uuid);
                                if (transaction.transactionType == wallet::WalletTransaction::TRANSACTION_TYPE::RECEIVE) {
                                    core::getWndManager()->messageHtmlDlg("Congratulations!",
                                                                          "You received <b>" + util::nano2one(transaction.credited) + "</b> MWC<br>" +
                                                                          // Message info is lost, can't show anything
                                                                          "<br>From: " + transaction.address +
                                                                          "<br>Transaction: " + uuid);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (statusMessage.contains("Warning")) {
        QString message = statusMessage["Warning"].toString();

        if (message.contains("Please wait") || message.contains("Changing status for output") ||
            message.contains("Changing status for coin base") ||
            message.contains("Deleting unconfirmed Output not mapped to any transaction") ||
            message.contains("We detected transaction collision on")) {
        }
        else if (message.contains("exists in UTXO set but not in wallet. Restoring.")) {
            // Confirmed output for 9979984000 with ID 030000000000000000000001b300184bb3 (Commitment(0874201d3b7ded5380fc4d904aa2c88a6e31768833c6a5dc543d25c7f800996762), index 3321549) exists in UTXO set but not in wallet. Restoring.
            // 9979984000 here is amount, neew to emit notification about that.
            int idx1 = strlen("Confirmed output for ");
            int idx2 = message.indexOf("with");
            Q_ASSERT(idx1<idx2);
            if (idx1<idx2) {
                QString amount = message.mid(idx1, idx2-idx1).trimmed();
                QString mwc = util::nano2one( amount.toLongLong() );
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::CRITICAL, "Restored unknown commit for " + mwc + " MWC" );
            }
        }
        else {
            notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::WARNING, message);
        }
    }

}

void HeartBeat::onScanDone( QString responseId, bool fullScan, int height, QString errorMessage ) {
    Q_UNUSED(responseId);
    Q_UNUSED(fullScan);
    Q_UNUSED(height);
    Q_UNUSED(errorMessage);

    if (config::isColdWallet()) {
        if (coldWalletSyncState != state::STATE::NONE) {
            context->stateMachine->resetLogoutLimit(true);
            context->stateMachine->setActionWindow(coldWalletSyncState, true);
            coldWalletSyncState = state::STATE::NONE;
        }
    }

    if (!errorMessage.isEmpty()) {
        logger::logError(logger::QT_WALLET, "Scan finished with error: " + errorMessage );
        return;
    }


    if (lastNodeHeight > height) {
        // retry to finish the update.
        context->wallet->update_wallet_state();
    }
    else {
        // Update is done - notify all that balance can be recalculated
        context->wallet->emitWalletBalanceUpdated();
    }
}

void HeartBeat::emitTorMqsStateChange() {
    logger::logInfo(logger::STATE, QString("Call HeartBeat::emitTorMqsStateChange with mqsOnline=") + (mqsOnline ? "true" : "false") +
        " torOnline=" + (torOnline ? "true" : "false"));
    for (auto br : bridge::getBridgeManager()->getHeartBeat()) {
        br->sgnUpdateListenerStatus(mqsOnline, torOnline);
    }
}


}
