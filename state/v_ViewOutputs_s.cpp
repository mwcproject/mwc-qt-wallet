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
#include "a_initaccount.h"
#include "../core/WndManager.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/z_progresswnd_b.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include <QJsonArray>
#include <QJsonDocument>
#include "../util/Log.h"

#include "node/node_client.h"
#include "util/message_mapper.h"

namespace state {

ViewOutputs::ViewOutputs(StateContext * context) :
        State(context, STATE::VIEW_ACCOUNTS)
{
    QObject::connect(context->wallet, &wallet::Wallet::onScanProgress,
                     this, &ViewOutputs::onScanProgress, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onScanRewindHash,
                     this, &ViewOutputs::onScanRewindHash, Qt::QueuedConnection);
}

ViewOutputs::~ViewOutputs() {
}

NextStateRespond ViewOutputs::execute() {
    if ( context->appContext->getActiveWndState() != STATE::VIEW_ACCOUNTS ) {
        return NextStateRespond(NextStateRespond::RESULT::DONE);
    }

    if (scanRespondId.isEmpty()) {
        core::getWndManager()->pageViewHash();
        helpDocName = "viewing_key_start.html";
    }
    else {
        core::getWndManager()->pageProgressWnd(mwc::PAGE_V_VIEW_SCANNING, scanRespondId,
                                       "Scanning blockchain with Viewing Key", "Reading outputs from the node...", "", false);
        helpDocName = "viewing_key_outputs.html";
    }
    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

bool ViewOutputs::canExitState(STATE nextWindowState) {
    logger::logInfo(logger::STATE, "Call ViewOutputs::canExitState with nextWindowState=" + QString::number(nextWindowState));
    Q_UNUSED(nextWindowState)
    return scanRespondId.isEmpty();
}

void ViewOutputs::exitingState() {
    logger::logInfo(logger::STATE, "Call ViewOutputs::exitingState");
    context->stateMachine->unblockLogout("ViewOutputs");
    helpDocName = "viewing_key_start.html";
}

// Return true if processing was made, so the sate still need to be active.
bool ViewOutputs::mobileBack() {
    return !scanRespondId.isEmpty();
}

void ViewOutputs::onScanProgress( QString responseId, QJsonObject statusMessage ) {
    if (responseId != scanRespondId)
        return;

    if (statusMessage.contains("Scanning")) {
        QJsonArray vals = statusMessage["Scanning"].toArray();
        QString message = vals[1].toString();
        int percent_progress = vals[2].toInt();

        for (auto p :  bridge::getBridgeManager()->getProgressWnd()) {
            p->initProgress(scanRespondId, 0, 100);
            p->updateProgress(scanRespondId, percent_progress, util::mapMessage(message));
            p->setMsgPlus(scanRespondId, "");
        }
    }
}

void ViewOutputs::onScanRewindHash(QString responseId, wallet::ViewWallet walletOutputs, QString error  ) {
    if (responseId != scanRespondId)
        return;

    scanRespondId = "";

    context->stateMachine->unblockLogout("ViewOutputs");

    if ( context->appContext->getActiveWndState() == STATE::VIEW_ACCOUNTS ) {
        if (error.isEmpty()) {
            qint64 height = context->nodeClient->requestNodeStatus().nodeHeight;
            core::getWndManager()->pageViewAccounts();
            helpDocName = "viewing_key_outputs.html";
            for (auto b: bridge::getBridgeManager()->getViewAccounts()) {
                b->setSgnViewOutputs(walletOutputs, height);
            }
        }
        else {
            core::getWndManager()->pageViewHash();
            core::getWndManager()->messageTextDlg("Scanning Error", "Failed to scan outputs with Viwing Key.\n\n" + util::mapMessage(error) );
        }
    }
}

void ViewOutputs::startScanning(QString hashKey) {
    logger::logInfo(logger::STATE, "Call ViewOutputs::startScanning with hashKey=" + hashKey);
    context->stateMachine->blockLogout("ViewOutputs");
    helpDocName = "viewing_key_outputs.html";

    scanRespondId = context->wallet->scanRewindHash(hashKey);
    if (scanRespondId.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Another viewing key scan job is still running. Please wait until it finishes before starting another one.");
    }
    else {
        core::getWndManager()->pageProgressWnd(mwc::PAGE_V_VIEW_SCANNING, scanRespondId,
                                               "Scanning blockchain with Viewing Key", "Reading outputs from the node...", "", false);
    }
}

void ViewOutputs::backFromOutputsView() {
    logger::logInfo(logger::STATE, "Call ViewOutputs::backFromOutputsView");
    core::getWndManager()->pageViewHash();
    helpDocName = "viewing_key_start.html";
}

void ViewOutputs::back() {
    logger::logInfo(logger::STATE, "Call ViewOutputs::back");
    core::getWndManager()->pageViewHash();
    helpDocName = "viewing_key_start.html";
}

void ViewOutputs::generateOwnershipProofStart() {
    logger::logInfo(logger::STATE, "Call ViewOutputs::generateOwnershipProofStart");
    helpDocName = "viewing_key_gen_in.html";
    core::getWndManager()->pageGenerateOwnershipInput();
}

void ViewOutputs::generate_proof(QString message, bool viewingKey, bool torAddress, bool mqsAddress) {
    logger::logInfo(logger::STATE, "Call ViewOutputs::generate_proof with message=<hidden> viewingKey=" + QString::number(viewingKey) +
        " torAddress=" + QString::number(torAddress) + " mqsAddress=" + QString::number(mqsAddress));
    helpDocName = "viewing_key_gen_res.html";
    QJsonObject proof = context->wallet->generateOwnershipProof(message, viewingKey, torAddress, mqsAddress);
    if (proof.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Unable to generate ownership proof.");
        return;
    }

    QString proofStr = QJsonDocument(proof).toJson(QJsonDocument::Compact);
    core::getWndManager()->pageGenerateOwnershipResult(proofStr);
}

void ViewOutputs::validateOwnershipProofStart() {
    logger::logInfo(logger::STATE, "Call ViewOutputs::validateOwnershipProofStart");
    helpDocName = "viewing_key_val.html";
    core::getWndManager()->pageValidateOwnershipInput();
}

void ViewOutputs::validate_proof(QString proof) {
    logger::logInfo(logger::STATE, "Call ViewOutputs::validate_proof with proof=<hidden>");
    helpDocName = "viewing_key_val.html";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(proof.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        core::getWndManager()->messageTextDlg("Error", "Unable to parse proof, it is not in Json format.");
        return;
    }

    wallet::OwnershipProofValidation validation_result = context->wallet->validateOwnershipProof(doc.object());
    if (!validation_result.error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Proof validation error", "Unable to validate a proof.\n\n" + util::mapMessage(validation_result.error));
        return;
    }

    core::getWndManager()->pageValidateOwnershipResult(validation_result.network, validation_result.message, validation_result.viewing_key,
                        validation_result.tor_address, validation_result.mqs_address);
}

} // state
