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

#include "x_events.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/x_events_b.h"

namespace state {

Events::Events(StateContext * context):
    State(context, STATE::EVENTS)
{
    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
            this, &Events::onNewNotificationMessage, Qt::QueuedConnection );

    QObject::connect(context->wallet, &wallet::Wallet::onTransactionById,
                 this, &Events::onTransactionById, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onSlateReceivedFrom,
                 this, &Events::onSlateReceivedFrom, Qt::QueuedConnection);
}

Events::~Events() {}

NextStateRespond Events::execute() {
    if (context->appContext->getActiveWndState() != STATE::EVENTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    core::getWndManager()->pageEvents();

    for (auto b : bridge::getBridgeManager()->getEvents())
        b->updateNonShownWarnings(false);

    messageWaterMark = QDateTime::currentMSecsSinceEpoch();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Events::eventsWndIsDeleted()
{
    messageWaterMark = QDateTime::currentMSecsSinceEpoch();
}

// Historical design. UI can call this method now, but not in the past
QVector<notify::NotificationMessage> Events::getWalletNotificationMessages() {
    return notify::getNotificationMessages();
}

void Events::onNewNotificationMessage(bridge::MESSAGE_LEVEL  level, QString message) {
    for (auto b : bridge::getBridgeManager()->getEvents())
        b->updateShowMessages();

    if ( state::getStateMachine()->getCurrentStateId() != STATE::EVENTS ) {
        if ( notify::NotificationMessage::isCritical(level) ) {
            for (auto b : bridge::getBridgeManager()->getEvents())
                b->updateNonShownWarnings(true);
        }
    }

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
                        core::getWndManager()->messageHtmlDlg("Congratulations!",
                                              "You received <b>" + txInfo.mwc + "</b> MWC<br>" +
                                              (txInfo.message.isEmpty() ? "" : "Description: " + txInfo.message + "<br>") +
                                              "<br>From: " + txInfo.fromAddr +
                                              "<br>Transaction: " + uuid);
                    }
                    else {
                        // Need to request the transaction details to see the amounts...
                        activeUUID.insert(uuid);
                        for (const wallet::AccountInfo & account : context->wallet->getWalletBalance()) {
                            context->wallet->getTransactionById(account.accountName, uuid);
                        }
                    }
                }
            }
        }
    }
}

void Events::onSlateReceivedFrom(QString slate, QString mwc, QString fromAddr, QString message ) {
    recievedTxs.insert(slate, ReceivedTxInfo(mwc, fromAddr, message) );
    if (fromAddr == "http listener") {
        // It is self transaction, http listener is not supported by QT wallet
        // We don't want to show congrats message to that.
        context->appContext->setShowCongratsForTx(slate);
    }
}

void Events::onTransactionById( bool success, QString account, int64_t height, wallet::WalletTransaction transaction,
                                QVector<wallet::WalletOutput> outputs, QVector<QString> messages ) {
    if (!success)
        return;

    QString uuid = transaction.txid.toLower();
    if (activeUUID.contains(uuid)) {
        activeUUID.remove(uuid);
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


// Check if some error/warnings need to be shown
bool Events::hasNonShownWarnings() const {
    QVector<notify::NotificationMessage> msgs = notify::getNotificationMessages();

    for ( int i = msgs.size()-1; i>=0; i-- ) {
        if (msgs[i].time.toMSecsSinceEpoch() <= messageWaterMark )
            return false;

        if ( msgs[i].isCritical() )
            return true;
    }

    return false;
}


}
