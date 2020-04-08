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

#include "e_transactions.h"
#include "windows/e_transactions_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../core/global.h"

namespace state {

Transactions::Transactions( StateContext * context) :
    State(context, STATE::TRANSACTIONS)
{
    // Let's use static connections for proofes. It will be really the only listener
    QObject::connect( context->wallet, &wallet::Wallet::onExportProof, this, &Transactions::updateExportProof, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onVerifyProof, this, &Transactions::updateVerifyProof, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onCancelTransacton, this, &Transactions::onCancelTransacton, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Transactions::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onTransactions, this, &Transactions::updateTransactions, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onTransactionById, this, &Transactions::onTransactionById, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onNodeStatus, this, &Transactions::onUpdateNodeStatus, Qt::QueuedConnection);

    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                      this, &Transactions::onNewNotificationMessage, Qt::QueuedConnection );
}

Transactions::~Transactions() {}

NextStateRespond Transactions::execute() {
    if (context->appContext->getActiveWndState() != STATE::TRANSACTIONS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd = (wnd::Transactions*)context->wndManager->switchToWindowEx( mwc::PAGE_E_TRANSACTION,
                new wnd::Transactions( context->wndManager->getInWndParent(), this, context->wallet->getWalletConfig() ) );
    }
    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
};

QString Transactions::getCurrentAccountName() const {
    return context->wallet->getCurrentAccountName();
}

void Transactions::cancelTransaction(const wallet::WalletTransaction & transaction) {
    if (!transaction.isValid()) {
        Q_ASSERT(false);
        return;
    }

    context->wallet->cancelTransacton(transaction.txIdx);
}

void Transactions::switchCurrentAccount(const wallet::AccountInfo & account) {
    // Switching without expected feedback.   Possible error will be cought by requestTransactions.
    context->wallet->switchAccount( account.accountName );
}

// Current transactions that wallet has
void Transactions::requestTransactions(QString account, bool enforceSync) {
    // the transaction count needs to always be requested first
    if (cachedTxs.currentAccount != account) {
        cachedTxs.resetCache(account);
        enforceSync = true;
    }

    if (enforceSync)
    {
        // request node status so we can get the node height to use when calculating confirmations
        context->wallet->getNodeStatus();
        // request all transactions so we can cache them
        context->wallet->getTransactions(account, enforceSync);
        context->wallet->updateWalletBalance(false,false); // With transactions refresh, need to update the balance
    }
    else if (wnd)
    {
        // pass the subset of transactions the windows needs
        wnd->setTransactionData(account, cachedTxs.height, cachedTxs.transactions);
    }
}

void Transactions::updateTransactions( QString account, int64_t height, QVector<wallet::WalletTransaction> transactions) {
    if (cachedTxs.currentAccount != account) {
        Q_ASSERT(false); // rarely possible if there are many requests in the Q
        return;
    }

    cachedTxs.setCache(account, height, transactions);

    if (wnd) {
        wnd->setTransactionCount(account, cachedTxs.transactions.size());
        wnd->setTransactionData(account, height, cachedTxs.transactions);
    }
}

// Proofs
void Transactions::generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName ) {
    context->wallet->generateMwcBoxTransactionProof(transactionId, resultingFileName);
    // respond will be async
}

void Transactions::verifyMwcBoxTransactionProof( QString proofFileName ) {
    context->wallet->verifyMwcBoxTransactionProof(proofFileName);
    // respond will be async
}

void Transactions::updateExportProof( bool success, QString fn, QString msg ) {
    if (wnd) {
        wnd->showExportProofResults(success, fn, msg );
    }
}

void Transactions::updateVerifyProof( bool success, QString fn, QString msg ) {
    if (wnd) {
        wnd->showVerifyProofResults( success, fn, msg );
    }
}


QVector<int> Transactions::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("TransTblColWidth");
}

void Transactions::updateColumnsWidhts(const QVector<int> & widths) {
    context->appContext->updateIntVectorFor("TransTblColWidth", widths);
}

QString Transactions::getProofFilesPath() const {
    return context->appContext->getPathFor("Transactions");
}

void Transactions::updateProofFilesPath(QString path) {
    context->appContext->updatePathFor("Transactions", path);
}

QVector<wallet::AccountInfo> Transactions::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

void Transactions::onCancelTransacton( bool success, int64_t trIdx, QString errMessage ) {
    if (success)
        context->wallet->updateWalletBalance(false,false); // Updating balance, Likely something will be unblocked

    if (wnd) {
        wnd->updateCancelTransacton(success, trIdx, errMessage);
    }
}

void Transactions::onWalletBalanceUpdated() {
    if (wnd) {
        wnd->updateWalletBalance();
    }
}

void Transactions::onNewNotificationMessage(notify::MESSAGE_LEVEL  level, QString message) {
    Q_UNUSED(level)

    if (wnd && message.contains("Changing transaction")) {
        wnd->triggerRefresh();
    }
}


// Request full info for the transaction
void Transactions::getTransactionById(QString account, int64_t txIdx) const {
    context->wallet->getTransactionById(account, txIdx);
}

void Transactions::onTransactionById( bool success, QString account, int64_t height,
        wallet::WalletTransaction transaction,
        QVector<wallet::WalletOutput> outputs,
        QVector<QString> messages ) {
    if (wnd) {
        wnd->updateTransactionById(success, account, height, transaction, outputs, messages);
    }
}

void Transactions::onUpdateNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    if (wnd) {
        // gather settings needed to calculate transaction confirmations
        int64_t height = 0;
        if (online && connections > 0 && totalDifficulty > 0 && (nodeHeight > peerHeight - mwc::NODE_HEIGHT_DIFF_LIMIT)) {
            // node is online and synced, use the current node height to determine transaction confirmations
            height = nodeHeight;
        }
        int confirmNumber = context->appContext->getSendCoinsParams().inputConfirmationNumber;
        wnd->setConfirmData(height, confirmNumber);
    }
}


}
