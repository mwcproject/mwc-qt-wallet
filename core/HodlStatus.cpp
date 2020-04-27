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
#include <QDataStream>

namespace core {

const uint DATA_HODL_OUTPUTS    = 0x0002;
const uint DATA_AMOUNT_TO_CLAIM = 0x0004;

// currently submitted HODL outputs as a server see them
void HodlOutputInfo::setData( const QString & _outputCommitment, double _value, double _weight, const QString & _cls ) {
    outputCommitment = _outputCommitment;
    value = _value;
    weight = _weight;
    cls = _cls;
}

void HodlOutputInfo::saveData(QDataStream & out) const {
    out << int(0x345876);
    out << outputCommitment;
    out << value;
    out << weight;
    out << cls;
}

bool HodlOutputInfo::loadData(QDataStream & in) {
    int id = 0;
    in >> id;

    if ( id!=0x345876)
        return false;

    in >> outputCommitment;
    in >> value;
    in >> weight;
    in >> cls;
    return true;
}

void HodlClaimStatus::setData( int64_t _amount, int _claimId, int _status ) {
    amount = _amount;
    claimId = _claimId;
    status = _status;
}

QString HodlClaimStatus::getStatusAsString() const {
    // Status values
    //    0 - initial state
    //    1 - challenge requested
    //    2 - claim complete
    //    3 - response accepted
    //    4 - will be for finalized

    switch (status) {
        case 0:
            return "Ready to claim";
        case 1:
            return "Challenge requested";
        case 2:
            return "Slate requested";
        case 3:
            return "Response accepted";
        case 4:
            return "Finalized";
        case 5: // Artificial status, from QT wallet
            return "Claiming in progress";
        default:
            return "Unknown State";
    }
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

QVector<HodlOutputInfo> HodlStatus::getHodlOutputs(const QString & hash) const {
    QVector<HodlOutputInfo> res;
    QMap<QString, HodlOutputInfo> outputs = hodlOutputs.value(getHash(hash));
    for ( auto out = outputs.begin(); out!=outputs.end(); out++)
        res.push_back(out.value());

    return res;
}

void HodlStatus::setHodlStatus( const QString & _hodlStatus, const QString & errKey ) {
    hodlStatus = _hodlStatus;
    requestErrors.remove(errKey);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlStatus");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setHodlOutputs( const QString & hash, bool _inHodl, const QVector<HodlOutputInfo> & _hodlOutputs, const QString & errKey ) {
    availableData |= DATA_HODL_OUTPUTS;
    inHodl.insert(getHash(hash), _inHodl);

    QMap<QString, HodlOutputInfo> outputs;
    for (const auto & out : _hodlOutputs) {
        outputs.insert(out.outputCommitment, out);
    }
    requestErrors.remove(errKey);

    hodlOutputs.insert( getHash(hash), outputs );

    // Updating cache with HODL outputs
    if ( hash.isEmpty() && !rootPubKeyHash.isEmpty())
        context->appContext->saveHodlOutputs(rootPubKeyHash, outputs);

    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlOutputs");
    emit onHodlStatusWasChanged();
}

void HodlStatus::finishWalletOutputs() {
    logger::logEmit("HODL", "onHodlStatusWasChanged", "setWalletOutputs");
    emit onHodlStatusWasChanged();
}

void HodlStatus::setHodlClaimStatus(const QString & hash, const QVector<HodlClaimStatus> & claims, const QString & errKey) {
    claimStatus.insert( getHash(hash), claims);
    requestErrors.remove(errKey);
    availableData |= DATA_AMOUNT_TO_CLAIM;
    logger::logEmit("HODL", "onHodlStatusWasChanged", "setHodlClaimStatus");
    emit onHodlStatusWasChanged();
}

void HodlStatus::lockClaimsRequestStatus(const QString & hash, int claimId) {
    QVector<HodlClaimStatus> & claims = claimStatus[getHash(hash)];
    for (auto & cl : claims) {
        if (cl.claimId == claimId) {
            cl.status = 5;
            logger::logEmit("HODL", "onHodlStatusWasChanged", "lockClaimsRequestStatus");
            emit onHodlStatusWasChanged();
            return;
        }
    }
}


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
    inHodl.insert( rootPubKeyHash, false );
    availableData &= ~(DATA_HODL_OUTPUTS | DATA_AMOUNT_TO_CLAIM);

    // HODL outputs updating from the cache. Reason that wallet does manage outputs and we
    if (!rootPubKeyHash.isEmpty())
        hodlOutputs.insert( rootPubKeyHash, context->appContext->loadHodlOutputs(rootPubKeyHash));
    else
        hodlOutputs.clear();

    emit onHodlStatusWasChanged();
}


bool HodlStatus::hasHodlOutputs() const {
    return (availableData & DATA_HODL_OUTPUTS)!=0;
}

bool HodlStatus::hasAmountToClaim() const {
    return (availableData & DATA_AMOUNT_TO_CLAIM)!=0;
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

    inHodl.clear();
    hodlOutputs.clear();
    claimStatus.clear();

    requestErrors.clear();
}

// Calculates what we have for account
QString HodlStatus::getWalletHodlStatus(const QString & hash) const {

    bool canSkipWalletData = config::isOnlineNode() || !hash.isEmpty();

    if ( !rootPubKeyHash.isEmpty() && (availableData & DATA_HODL_OUTPUTS)!=0 && canSkipWalletData) {
        if (!inHodl.value(getHash(hash), false)) {
            return "Wallet not registered for HODL";
        }

        if ( hodlOutputs.value(getHash(hash)).isEmpty() && !isHodlRegistrationTimeLongEnough() ) {
            return "Waiting for HODL server to scan outputs, can take up to 24 hours";
        }

        // in nano coins
        QMap<QString, int64_t> hodlBalancePerClass;

        if (canSkipWalletData) {
            for ( auto & ho : hodlOutputs.value(getHash(hash)) ) {
                int64_t balance = hodlBalancePerClass.value( ho.cls, 0 );
                balance += int64_t(ho.value * 1000000000.0 + 0.5);
                hodlBalancePerClass.insert( ho.cls, balance );
            }
        }
        else {
            const QMap<QString, HodlOutputInfo> & hodl_outputs = hodlOutputs[getHash(hash)];
            const QMap<QString, QVector<wallet::WalletOutput> > & walletOutputs = context->wallet->getwalletOutputs();
            for ( auto o = walletOutputs.constBegin(); o != walletOutputs.constEnd(); ++o ) {
                for ( const auto & walletOutput : o.value() ) {
                    // Counting only exist outputs. Unconfirmed doesn't make sense to count
                    if ( (walletOutput.status=="Unspent" || walletOutput.status=="Locked") && hodl_outputs.contains(walletOutput.outputCommitment) ) {
                        auto ho = hodl_outputs[walletOutput.outputCommitment];
                        int64_t balance = hodlBalancePerClass.value( ho.cls, 0 );
                        balance += int64_t(ho.value * 1000000000.0 + 0.5);
                        hodlBalancePerClass.insert( ho.cls, balance );
                    }
                }
            }
        }

        QString resultStr = "Your HODL amount:\n";
        for (auto balance = hodlBalancePerClass.begin(); balance != hodlBalancePerClass.end(); balance++ ) {
            resultStr += balance.key() + " : " + util::nano2one(balance.value()) + " MWC\n";
        }

        // Check if has something to claim
        if (!claimStatus.isEmpty()) {
            int64_t available = 0;
            int64_t inprogress = 0;

            for (const HodlClaimStatus & status : claimStatus.value(getHash(hash)) ) {
                if ( status.status==0 ) {
                    available += status.amount;
                }
                else if ( status.status<4 ) {
                    inprogress += status.amount;
                }
            }

            if ( available > 0 || inprogress > 0 ) {
                resultStr += "\n";

                if (available>0)
                    resultStr += "Available for Claim : " + util::nano2one(available) + " MWC\n";

                if (inprogress>0)
                    resultStr += "Claim in progress : " + util::nano2one(inprogress) + " MWC\n";
            }
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
