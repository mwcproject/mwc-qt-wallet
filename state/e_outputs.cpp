#include "e_outputs.h"
#include "../windows/e_outputs_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {


Outputs::Outputs(const StateContext & context) :
    State(context, STATE::OUTPUTS)
{
    connect( context.wallet, &wallet::Wallet::onOutputs, this, &Outputs::onOutputs );
}

Outputs::~Outputs() {}

NextStateRespond Outputs::execute() {
    if (context.appContext->getActiveWndState() != STATE::OUTPUTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    wnd = new wnd::Outputs( context.wndManager->getInWndParent(), this);
    context.wndManager->switchToWindow( wnd );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// request wallet for outputs
void Outputs::requestOutputs() {
    context.wallet->getOutputs();
    // Respond:  onOutputs(...)
}

void Outputs::onOutputs( QString account, long height, QVector<wallet::WalletOutput> outputs) {
    if (wnd) {
        wnd->setOutputsData(account,height, outputs);
    }
}

void Outputs::switchCurrentAccount(const wallet::AccountInfo & account) {
    // Switching without expected feedback.   Possible error will be cought by requestTransactions.
    context.wallet->switchAccount( account.accountName );
}

QVector<wallet::AccountInfo> Outputs::getWalletBalance() {
    return context.wallet->getWalletBalance();
}

QString Outputs::getCurrentAccountName() const {
    return context.wallet->getCurrentAccountName();
}

// IO for columns widhts
QVector<int> Outputs::getColumnsWidhts() const {
    return context.appContext->getIntVectorFor("OutputsTblColWidth");
}

void Outputs::updateColumnsWidhts(const QVector<int> & widths) {
    context.appContext->updateIntVectorFor("OutputsTblColWidth", widths);
}


}
