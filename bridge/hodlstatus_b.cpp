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

#include "hodlstatus_b.h"
#include "../core/HodlStatus.h"
#include "../state/state.h"

namespace bridge {

static core::HodlStatus * getHodl() {
    auto res = state::getStateContext()->hodlStatus;
    Q_ASSERT(res);
    return res;
}

HodlStatus::HodlStatus(QObject * parent) : QObject(parent) {}

HodlStatus::~HodlStatus() {}

// Return true if wallet with Hash is in HODL.
// Empty hash value - mean current wallet
bool HodlStatus::isInHodl(QString walletHash) {
    return getHodl()->isInHodl(walletHash);
}

bool HodlStatus::isOutputInHODL(QString outputCommitment) {
    return getHodl()->isOutputInHODL(outputCommitment);
}

// Returns output Class for hodl. Empty String if not in HODL.
QString HodlStatus::getOutputHodlStatus(QString outputCommitment) {
    return getHodl()->getHodlOutput( "", outputCommitment ).cls;
}

// Get HODL status for the wallet. Wallet Hash can be empty for this wallet.
// Return pair: [ message, Mwc to claim ]
QVector<QString> HodlStatus::getWalletHodlStatus(QString hash) {
    auto res = getHodl()->getWalletHodlStatus(hash);

    return {res.first, util::nano2one(res.second)};
}

// general satus strign for the HODL as HODL server return it
QString HodlStatus::getHodlStatus() {
    return getHodl()->getHodlStatus();
}

// Root public key for this wallet
QString HodlStatus::getRootPubKey() {
    return getHodl()->getRootPubKey();
}

// Hash of the Root public key for this wallet
QString HodlStatus::getRootPubKeyHash() {
    return getHodl()->getRootPubKeyHash();
}

// Return true if it has hodl outputs
bool HodlStatus::hasHodlOutputs() {
    return getHodl()->hasHodlOutputs();
}


// Request all outputs that are known as Json Strings.
QVector<QString> HodlStatus::getHodlOutputs(QString hash) {
    QVector<QString> res;
    QVector<core::HodlOutputInfo> outs = getHodl()->getHodlOutputs(hash);
    for (auto & o : outs)
        res.push_back(o.toJson());

    return res;
}

// Pairs of HODL related request errors
QVector<QString> HodlStatus::getRequestErrors() {
    auto errMap = getHodl()->getRequestErrors();
    QVector<QString> res;

    for ( auto re = errMap.begin(); re != errMap.end(); re++ ) {
        res.push_back(re.key());
        res.push_back(re.value());
    }
    return res;
}

// Lines for Requested status
// Return groups of 4 [ <true/false is has claim>, <mwc>, <claimId>, <status string> ]
QVector<QString> HodlStatus::getClaimsRequestStatus(QString hash) {
    QVector<core::HodlClaimStatus> claimStatus = getHodl()->getClaimsRequestStatus(hash);
    QVector<QString> result;
    for (const auto & st : claimStatus) {
        result.push_back( st.status<3 ? "true" : "false");
        result.push_back(util::nano2one(st.amount));
        result.push_back(QString::number(st.claimId));
        result.push_back(st.getStatusAsString());
    }
    return result;
}


};
