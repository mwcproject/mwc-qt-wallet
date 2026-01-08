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
// Created by Konstantin Bay on 9/23/24.
//

#include "v_viewoutputs_b.h"

#include <QJsonArray>
#include <QJsonDocument>

#include "../BridgeManager.h"
#include "../../state/v_ViewOutputs_s.h"
#include "../../util/stringutils.h"
#include "../../util/Log.h"

namespace bridge {

static state::ViewOutputs * getState() {return (state::ViewOutputs *) state::getState(state::STATE::VIEW_ACCOUNTS);}

QString ViewOutputs::lastViewingKey;


ViewOutputs::ViewOutputs(QObject *parent) :
        QObject(parent) {
    getBridgeManager()->addViewAccounts(this);
}

ViewOutputs::~ViewOutputs() {
    getBridgeManager()->removeViewAccounts(this);
}

// Switching to the progress window
void ViewOutputs::startScanning(QString hashKey) {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::startScanning with hashKey=" + hashKey);
    lastViewingKey = hashKey;
    getState()->startScanning(hashKey);
}

void ViewOutputs::backFromOutputsView() {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::backFromOutputsView");
    getState()->backFromOutputsView();
}

void ViewOutputs::setSgnViewOutputs(const wallet::ViewWallet & walletOutputs, int64_t height) {
    QJsonArray outputs;

    for (const wallet::ViewWalletOutputResult & o : walletOutputs.output_result) {

        wallet::WalletOutput out = wallet::WalletOutput::create( o.commit,
                                           QString::number(o.mmr_index),
                                           QString::number(o.height),
                                           o.lock_height>0 ? QString::number(o.lock_height) : "",
                                           "Unspent",
                                           o.is_coinbase,
                                           QString::number(height - o.height),
                                           o.value,
                                           -1);

        outputs.push_back(out.toJson());
    }

    emit onSgnViewOutputs( lastViewingKey, outputs, util::zeroDbl2Dbl(util::nano2one( walletOutputs.total_balance)));
}

void ViewOutputs::generateOwnershipProofStart() {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::generateOwnershipProofStart");
    getState()->generateOwnershipProofStart();
}

void ViewOutputs::validateOwnershipProofStart() {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::validateOwnershipProofStart");
    getState()->validateOwnershipProofStart();
}


void ViewOutputs::back() {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::back");
    getState()->back();
}

void ViewOutputs::generate_proof(QString message, bool viewingKey, bool torAddress, bool mqsAddress) {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::generate_proof with message=" + message +
        " viewingKey=" + QString::number(viewingKey) + " torAddress=" + QString::number(torAddress) +
        " mqsAddress=" + QString::number(mqsAddress));
    getState()->generate_proof(message, viewingKey, torAddress, mqsAddress);
}

void ViewOutputs::validate_proof(QString proof) {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::validate_proof with proof=<hidden>");
    getState()->validate_proof(proof);
}

QString ViewOutputs::getLastViewViewingKey() {
    logger::logInfo(logger::BRIDGE, "Call ViewOutputs::getLastViewViewingKey");
    return lastViewingKey;
}

}
