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

#include <QJsonArray>

#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/z_progresswnd_b.h"
#include "../util/Log.h"
#include "wallet/tasks/StartStopListeners.h"

namespace state {

Resync::Resync( StateContext * context) :
            State(context,  STATE::RESYNC ) {

    QObject::connect(context->wallet, &wallet::Wallet::onScanProgress,
                     this, &Resync::onScanProgress, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onScanDone,
                     this, &Resync::onScanDone, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onStartStopListenersDone,
                     this, &Resync::onStartStopListenersDone, Qt::QueuedConnection);

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
    bool scheduled = context->wallet->listeningStop(true, true);
    scanRespId = "";

    // Next should start Scan when
    if (!scheduled) {
        start_scanning();
        Q_ASSERT(!scanRespId.isEmpty());
    }

    core::getWndManager()->pageProgressWnd(mwc::PAGE_X_RESYNC, scanRespId,
            "Re-sync with full node", "Stopping Listeners, please wait...", "", false);

    respondCounter = 0;
    respondZeroLevel = 0;
    progressBase = 0;
    inSyncProcess = true;

    // We can't really be blocked form resync. Result will be looping with locking screen
    context->stateMachine->blockLogout("Resync");

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Resync::start_scanning() {
    Q_ASSERT(scanRespId.isEmpty());
    scanRespId = context->wallet->scan(true);

    core::getWndManager()->pageProgressWnd(mwc::PAGE_X_RESYNC, scanRespId,
            "Re-sync with full node", "Preparing to re-sync", "", false);
}

void Resync::exitingState() {
    logger::logInfo(logger::STATE, "Call Resync::exitingState");
    context->stateMachine->unblockLogout("Resync");
}

bool Resync::canExitState(STATE nextWindowState) {
    logger::logInfo(logger::STATE, "Call Resync::canExitState with nextWindowState=" + QString::number(nextWindowState));
    Q_UNUSED(nextWindowState)
    return !inSyncProcess;
}

void Resync::onStartStopListenersDone(int operation) {
    if (operation & (LISTENER_TOR_STOP | LISTENER_MQS_STOP) ) {
        if (inSyncProcess && scanRespId.isEmpty()) {
            start_scanning();
            Q_ASSERT(!scanRespId.isEmpty());
        }
    }
}

void Resync::onScanProgress( QString responseId, QJsonObject statusMessage ) {
    if (responseId!=scanRespId)
        return;

    if (statusMessage.contains("Scanning")) {
        QJsonArray vals = statusMessage["Scanning"].toArray();
        QString message = vals[1].toString();
        int percent_progress = vals[2].toInt();

        for (auto b: bridge::getBridgeManager()->getProgressWnd()) {
            if (b->getCallerId() == scanRespId) {
                int maxVal = 100;
                if (respondCounter<3) {
                    respondCounter++;
                    respondZeroLevel = percent_progress;

                    percent_progress = respondCounter;
                }
                else {
                    percent_progress -= respondZeroLevel;
                    maxVal -= respondZeroLevel;
                    percent_progress += 2;
                    maxVal += 2;
                }

                b->initProgress(scanRespId, 0, maxVal);

                maxProgrVal = maxVal;
                QString msgProgress = "Re-sync in progress...  " + QString::number(percent_progress * 100.0 / maxVal, 'f',0) + "%";
                b->updateProgress(scanRespId, percent_progress, msgProgress);
                b->setMsgPlus(scanRespId, "");
            }
        }
    }
}

void Resync::onScanDone( QString responseId, bool fullScan, int height, QString errorMessage ) {
    Q_UNUSED(fullScan);
    Q_UNUSED(height);

    if (responseId!=scanRespId)
        return;

    context->wallet->listeningStart( context->appContext->isFeatureMWCMQS(), context->appContext->isFeatureTor());

    for (auto b: bridge::getBridgeManager()->getProgressWnd()) {
        if (b->getCallerId() == scanRespId) {
            b->updateProgress(scanRespId, maxProgrVal, errorMessage.isEmpty() ? "Done" : "Failed");
        }
    }

    if (errorMessage.isEmpty())
        core::getWndManager()->messageTextDlg("Success", "Account re-sync finished successfully.");
    else
        core::getWndManager()->messageTextDlg("Failure", "Account re-sync failed.\n\n" + errorMessage);

    inSyncProcess = false;

    if (context->appContext->getActiveWndState() == STATE::RESYNC ) {
        context->stateMachine->setActionWindow( (STATE)prevState );
    }
}

bool Resync::mobileBack() {
    logger::logInfo(logger::STATE, "Call Resync::mobileBack");
    // Blocking back while in sync
    return inSyncProcess;
}


}
