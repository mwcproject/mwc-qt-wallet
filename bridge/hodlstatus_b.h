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

#ifndef MWC_QT_WALLET_HODLSTATUS_B_H
#define MWC_QT_WALLET_HODLSTATUS_B_H

#include <QObject>
#include <QVector>

namespace bridge {

class HodlStatus : public QObject {
Q_OBJECT
public:
    explicit HodlStatus(QObject * parent);
    ~HodlStatus();

    // Return true if wallet with Hash is in HODL.
    // Empty hash value - mean current wallet
    Q_INVOKABLE bool isInHodl(QString walletHash = "");

    // Check if output belong to HODL
    Q_INVOKABLE bool isOutputInHODL(QString outputCommitment);

    // Returns output Class for hodl. Empty String if not in HODL.
    Q_INVOKABLE QString getOutputHodlStatus(QString outputCommitment);

    // Get HODL status for the wallet. Wallet Hash can be empty for this wallet.
    // Return pair: [ message, mwc to claim ]
    Q_INVOKABLE QVector<QString> getWalletHodlStatus(QString hash);

    // general satus strign for the HODL as HODL server return it
    Q_INVOKABLE QString getHodlStatus();

    // Root public key for this wallet
    Q_INVOKABLE QString getRootPubKey();

    // Hash of the Root public key for this wallet
    Q_INVOKABLE QString getRootPubKeyHash();

    // Return true if it has hodl outputs
    Q_INVOKABLE  bool hasHodlOutputs();

    // Request all outputs that are known as Json Strings.
    Q_INVOKABLE QVector<QString> getHodlOutputs(QString hash = "");

    // Pairs of HODL related request errors [key, vaues as error]
    Q_INVOKABLE QVector<QString> getRequestErrors();

    // Lines for Requested status
    // Return groups of 4 [ <true/false is has claim>, <mwc>, <claimId>, <status string> ]
    Q_INVOKABLE QVector<QString>  getClaimsRequestStatus(QString hash = "");
};

}

#endif //MWC_QT_WALLET_HODLSTATUS_B_H
