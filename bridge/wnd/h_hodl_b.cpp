// Copyright 2020 The MWC Developers
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

#include "h_hodl_b.h"
#include "../BridgeManager.h"
#include "../../state/h_hodl.h"
#include "../../state/state.h"
#include "../../util/crypto.h"

namespace bridge {

static state::Hodl * getState() {return (state::Hodl *) getState(state::STATE::HODL); }

Hodl::Hodl(QObject * parent) : QObject(parent) {
    getBridgeManager()->addHodl(this);
}

Hodl::~Hodl() {
    getBridgeManager()->removeHodl(this);
}

void Hodl::setRootPubKeyWithSignature(const QString & key, const QString & message, const QString & signature) {
    QByteArray keyHex = key.toUtf8();
    QString hash = crypto::hex2str( crypto::HSA256( keyHex ));
    emit sgnSetRootPubKeyWithSignature(key, hash, message, signature);
}

// Hodl object changed it's state, need to refresh
void Hodl::updateHodlState() {
    emit sgnUpdateHodlState();
}
void Hodl::reportMessage(const QString & title, const QString & message) {
    emit sgnReportMessage(title, message);
}

void Hodl::hideWaitingStatus() {
    emit sgnHideWaitingStatus();
}

void Hodl::setColdWalletPublicKey(QString pubkey) {
    getState()->setColdWalletPublicKey(pubkey);
}

void Hodl::registerAccountForHODL() {
    getState()->registerAccountForHODL();
}

void Hodl::moveToClaimPage(QString hash)  {
    getState()->moveToClaimPage(hash);
}

void Hodl::requestHodlInfoRefresh(QString hash) {
    getState()->requestHodlInfoRefresh(hash);
}

void Hodl::claimMWC(QString hash) {
    getState()->claimMWC(hash);
}

void Hodl::requestSignMessage(QString msg) {
    getState()->requestSignMessage(msg);
}


}
