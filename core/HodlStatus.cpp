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
#include "../core/appcontext.h"
#include "../core/Config.h"

namespace core {

const uint DATA_WALLET_OUTPUTS  = 0x0001;
const uint DATA_HODL_OUTPUTS    = 0x0002;
const uint DATA_AMOUNT_TO_CLAIM = 0x0004;

// currently submitted HODL outputs as a server see them
void HodlOutputInfo::setData( const QString & _outputCommitment, double _value, double _weight, const QString & _cls ) {
    outputCommitment = _outputCommitment;
    value = _value;
    weight = _weight;
    cls = _cls;
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
    if (context) {
        connect(context->wallet, &wallet::Wallet::onLoginResult, this, &HodlStatus::onLoginResult, Qt::QueuedConnection);
        connect(context->wallet, &wallet::Wallet::onLogout, this, &HodlStatus::onLogout, Qt::QueuedConnection);
    }

    resetData();
}

QVector<HodlOutputInfo> HodlStatus::getHodlOutputs() const {
    QVector<HodlOutputInfo> res;
    for ( auto out = hodlOutputs.begin(); out!=hodlOutputs.end(); out++)
        res.push_back(out.value());

    return res;
}

void HodlStatus::setHodlStatus( const QString & _hodlStatus, const QString & errKey ) {
    hodlStatus = _hodlStatus;
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlStatus");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setHodlOutputs( bool _inHodl, const QVector<HodlOutputInfo> & _hodlOutputs, const QString & errKey ) {
    availableData |= DATA_HODL_OUTPUTS;
    inHodl = _inHodl;

    for (const auto & out : _hodlOutputs) {
        hodlOutputs.insert(out.outputCommitment, out);
    }
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlOutputs");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setWalletOutputs( const QString & account, const QVector<wallet::WalletOutput> & outputs, const QString & errKey ) {
    walletOutputs[account] = outputs;
    requestErrors.remove(errKey);
}

void HodlStatus::finishWalletOutputs(bool done) {
    if (!done) {
        walletOutputs.clear();
        availableData &= ~DATA_WALLET_OUTPUTS;
    }
    else {
        availableData |= DATA_WALLET_OUTPUTS;
        logger::logEmit("HODL", "onHodlStatusWasChanged", "setWalletOutputs");
    }
    emit onHodlStatusWasChanged();
}

/*
void HodlStatus::setClaimAmount( int64_t _amount2claim, const QString & errKey ) {
    availableData |= DATA_AMOUNT_TO_CLAIM;
    amount2claim = _amount2claim;
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setClaimAmount");
    emit onHodlStatusWasChanged();
}*/

void HodlStatus::setError( const QString & errKey, const QString & error ) {
    requestErrors[errKey] = error;

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setError");

    emit onHodlStatusWasChanged();
}


void HodlStatus::setRootPubKey( const QString & pubKey )
{
    rootPubKey = pubKey;
    QByteArray keyHex = pubKey.toUtf8();// crypto::str2hex(pubKey);
    if (!keyHex.isEmpty()) {
        rootPubKeyHash = crypto::hex2str( crypto::HSA256( keyHex ) );
    }
    else {
        rootPubKeyHash = "";
    }

    // reseting account related data
    inHodl = false;
    availableData &= ~(DATA_HODL_OUTPUTS | DATA_AMOUNT_TO_CLAIM);
    hodlOutputs.clear();
    amount2claim = 0;

    emit onHodlStatusWasChanged();
}


/*bool HodlStatus::hasHodlStatus() const {
    return !hodlStatus.isEmpty();
}*/

bool HodlStatus::hasHodlOutputs() const {
    return (availableData & DATA_HODL_OUTPUTS)!=0;
}

/*bool HodlStatus::hasAmountToClaim() const {
    return (availableData & DATA_AMOUNT_TO_CLAIM)!=0;
}*/

/*bool HodlStatus::hasErrors() const {
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
}*/

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

    availableData = 0;

    inHodl = false;
    walletOutputs.clear(); // Available outputs from the wallet.
    hodlOutputs.clear();

    amount2claim = 0;

    requestErrors.clear();
}

// Calculates what we have for account
QString HodlStatus::getWalletHodlStatus() const {

    bool canSkipWalletData = config::isOnlineNode();

    if ( !rootPubKeyHash.isEmpty() && (availableData & DATA_HODL_OUTPUTS)!=0 && (canSkipWalletData || (availableData & DATA_WALLET_OUTPUTS)!=0 )) {
        if (!inHodl) {
            return "Wallet not registered for HODL";
        }

        if ( hodlOutputs.isEmpty() && !isHodlRegistrationTimeLongEnough() ) {
            return "Waiting for HODL server to scan outputs, can take up to 24 hours";
        }

        // in nano coins
        QMap<QString, int64_t> hodlBalancePerClass;

        if (canSkipWalletData) {
            for ( auto & ho : hodlOutputs ) {
                int64_t balance = hodlBalancePerClass.value( ho.cls, 0 );
                balance += int64_t(ho.value * 1000000000.0 + 0.5);
                hodlBalancePerClass.insert( ho.cls, balance );
            }
        }
        else {
            for ( auto o = walletOutputs.constBegin(); o != walletOutputs.constEnd(); ++o ) {
                for ( const auto & walletOutput : o.value() ) {
                    // Counting only exist outputs. Unconfirmed doesn't make sense to count
                    if ( (walletOutput.status=="Unspent" || walletOutput.status=="Locked") && hodlOutputs.contains(walletOutput.outputCommitment) ) {
                        auto ho = hodlOutputs[walletOutput.outputCommitment];
                        int64_t balance = hodlBalancePerClass.value( ho.cls, 0 );
                        balance += int64_t(ho.value * 1000000000.0 + 0.5);
                        hodlBalancePerClass.insert( ho.cls, balance );
                    }
                }
            }
        }

        QString resultStr = "Your HODL amount:\n";
        for (auto balance = hodlBalancePerClass.begin(); balance != hodlBalancePerClass.end(); balance++ ) {
            resultStr += balance.key() + " : " + util::nano2one(balance.value()) + " MWC";
        }

        return resultStr;
    }
    else {
        if (config::isOnlineNode()) {
            if (rootPubKeyHash.isEmpty()) {
                return "";
            }
        }

        if (requestErrors.isEmpty()) {
            return "Waiting for Account Data";
        }
        else {
            return "HODL request error: " + requestErrors.values().join(", ");
        }

    }
}

// registration was sucessfull, let's update it
void HodlStatus::updateRegistrationTime() {
    Q_ASSERT(context);
    Q_ASSERT(context->appContext);
    Q_ASSERT(!rootPubKeyHash.isEmpty());

    if (rootPubKeyHash.isEmpty())
        return;

    if (context->appContext->getHodlRegistrationTime(rootPubKeyHash)==0){
        context->appContext->setHodlRegistrationTime(rootPubKeyHash, QDateTime::currentMSecsSinceEpoch() );
    }

}

// Return true if we can trust the outputs that we get from HODL server. Likely scan was happens and data is updated
bool HodlStatus::isHodlRegistrationTimeLongEnough() const {
    if (rootPubKeyHash.isEmpty())
        return false;
    int64_t t = context->appContext->getHodlRegistrationTime(rootPubKeyHash);
    if (t==0)
        return false;

    return (QDateTime::currentMSecsSinceEpoch() - t) > 1000L*3600L*26; // Let's wait 2 extra hours for scan. Scan job might take time
}

}
