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

QVector<wallet::WalletTransaction>& CachedTransactionInfo::requestTransactions(int offset, int number)
{
    requestedTransactions.clear();

    // offset is the number of the first transaction to display (txn numbers start with 0)
    // number is the number of transactions to display
    if (offset >= 0 && offset < transactions.size())
    {
        int count = number;
        if ((offset + number) > transactions.size())
        {
            count = transactions.size() - offset;
        }
        requestedTransactions.resize(count);

        int j = offset;
        for (int i=0; i < count; i++)
        {
            requestedTransactions[i] = transactions[j++];
        }
    }
    return requestedTransactions;
}

Transactions::Transactions( StateContext * context) :
    State(context, STATE::TRANSACTIONS)
{
    // Let's use static connections for proofes. It will be really the only listener
    QObject::connect( context->wallet, &wallet::Wallet::onExportProof, this, &Transactions::updateExportProof, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onVerifyProof, this, &Transactions::updateVerifyProof, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onCancelTransacton, this, &Transactions::onCancelTransacton, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Transactions::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onTransactionCount, this, &Transactions::updateTransactionCount, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onTransactions, this, &Transactions::updateTransactions, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onTransactionById, this, &Transactions::onTransactionById, Qt::QueuedConnection );

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

void Transactions::requestTransactionCount(QString account) {
    cachedTxs.resetCache(account);
    context->wallet->getTransactionCount(account);
}

// Current transactions that wallet has
void Transactions::requestTransactions(QString account, int offset, int number, bool enforceSync) {
    // the transaction count needs to always be requested first
    Q_ASSERT(cachedTxs.totalTransactions >= 0);
    Q_ASSERT(cachedTxs.currentAccount == account);

    if (enforceSync)
    {
        // request all transactions so we can cache them
        context->wallet->getTransactions(account, 0, std::max(1,cachedTxs.totalTransactions), enforceSync);
        context->wallet->updateWalletBalance(false,false); // With transactions refresh, need to update the balance
        cachedTxs.saveTransactionsRequest(offset, number);
    }
    else if (wnd)
    {
        // pass the subset of transactions the windows needs
        QVector<wallet::WalletTransaction>& wndTxs = cachedTxs.requestTransactions(offset, number);
        wnd->setTransactionData(account, cachedTxs.height, wndTxs);
    }
}

void Transactions::updateTransactionCount(QString account, int number) {
    cachedTxs.totalTransactions = number;
    if (wnd) {
        wnd->setTransactionCount(account, number);
    }
}


void Transactions::updateTransactions( QString account, int64_t height, QVector<wallet::WalletTransaction> transactions) {
    cachedTxs.transactions = transactions;
    cachedTxs.height = height;

    if (wnd) {
        QVector<wallet::WalletTransaction>& wndTxs = cachedTxs.requestTransactions(cachedTxs.requestedOffset, cachedTxs.requestedCount);
        wnd->setTransactionData(account, height, wndTxs);
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


}
