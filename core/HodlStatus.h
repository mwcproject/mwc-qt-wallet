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

#ifndef MWC_QT_WALLET_STATUS_H
#define MWC_QT_WALLET_STATUS_H

#include <QObject>
#include <QMap>
#include "../wallet/wallet.h"
#include <QSet>

namespace state {
struct StateContext;
}

namespace core {

// currently submitted HODL outputs as a server see them
struct HodlOutputInfo {
    QString    outputCommitment;
    double     value  = 0.0;
    double     weight = 0.0;
    QString    cls; // class

    void setData( const QString & outputCommitment, double value, double weight, const QString & cls );

    static HodlOutputInfo create(const QString & outputCommitment, double value, double weight, const QString & cls) {
        HodlOutputInfo item;
        item.setData(outputCommitment, value, weight, cls);
        return item;
    }

    bool isValid() const {
        return !outputCommitment.isEmpty() && value>0.0;
    }

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);
};

struct HodlClaimStatus {
    // Status values
    //    0 - initial state
    //    1 - challenge requested
    //    2 - claim complete
    //    3 - response accepted
    //    4 - will be for finalized

    int64_t amount = 0;
    int claimId = -1;
    int status = -1;

    void setData( int64_t amount, int claimId, int status );

    QString getStatusAsString() const;
};

// the status of all hodl process.
// The status is updated from several sources. That is why because of
// race conditions ti can be changed in any order and combination
class HodlStatus : public QObject {
    Q_OBJECT
public:
    HodlStatus( state::StateContext * context );

    void setHodlStatus( const QString & hodlStatus, const QString & errKey );
    void setHodlOutputs( const QString & hash, bool inHodl, const QVector<HodlOutputInfo> & hodlOutputs, const QString & errKey ); //
    void finishWalletOutputs();

    void setError( const QString & errKey, const QString & error );

    QString getRootPubKey() const {return rootPubKey;}
    // return true if new key was set
    void setRootPubKey( const QString & pubKey );

    // Hex representation of HSA256 hash from rootpublickey binary representation
    QString getRootPubKeyHash() const {return rootPubKeyHash;}

    bool isInHodl( const QString & hash) const {return inHodl.value(getHash(hash), false) || hodlOutputs.size()>0;}
    bool hasHodlOutputs() const;
    bool hasAmountToClaim() const;

    QString getHodlStatus() const {return hodlStatus;}
    // Calculates what we have for account
    QString getWalletHodlStatus(const QString & hash) const;

    QVector<HodlOutputInfo> getHodlOutputs(const QString & hash) const;

    void setHodlClaimStatus(const QString & hash, const QVector<HodlClaimStatus> & claims, const QString & errKey);
    QVector<HodlClaimStatus> getClaimsRequestStatus(const QString & hash) const { return claimStatus.value( getHash(hash)); }
    void lockClaimsRequestStatus(const QString & hash, int claimId);

    bool hasAnyOutputsInHODL() const { return !hodlOutputs.isEmpty();}

    bool isOutputInHODL(const QString & output) const {return hodlOutputs.contains(output);}

    // return empty if not exist
    HodlOutputInfo getHodlOutput(const QString & hash, const QString & output) const {return hodlOutputs.value(getHash(hash)).value(output);}

    // registration was sucessfull, let's update it
    void updateRegistrationTime();

    // Return true if we can trust the outputs that we get from HODL server. Likely scan was happens and data is updated
    bool isHodlRegistrationTimeLongEnough() const;

    QMap<QString, QString> getRequestErrors() const {return requestErrors;}

    // For empty return this wallet root pubKey hash. Otherwise this param.
    const QString & getHash(const QString & hash) const { return hash.isEmpty() ? rootPubKeyHash : hash; }
private slots:
    void onLoginResult(bool ok);
    void onLogout();

private:
signals:
    void onHodlStatusWasChanged();

private:
    // Logout repond
    void resetData();

private:
    state::StateContext * context;

    QString rootPubKey;
    QString rootPubKeyHash; // HSA256 hash

    QString hodlStatus; // Replay from /v1/getNextStartDate

    uint availableData = 0;

    // Key: Hash
    QMap<QString, bool> inHodl; // If accountin HODL. May in in Hodl but no outputs are there

    // Key: rootPubKeyHash. We can have several wallets here. Need to cover cold wallet case
    QMap<QString, QMap<QString, HodlOutputInfo>> hodlOutputs;
    QMap<QString, QVector<HodlClaimStatus>> claimStatus;

    //
    QMap<QString, QString> requestErrors;
};

}

#endif //MWC_QT_WALLET_STATUS_H
