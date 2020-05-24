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

    // calculates the transaction fee (in nanocoins) for the given amount (in nanocoins)
    // and populates the transaction fee parameter
    //
    // if txnOutputList is given, computes the fee based upon those outputs in the wallet
    // if txnOutputList is not given, computes the fee based upon all spendable outputs in the wallet
    //
    // returns true if the fee was able to be calculated and there are enough outputs
    // to cover the amount + fee
    // returns false there was an error calculating the fee or if there were not enough
    // outputs to cover the amount + fee
    // transactionFee will be populated with 0, if there was an error calculating
    // the fee. Otherwise transaction fee will contain the actual fee regardless
    // of the return status.
    bool getTxnFee(QWidget* parent, const QString& accountName, int64_t amount,
                   wallet::Wallet* wallet, core::AppContext* appContext,
                   QStringList& txnOutputList, uint64_t changeOutputs, uint64_t* transactionFee);

    // Returns the transaction fee as a string in the form x.xxx which can be used for
    // displaying the txn fee. If an error occurs, a message will be displayed on the parent.
    //
    // An empty string will be returned if there was an error.
    QString getTxnFeeString(QWidget* parent, const QString& accountName, int64_t amount,
                            wallet::Wallet* wallet, core::AppContext* appContext,
                            QStringList& txnOutputList, uint64_t changeOutputs);

    //
    // Even though you will find documentation which says the transaction fee is
    // calculated as 4*(num_outputs + num_kernels) - num_inputs that is not what is actually
    // in the grin code. The calculation they use is:
    //     (4*num_outputs) + num_kernels - num_inputs
    //
    // Using more inputs lowers the fee.
    //
    uint64_t calcTxnFee(uint64_t numInputs, uint64_t numOutputs, uint64_t numKernels);

};


#endif //MWC_QT_WALLET_UI_H
