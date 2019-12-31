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

#include "HodlStatus.h"
#include "../state/state.h"
#include "../util/crypto.h"
#include "../util/Log.h"

namespace core {

const uint DATA_WALLET_OUTPUTS  = 0x0001;
const uint DATA_HODL_OUTPUTS    = 0x0002;
const uint DATA_AMOUNT_TO_CLAIM = 0x0004;

// currently submitted HODL outputs as a server see them
void HodlOutputInfo::setData( const QString & _outputCommitment, int64_t _valueNano, int _height ) {
    outputCommitment = _outputCommitment;
    valueNano = _valueNano;
    height = _height;
}

void HodlClaimStatus::setData( int64_t _HodlAmount, int64_t _claimedMwc, const QString & _status, const QString & _date ) {
    HodlAmount = _HodlAmount;
    claimedMwc = _claimedMwc;
    status = _status;
    date = _date;
}

///////////////////////////////////////////////////////////////////////////
// HodlStatus

HodlStatus::HodlStatus( state::StateContext * _context ) : context(_context) {
    connect( context->wallet , &wallet::Wallet::onLoginResult,   this, &HodlStatus::onLoginResult, Qt::QueuedConnection );
    connect( context->wallet , &wallet::Wallet::onLogout,        this, &HodlStatus::onLogout,      Qt::QueuedConnection );

    resetData();
}

void HodlStatus::setHodlStatus( const QString & _hodlStatus, const QString & errKey ) {
    hodlStatus = _hodlStatus;
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlStatus");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setHodlOutputs( const QVector<HodlOutputInfo> & _hodlOutputs, const QString & errKey ) {
    availableData |= DATA_HODL_OUTPUTS;
    hodlOutputs = _hodlOutputs;
    requestErrors.remove(errKey);

    hodlOutputCommitment.clear();
    for (const auto & ho : hodlOutputs)
        hodlOutputCommitment += ho.outputCommitment;

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlOutputs");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setWalletOutputs( const QString & account, const QVector<wallet::WalletOutput> & outputs, const QString & errKey ) {
    availableData |= DATA_WALLET_OUTPUTS;
    walletOutputs[account] = outputs;
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setWalletOutputs");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setClaimAmount( int64_t _amount2claim, const QString & errKey ) {
    availableData |= DATA_AMOUNT_TO_CLAIM;
    amount2claim = _amount2claim;
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setClaimAmount");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setError( const QString & errKey, const QString & error ) {
    requestErrors[errKey] = error;

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setError");
    emit onHodlStatusWasChanged();
}


void HodlStatus::setRootPubKey( const QString & pubKey )
{
    rootPubKey = pubKey;
    QByteArray keyHex = crypto::str2hex(pubKey);
    if (!keyHex.isEmpty()) {
        rootPubKeyHash = crypto::hex2str( crypto::HSA256( keyHex ) );
    }
    else {
        rootPubKeyHash = "";
    }
}


bool HodlStatus::hasHodlStatus() const {
    return !hodlStatus.isEmpty();
}

bool HodlStatus::hasHodlOutputs() const {
    return (availableData & DATA_HODL_OUTPUTS)!=0;
}

bool HodlStatus::hasAmountToClaim() const {
    return (availableData & DATA_AMOUNT_TO_CLAIM)!=0;
}

bool HodlStatus::hasErrors() const {
    return !requestErrors.isEmpty();
}
QString HodlStatus::getErrorsAsString() const {
    QString errsStr;
    for ( auto & errValues : requestErrors ) {
        if (!errsStr.isEmpty())
            errsStr += "\n";
        errsStr += errValues;
    }
    return errsStr;
}

QVector<HodlClaimStatus> HodlStatus::getClaimsRequestStatus() const {
    return QVector<HodlClaimStatus>();
}

void HodlStatus::onLoginResult(bool ok) {
    Q_UNUSED(ok)
    resetData();
}

void HodlStatus::onLogout() {
    resetData();
}

// Logout repond
void HodlStatus::resetData() {
    rootPubKey = "";
    rootPubKeyHash = "";

    hodlStatus = "Waiting for HODL data..."; // Replay from /v1/getNextStartDate
    accountStatus = "Waiting for Account Data"; // Replay from checkAddresses

    availableData = 0;

    walletOutputs.clear(); // Available outputs from the wallet.
    hodlOutputs.clear();

    amount2claim = 0;

    requestErrors.clear();
}


}
