#include "transactions.h"
#include "../windows/transactions_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

Transactions::Transactions(const StateContext & context) :
    State(context, STATE::TRANSACTIONS)
{
}

Transactions::~Transactions() {}

NextStateRespond Transactions::execute() {
    if (context.appContext->getActiveWndState() != STATE::TRANSACTIONS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    wnd = new wnd::Transactions( context.wndManager->getInWndParent(), this );
    context.wndManager->switchToWindow( wnd );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
};

QString Transactions::getCurrentAccountName() const {
    return context.wallet->getCurrentAccountName();
}


// Current transactions that wallet has
void Transactions::requestTransactions() {

//    connect ....

    context.wallet->getTransactions();
}

// Proofs
wallet::WalletProofInfo Transactions::generateMwcBoxTransactionProof( long transactionId, QString resultingFileName ) {
    return context.wallet->generateMwcBoxTransactionProof(transactionId, resultingFileName);
}

wallet::WalletProofInfo  Transactions::verifyMwcBoxTransactionProof( QString proofFileName ) {
    return context.wallet->verifyMwcBoxTransactionProof(proofFileName);
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


}
