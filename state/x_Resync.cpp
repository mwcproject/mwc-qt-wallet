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
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/z_progresswnd_b.h"

namespace state {

Resync::Resync( StateContext * context) :
            State(context,  STATE::RESYNC ) {

    QObject::connect(context->wallet, &wallet::Wallet::onRecoverProgress,
                     this, &Resync::onRecoverProgress, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onCheckResult,
                     this, &Resync::onCheckResult, Qt::QueuedConnection);

    inSyncProcess = false;
}

Resync::~Resync() {}


NextStateRespond Resync::execute() {
    if ( context->appContext->getActiveWndState() != STATE::RESYNC ) {
        inSyncProcess = false;
        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    prevState = context->appContext->pullCookie<int>("PrevState");
    if (prevState<=0 || prevState==STATE::RESYNC)
        prevState = STATE::TRANSACTIONS;

    // Starting recovery process
    prevListeningStatus = context->wallet->getListenerStartState();

    context->wallet->listeningStop(prevListeningStatus.mqs, prevListeningStatus.tor);

    core::getWndManager()->pageProgressWnd(mwc::PAGE_X_RESYNC, RESYNC_CALLER_ID,
            "Re-sync with full node", "Preparing to re-sync", "", false);

    respondCounter = 0;
    respondZeroLevel = 0;
    progressBase = 0;
    inSyncProcess = true;

    // We can't really be blocked form resync. Result will be looping with locking screen
    context->stateMachine->blockLogout("Resync");

    context->wallet->check( prevListeningStatus.mqs || prevListeningStatus.tor );
    // Check does full resync, no needs to 'sync'
    context->wallet->updateWalletBalance(false,false);

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Resync::exitingState() {
    context->stateMachine->unblockLogout("Resync");
}

bool Resync::canExitState(STATE nextWindowState) {
    Q_UNUSED(nextWindowState)
    return !inSyncProcess;
}


void Resync::onRecoverProgress( int progress, int maxVal ) {
    for (auto b: bridge::getBridgeManager()->getProgressWnd()) {
        if (b->getCallerId() == RESYNC_CALLER_ID) {
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

            b->initProgress(RESYNC_CALLER_ID, 0, maxVal);

            maxProgrVal = maxVal;
            QString msgProgress = "Re-sync in progress...  " + QString::number(progress * 100.0 / maxVal, 'f',0) + "%";
            b->updateProgress(RESYNC_CALLER_ID, progress, msgProgress);
            b->setMsgPlus(RESYNC_CALLER_ID, "");
        }
    }
}

void Resync::onCheckResult(bool ok, QString errors ) {

    context->wallet->listeningStart(prevListeningStatus.mqs, prevListeningStatus.tor,true);
    prevListeningStatus = wallet::ListenerStatus(); // reset status to all false.

    for (auto b: bridge::getBridgeManager()->getProgressWnd()) {
        if (b->getCallerId() == RESYNC_CALLER_ID) {
            b->updateProgress(RESYNC_CALLER_ID, maxProgrVal, ok? "Done" : "Failed");
        }
    }

    if (ok)
        core::getWndManager()->messageTextDlg("Success", "Account re-sync finished successfully.");
    else
        core::getWndManager()->messageTextDlg("Failure", "Account re-sync failed.\n" + errors);

    inSyncProcess = false;

    if (context->appContext->getActiveWndState() == STATE::RESYNC ) {
        context->stateMachine->setActionWindow( (STATE)prevState );
    }

}

bool Resync::mobileBack() {
    // Blocking back while in sync
    return inSyncProcess;
}


}
