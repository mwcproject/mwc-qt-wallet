#include "e_outputs.h"
#include "../windows/e_outputs_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include <QDebug>

namespace state {


Outputs::Outputs(StateContext * context) :
    State(context, STATE::OUTPUTS)
{
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Outputs::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onOutputs, this, &Outputs::onOutputs );
    QObject::connect( context->wallet, &wallet::Wallet::onOutputCount, this, &Outputs::onOutputCount );

}

Outputs::~Outputs() {}

NextStateRespond Outputs::execute() {
    if (context->appContext->getActiveWndState() != STATE::OUTPUTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd = (wnd::Outputs*) context->wndManager->switchToWindowEx( new wnd::Outputs( context->wndManager->getInWndParent(), this) );
        // Requesting wallet balance update because Accounts into is there
        context->wallet->updateWalletBalance();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Outputs::requestOutputCount(QString account) {
    context->wallet->getOutputCount(account);
}

// request wallet for outputs
void Outputs::requestOutputs(QString account, int offset, int number) {
    context->wallet->getOutputs(account, offset, number);
    // Respond:  onOutputs(...)
}

void Outputs::onOutputCount(QString account, int count) {
    if (wnd) {
        wnd->setOutputCount(account, count);
    }
}

void Outputs::onOutputs( QString account, int64_t height, QVector<wallet::WalletOutput> outputs) {
    qDebug() << "state onOutputs call for wnd=" << wnd;
    if (wnd) {
        wnd->setOutputsData(account,height, outputs);
    }
}

void Outputs::switchCurrentAccount(const wallet::AccountInfo & account) {
    // Switching without expected feedback.   Possible error will be cought by requestTransactions.
    context->wallet->switchAccount( account.accountName );
}

QVector<wallet::AccountInfo> Outputs::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

QString Outputs::getCurrentAccountName() const {
    return context->wallet->getCurrentAccountName();
}

// IO for columns widhts
QVector<int> Outputs::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("OutputsTblColWidth");
}

void Outputs::updateColumnsWidhts(const QVector<int> & widths) {
    context->appContext->updateIntVectorFor("OutputsTblColWidth", widths);
}

void Outputs::onWalletBalanceUpdated() {
    if (wnd) {
        wnd->updateWalletBalance();
    }
}


}
