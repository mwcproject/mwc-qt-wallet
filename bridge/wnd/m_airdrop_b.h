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

#ifndef MWC_QT_WALLET_M_AIRDROP_B_H
#define MWC_QT_WALLET_M_AIRDROP_B_H

#include <QObject>
#include "../../state/m_airdrop.h"

namespace bridge {

class Airdrop : public QObject {
Q_OBJECT
public:
    explicit Airdrop(QObject * parent = nullptr);
    ~Airdrop();

    // true if status is active
    void updateAirDropStatus( bool waiting, bool status, QString message );

    void updateClaimStatus( int idx, QString requestBtcAddress,
                            QString status, QString message, int64_t amount, int errCode);

    void reportMessage( QString title, QString message );

    // request refresh for airdrop data
    Q_INVOKABLE void refreshAirdropStatusInfo();

    // initiate claiming process for btc address
    Q_INVOKABLE void startClaimingProcess( QString btcAddress, QString password );

    // start claim for btc address
    Q_INVOKABLE void requestClaimMWC( QString btcAddress, QString challendge, QString signature, QString identifier );

    // return back to ardrop page
    Q_INVOKABLE void backToMainAirDropPage();
signals:
    void sgnUpdateAirDropStatus( bool waiting, bool status, QString message );

    void sgnUpdateClaimStatus( int idx, QString requestBtcAddress,
                            QString status, QString message, QString mwc, int errCode);

    void sgnReportMessage( QString title, QString message );
};

}

#endif //MWC_QT_WALLET_M_AIRDROP_B_H
