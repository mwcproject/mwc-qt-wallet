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

#include "x_Resync.h"
#include "../core/appcontext.h"
#include "../core/windowmanager.h"
#include "../state/statemachine.h"
#include "../control/messagebox.h"
#include "../core/global.h"

namespace state {

Resync::Resync( StateContext * context) :
            State(context,  STATE::RESYNC ) {

    QObject::connect(context->wallet, &wallet::Wallet::onRecoverProgress,
                     this, &Resync::onRecoverProgress, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onCheckResult,
                     this, &Resync::onCheckResult, Qt::QueuedConnection);

}

Resync::~Resync() {}


NextStateRespond Resync::execute() {
    if ( context->appContext->getActiveWndState() != STATE::RESYNC )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    prevState = context->appContext->pullCookie<int>("PrevState");
    if (prevState<=0)
        prevState = STATE::TRANSACTIONS;

    // Starting recovery process
    prevListeningStatus = context->wallet->getListeningStatus();

    if (prevListeningStatus.first)
        context->wallet->listeningStop(true,false);
    if (prevListeningStatus.second)
        context->wallet->listeningStop(false, true);

    wnd = (wnd::ProgressWnd*)context->wndManager->switchToWindowEx( mwc::PAGE_X_RESYNC,
            new wnd::ProgressWnd( context->wndManager->getInWndParent(), this, "Re-sync with full node", "Preparing to re-sync", "", false) );

    respondCounter = 0;
    respondZeroLevel = 0;
    progressBase = 0;

    // We can't really be blocked form resync. Result will be looping with locking screen
    context->stateMachine->blockLogout();

    context->wallet->check( prevListeningStatus.first || prevListeningStatus.second );
    context->wallet->updateWalletBalance();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );

}

void Resync::exitingState() {
    context->stateMachine->unblockLogout();
}


void Resync::onRecoverProgress( int progress, int maxVal ) {
    if (wnd) {

        if (respondCounter<3) {
            respondCounter++;
            respondZeroLevel = progress;
            progressBase = maxVal / 100 * respondCounter;

            progress = respondCounter;
            maxVal = 100;
        }
        else {
            progress -= respondZeroLevel;
            maxVal -= respondZeroLevel;
            progress += progressBase;
            maxVal += progressBase;
        }

        wnd->initProgress(0, maxVal);

        maxProgrVal = maxVal;
        QString msgProgress = "Re-sync in progress..." + QString::number(progress * 100 / maxVal) + "%";
        wnd->updateProgress(progress, msgProgress);
        wnd->setMsgPlus("");
    }
}

void Resync::onCheckResult(bool ok, QString errors ) {

    if (prevListeningStatus.first)
        context->wallet->listeningStart(true,false, true);
    if (prevListeningStatus.second)
        context->wallet->listeningStart(false,true, true);

    if (wnd) {
        wnd->updateProgress(maxProgrVal, ok? "Done" : "Failed" );
    }

    if (ok)
        control::MessageBox::message(nullptr, "Success", "Account re-sync finished successfully.");
    else
        control::MessageBox::message(nullptr, "Failure", "Account re-sync failed.\n" + errors);

    if (context->appContext->getActiveWndState() == STATE::RESYNC ) {
        context->stateMachine->setActionWindow( (STATE)prevState );
    }

}


}
