#include "outputs.h"
#include "../windows/outputs_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>

namespace state {


Outputs::Outputs(const StateContext & context) :
    State(context, STATE::OUTPUTS)
{
}

Outputs::~Outputs() {}

NextStateRespond Outputs::execute() {
    if (context.appContext->getActiveWndState() != STATE::OUTPUTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::Outputs( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


// Current transactions that wallet has
QVector<wallet::WalletOutput> Outputs::getOutputs() {
    return context.wallet->getOutputs();
}

QString Outputs::getCurrentAccountName() const {
    return context.wallet->getWalletBalance().accountName;
}

// IO for columns widhts
QVector<int> Outputs::getColumnsWidhts() const {
    return context.appContext->getIntVectorFor("OutputsTblColWidth");
}

void Outputs::updateColumnsWidhts(const QVector<int> & widths) {
    context.appContext->updateIntVectorFor("OutputsTblColWidth", widths);
}


}
