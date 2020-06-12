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
    // out: resultOutputs - what we want include into transaction.
    //      txnFee        - txn fee for send using these resultOutputs
    // return false if User cancel this action.
    bool getOutputsToSend( const QString & accountName, int outputsNumber, int64_t nanoCoins,
                           wallet::Wallet * wallet,
                           core::HodlStatus * hodlStatus,
                           core::AppContext * appContext,
                           QStringList & resultOutputs, uint64_t* txnFee );


    // Utility method. Exposed to testing only!!!
    bool calcOutputsToSpend( int64_t nanoCoins, const QVector<wallet::WalletOutput> & inputOutputs, QStringList & resultOutputs );

    // Calculates the transaction fee (in nanocoins) for the given amount (in nanocoins).
    // Returns the transaction fee and populates txnOutputList with the outputs used
    // in calculating the fee and therefore should be sent as the outputs for the
    // transaction.
    //
    // The fee is computed based upon all spendable outputs in the wallet.
    // This method should only be called if there are no HODL outputs or lockedOutputs
    // is not enabled. Call only if getOutputsToSend() doesn't return resultOutputs and the txnFee
    //
    // returns 0 if the fee could not be calculated and the txnOutputList should not be used.
    //
    uint64_t getTxnFee(const QString& accountName, int64_t amount, wallet::Wallet* wallet,
                       core::AppContext* appContext, uint64_t changeOutputs,
                       QStringList& txnOutputList);

    //
    // Even though you will find documentation which says the transaction fee is
    // calculated as 4*(num_outputs + num_kernels) - num_inputs that is not what is actually
    // in the grin code. The calculation they use is:
    //     (4*num_outputs) + num_kernels - num_inputs
    //
    // Using more inputs lowers the fee.
    //
    uint64_t calcTxnFee(uint64_t numInputs, uint64_t numOutputs, uint64_t numKernels);

    //
    // Converts the given txn fee in nano coins into a string of the form: x.xxx
    // If the given transaction fee is 0, "unknown" is returned.
    QString txnFeeToString(uint64_t nanoTxnFee);

    //
    // Returns a string representing the account's spendable balance minus the transaction fee.
    // If an error occurs, "all" is returned.
    //
    QString getAllSpendableAmount(const QString& accountName, wallet::Wallet* wallet, core::AppContext* appContext);

};


#endif //MWC_QT_WALLET_UI_H
