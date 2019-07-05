#include "x_Resync.h"
#include "../core/appcontext.h"
#include "../core/windowmanager.h"
#include "../state/statemachine.h"
#include "../control/messagebox.h"

namespace state {

Resync::Resync(const StateContext &context) :
            State(context,  STATE::RESYNC ) {

    QObject::connect(context.wallet, &wallet::Wallet::onRecoverProgress,
                     this, &Resync::onRecoverProgress, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onCheckResult,
                     this, &Resync::onCheckResult, Qt::QueuedConnection);

}

Resync::~Resync() {}


NextStateRespond Resync::execute() {
    if ( context.appContext->getActiveWndState() != STATE::RESYNC )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    prevState = context.appContext->pullCookie<int>("PrevState");
    if (prevState<=0)
        prevState = STATE::TRANSACTIONS;

    // Starting recovery process
    prevListeningStatus = context.wallet->getListeningStatus();

    if (prevListeningStatus.first)
        context.wallet->listeningStop(true,false);
    if (prevListeningStatus.second)
        context.wallet->listeningStop(false, true);

    wnd = new wnd::ProgressWnd(
            context.wndManager->getInWndParent(), this, "Re-sync with full node", "Preparing to re-sync", "", false);
    context.wndManager->switchToWindow( wnd );

    context.wallet->check( prevListeningStatus.first || prevListeningStatus.second );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );

}

void Resync::onRecoverProgress( int progress, int maxVal ) {
    if (wnd) {
        wnd->initProgress(0, maxVal);

        maxProgrVal = maxVal;
        QString msgProgress = "Re-sync in progress..." + QString::number(progress * 100 / maxVal) + "%";
        wnd->updateProgress(progress, msgProgress);
        wnd->setMsgPlus("");
    }
}

void Resync::onCheckResult(bool ok, QString errors ) {

    if (prevListeningStatus.first)
        context.wallet->listeningStart(true,false);
    if (prevListeningStatus.second)
        context.wallet->listeningStart(false,true);

    if (wnd) {
        wnd->updateProgress(maxProgrVal, ok? "Done" : "Failed" );
    }

    if (ok)
        control::MessageBox::message(nullptr, "Success", "Account re-sync was finished successfully.");
    else
        control::MessageBox::message(nullptr, "Failure", "Account re-sync was failed.\n" + errors);

    if (context.appContext->getActiveWndState() == STATE::RESYNC ) {
        context.stateMachine->setActionWindow( (STATE)prevState );
    }

}


}