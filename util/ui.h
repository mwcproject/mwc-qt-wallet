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

#ifndef MWC_QT_WALLET_UI_H
#define MWC_QT_WALLET_UI_H

#include <QString>
#include "../wallet/wallet.h"

namespace core {
class HodlStatus;
class AppContext;
}

class QWidget;

namespace util {
    // UI related small routines

    // in: nanoCoins < 0 - ALL
    // out: resultOutputs - what we want include into transaction. If
    // return false if User cancel this action.
    bool getOutputsToSend( const QString & accountName, int outputsNumber, int64_t nanoCoins,
                           wallet::Wallet * wallet,
                           core::HodlStatus * hodlStatus,
                           core::AppContext * appContext,
                           QWidget * parent, QStringList & resultOutputs );


    // Utility method. Exposed to testing only!!!
    bool calcOutputsToSpend( int64_t nanoCoins, const QVector<wallet::WalletOutput> & inputOutputs, QStringList & resultOutputs );

};


#endif //MWC_QT_WALLET_UI_H
