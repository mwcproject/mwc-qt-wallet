// Copyright 2021 The MWC Developers
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

//
// Created by Konstantin Bay on 9/22/24.
//

#include "v_ViewOutputs_s.h"
#include "../core/WndManager.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/z_progresswnd_b.h"
#include "../bridge/wnd/v_viewoutputs_b.h"

namespace state {

ViewOutputs::ViewOutputs(StateContext * context) :
        State(context, STATE::VIEW_ACCOUNTS)
{
    QObject::connect(context->wallet, &wallet::Wallet::onRecoverProgress,
                     this, &ViewOutputs::onRecoverProgress, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onScanRewindHash,
                     this, &ViewOutputs::onScanRewindHash, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onValidateOwnershipProof,
                     this, &ViewOutputs::onValidateOwnershipProof, Qt::QueuedConnection);

    inScanProcess = false;
}

ViewOutputs::~ViewOutputs() {
}

NextStateRespond ViewOutputs::execute() {
    if ( context->appContext->getActiveWndState() != STATE::VIEW_ACCOUNTS ) {
        inScanProcess = false;
        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    core::getWndManager()->pageViewHash();
    helpDocName = "viewing_key_start.html";
    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

bool ViewOutputs::canExitState(STATE nextWindowState) {
    Q_UNUSED(nextWindowState)
    return !inScanProcess;
}

void ViewOutputs::exitingState() {
    context->stateMachine->unblockLogout("ViewOutputs");
    helpDocName = "viewing_key_start.html";
}

// Return true if processing was made, so the sate still need to be active.
bool ViewOutputs::mobileBack() {
    return inScanProcess;
}

void ViewOutputs::onRecoverProgress(int progress, int maxVal ) {

    for (auto b: bridge::getBridgeManager()->getProgressWnd()) {
        if (b->getCallerId() == VIEW_ACCOUNTS_CALLER_ID) {
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
            b->initProgress(VIEW_ACCOUNTS_CALLER_ID, 0, maxVal);

            progress = std::min(maxVal, std::max(0, progress));
            QString msgProgress = "Blockchain scan is in progress...  " + QString::number(progress * 100.0 / maxVal, 'f',0) + "%";
            b->updateProgress(VIEW_ACCOUNTS_CALLER_ID, progress, msgProgress);
            b->setMsgPlus(VIEW_ACCOUNTS_CALLER_ID, "");
        }
    }
}

void ViewOutputs::onScanRewindHash(QVector< wallet::WalletOutput > outputResult, int64_t total, QString errors ) {
    inScanProcess = false;
    context->stateMachine->unblockLogout("ViewOutputs");

    if ( context->appContext->getActiveWndState() == STATE::VIEW_ACCOUNTS ) {
        if (errors.isEmpty()) {
            core::getWndManager()->pageViewAccounts();
            helpDocName = "viewing_key_outputs.html";
            for (auto b: bridge::getBridgeManager()->getViewAccounts()) {
                b->setSgnViewOutputs(outputResult, total);
            }
        }
        else {
            core::getWndManager()->pageViewHash();
            core::getWndManager()->messageTextDlg("Scanning Error", "Failed to scan outputs with Viwing Key.\n\n" + errors);
        }
    }
}

void ViewOutputs::startScanning(QString hashKey) {
    context->stateMachine->blockLogout("ViewOutputs");
    helpDocName = "viewing_key_outputs.html";

    core::getWndManager()->pageProgressWnd(mwc::PAGE_V_VIEW_SCANNING, VIEW_ACCOUNTS_CALLER_ID,
                                           "Scanning blockchain with Viewing Key", "Reading outputs from the node...", "", false);
    inScanProcess = true;
    context->wallet->scanRewindHash(hashKey);
}

void ViewOutputs::backFromOutputsView() {
    core::getWndManager()->pageViewHash();
    helpDocName = "viewing_key_start.html";
}

void ViewOutputs::back() {
    core::getWndManager()->pageViewHash();
    helpDocName = "viewing_key_start.html";
}

void ViewOutputs::generateOwnershipProofStart() {
    helpDocName = "viewing_key_gen_in.html";
    core::getWndManager()->pageGenerateOwnershipInput();
}

void ViewOutputs::generate_proof(QString message, bool viewingKey, bool torAddress, bool mqsAddress) {
    helpDocName = "viewing_key_gen_res.html";
    core::getWndManager()->pageGenerateOwnershipResult();
    context->wallet->generateOwnershipProof(message, viewingKey, torAddress, mqsAddress);
}

void ViewOutputs::validateOwnershipProofStart() {
    helpDocName = "viewing_key_val.html";
    core::getWndManager()->pageValidateOwnershipInput();
}

void ViewOutputs::validate_proof(QString proof) {
    helpDocName = "viewing_key_val.html";
    core::getWndManager()->pageValidateOwnershipResult();
    context->wallet->validateOwnershipProof(proof);
}

void ViewOutputs::onValidateOwnershipProof(QString network, QString message, QString viewingKey, QString torAddress, QString mqsAddress, QString error) {
    Q_UNUSED(network);
    Q_UNUSED(message);
    Q_UNUSED(torAddress);
    Q_UNUSED(mqsAddress);
    Q_UNUSED(error);
    if (!viewingKey.isEmpty() && viewingKey!="Not provided") {
        lastViewedViewingKey = viewingKey;
    }
}

} // state
