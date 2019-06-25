#include "e_transactions.h"
#include "windows/e_transactions_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"

namespace state {

Transactions::Transactions(const StateContext & context) :
    State(context, STATE::TRANSACTIONS)
{
    // Let's use static connections for proofes. It will be really the only listener
    QObject::connect( context.wallet, &wallet::Wallet::onExportProof, this, &Transactions::updateExportProof, Qt::QueuedConnection );
    QObject::connect( context.wallet, &wallet::Wallet::onVerifyProof, this, &Transactions::updateVerifyProof, Qt::QueuedConnection );

    QObject::connect( context.wallet, &wallet::Wallet::onCancelTransacton, this, &Transactions::onCancelTransacton, Qt::QueuedConnection );
    QObject::connect( context.wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Transactions::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect( context.wallet, &wallet::Wallet::onTransactions, this, &Transactions::updateTransactions, Qt::QueuedConnection );

}

Transactions::~Transactions() {}

NextStateRespond Transactions::execute() {
    if (context.appContext->getActiveWndState() != STATE::TRANSACTIONS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    wnd = new wnd::Transactions( context.wndManager->getInWndParent(), this );
    context.wndManager->switchToWindow( wnd );

    // Requesting wallet balance update because Accounts into is there
    context.wallet->updateWalletBalance();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
};

QString Transactions::getCurrentAccountName() const {
    return context.wallet->getCurrentAccountName();
}

void Transactions::cancelTransaction(const wallet::WalletTransaction & transaction) {
    if (!transaction.isValid()) {
        Q_ASSERT(false);
        return;
    }

    context.wallet->cancelTransacton(transaction.txIdx);
}

void Transactions::switchCurrentAccount(const wallet::AccountInfo & account) {
    // Switching without expected feedback.   Possible error will be cought by requestTransactions.
    context.wallet->switchAccount( account.accountName );
}


// Current transactions that wallet has
void Transactions::requestTransactions() {

    context.wallet->getTransactions();
}

void Transactions::updateTransactions( QString account, long height, QVector<wallet::WalletTransaction> transactions) {
    if (wnd) {
        wnd->setTransactionData(account, height, transactions);
    }
}

// Proofs
void Transactions::generateMwcBoxTransactionProof( long transactionId, QString resultingFileName ) {
    context.wallet->generateMwcBoxTransactionProof(transactionId, resultingFileName);
    // respond will be async
}

void Transactions::verifyMwcBoxTransactionProof( QString proofFileName ) {
    context.wallet->verifyMwcBoxTransactionProof(proofFileName);
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
    return context.appContext->getIntVectorFor("TransTblColWidth");
}

void Transactions::updateColumnsWidhts(const QVector<int> & widths) {
    context.appContext->updateIntVectorFor("TransTblColWidth", widths);
}

QString Transactions::getProofFilesPath() const {
    return context.appContext->getPathFor("Transactions");
}

void Transactions::updateProofFilesPath(QString path) {
    context.appContext->updatePathFor("Transactions", path);
}

QVector<wallet::AccountInfo> Transactions::getWalletBalance() {
    return context.wallet->getWalletBalance();
}


void Transactions::onCancelTransacton( bool success, long trIdx, QString errMessage ) {
    if (success)
        context.wallet->updateWalletBalance(); // Updating balance, Likely something will be unblocked

    if (wnd) {
        wnd->updateCancelTransacton(success, trIdx, errMessage);
    }
}

void Transactions::onWalletBalanceUpdated() {
    if (wnd) {
        wnd->updateWalletBalance();
    }
}


}
