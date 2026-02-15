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

#include "ui.h"
#include "../core/appcontext.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../util/stringutils.h"
#include <QVector>
#include <climits>
#include <QSet>

namespace util {

// forward declarations
static
quint64 getTxnFeeFromSpendableOutputs(qint64 amount, const QMultiMap<qint64, wallet::WalletOutput> spendableOutputs,
                                       quint64 changeOutputs, quint64 totalNanoCoins, QStringList& txnOutputList);

static int calcSubstituteIndex( QVector<wallet::WalletOutput> & resultBucket, const wallet::WalletOutput & testOutput, qint64 change ) {
    double bestWeightedGain = 0.0;
    int bestIndex = -1;

    const qint64 testWeightedValue = testOutput.valueNano;

    for (int k=0;k<resultBucket.size();k++) {
        const auto & out = resultBucket[k];
        if (out.valueNano - testOutput.valueNano > change)
            continue;

        double weightedGain = out.valueNano - testWeightedValue;
        if (weightedGain>bestWeightedGain) {
            bestWeightedGain = weightedGain;
            bestIndex = k;
        }
    }

    return bestIndex;
}

// Return resulting bucket weighted amount. Reason: we don't case about change, minimization target is weighted amount.
static double optimizeBucket( QVector<wallet::WalletOutput> & resultBucket, const QVector<wallet::WalletOutput> & inputOutputs, qint64 nanoCoins ) {

    QSet<QString> bucketCommits;
    qint64 change = -nanoCoins;
    for (const auto & out: resultBucket) {
        bucketCommits += out.outputCommitment;
        change += out.valueNano;
    }
    Q_ASSERT(change>=0); // Not enough funds?

    bool foundBetterSolution = true;
    while (foundBetterSolution) {
        foundBetterSolution = false;
        // doing single scan
        for (wallet::WalletOutput out: inputOutputs) {
            if ( bucketCommits.contains(out.outputCommitment) )
                continue;

            // Check if can substitute
            int idx = calcSubstituteIndex( resultBucket, out, change );
            while (idx>=0) {
                wallet::WalletOutput delOutput = resultBucket[idx];
                resultBucket[idx] = out;
                Q_ASSERT( out.valueNano < delOutput.valueNano );
                change -= delOutput.valueNano;
                change += out.valueNano;
                Q_ASSERT(change>=0);

                bucketCommits -= delOutput.outputCommitment;
                bucketCommits += out.outputCommitment;

                foundBetterSolution = true;

                out = delOutput;
                idx = calcSubstituteIndex( resultBucket, out, change );
            }
        }
    }

    double resultWeightedSum = 0.0;
    for (const auto & out: resultBucket)
        resultWeightedSum += out.valueNano;

    return resultWeightedSum;
}


// nanoCoins expected to include the fees. Here we are calculating the outputs that will produce minimal change
bool calcOutputsToSpend( qint64 nanoCoins, const QVector<wallet::WalletOutput> & inputOutputs, QStringList & resultOutputs ) {
    // Try 1,2,3 and 4 outputs to spend (sorry, different workflows)
    // Than - first smallest commits

    // Sorting in INC order
    QVector<wallet::WalletOutput> outputs = inputOutputs;
    std::sort(outputs.begin(), outputs.end(), [](const wallet::WalletOutput &o1, const wallet::WalletOutput &o2) {return o2.valueNano < o1.valueNano;});


    // Let's try to minimize the weighted cost for the outputs.
    // 1 Select few sets of outputs on the result
    // 2 Trying to minimize the weight by swapping the outputs from the bucket.
    // 3 Increas number of ouput by step (1 will not work and our tests show that for 3 outputs. ).
    // Repeat #2
    // If new result better or the same, go to #3
    // Done, prev result is an answer

    // Calculating min number of outputs.
    QVector<int> outputsNumber;

    // Min Number of outputs.
    {
        qint64 change = nanoCoins; // Expected to be negative
        int minOutputsNumber = 0;
        for ( const auto & out : outputs ) {
            if (change <= 0)
                break;
            change -= out.valueNano;
            minOutputsNumber++;
        }
        if(change>0)
            return false; // not enough funds

        change = nanoCoins; // Expected to be negative
        int maxOutputsNumber = 0;
        for ( auto o = outputs.rbegin(); o!=outputs.rend(); o++ ) {
            if (change <= 0)
                break;
            change -= o->valueNano;
            maxOutputsNumber++;
        }
        Q_ASSERT(change<=0);

        Q_ASSERT(minOutputsNumber<=maxOutputsNumber);

        int n=minOutputsNumber;
        for (; n<std::min(maxOutputsNumber,5); n++)
            outputsNumber.push_back(n);

        int step = std::max(1,(maxOutputsNumber-minOutputsNumber)/10);
        for (; n<maxOutputsNumber; n+=step)
            outputsNumber.push_back(n);

        outputsNumber.push_back(maxOutputsNumber);
    }
    Q_ASSERT(!outputsNumber.isEmpty());

    QVector<wallet::WalletOutput> resultBucket;
    double resultWeightAmount = 100000000.0;

    for ( int outN : outputsNumber ) {
        QVector<wallet::WalletOutput> bucket;
        for ( int t=0;t<outN; t++ ) {
            bucket.push_back(outputs[t]);
        }

        double weightedAmount = optimizeBucket( bucket, outputs, nanoCoins );
        if (resultBucket.isEmpty() || weightedAmount < resultWeightAmount) {
            resultBucket = bucket;
            resultWeightAmount = weightedAmount;
        }
    }

    Q_ASSERT(!resultBucket.isEmpty());

    for (const auto & o : resultBucket) {
        resultOutputs += o.outputCommitment;
    }
    return true;
}

// in: nanoCoins < 0 - ALL
// out: resultOutputs - what we want include into transaction. If
// Return true is any output was blocked
bool getOutputsToSend( const QString & accountPath, int outputsNumber, qint64 nanoCoins,
        wallet::Wallet * wallet,
        core::AppContext * appContext,
        QStringList & resultOutputs, quint64* txnFee ) {
    Q_ASSERT(appContext);
    Q_ASSERT(wallet);

    resultOutputs.clear();
    *txnFee = 0;  // txnFee will only be 0 if it wasn't calculated or there was an error

    // mwc713 doesn't know about HODL outputs or locked outputs
    // if we don't have HODL or locked outputs, let mwc713 select the outputs
    // !!!! Commented because isLockOutputEnabled  is about permanent user defined settings
    // For swap marketplace also there are temporary locks that we should process here
    //if (!appContext->isLockOutputEnabled())
    //    return true; // let mwc713 wallet handle it

    QVector<wallet::WalletOutput> outputs = wallet->getOutputs(accountPath, false);

    //QVector<wallet::WalletOutput> freeOuts;
    QMultiMap<qint64, wallet::WalletOutput> freeOuts;

    qint64 totalNanoCoins = 0;

    QStringList allOutputs;

    bool wasAnyOutputLocked = false;
    for (wallet::WalletOutput o : outputs) {
        // Keep unspent only
        if (o.status != "Unspent") // Interesting only in Unspent outputs
            continue;
        // Skip mined that can't spend
        if (o.coinbase && o.numOfConfirms.toLong() <= 1440)
            continue;
        if (!o.coinbase && o.numOfConfirms.toInt() < appContext->getSendCoinsParams().inputConfirmationNumber)
            continue;
        // Skip locked
        if (appContext->isLockedOutputs(o.outputCommitment).first) {
            wasAnyOutputLocked = true;
            continue;
        }

        allOutputs.push_back(o.outputCommitment);
        totalNanoCoins += o.valueNano;
        freeOuts.insert(o.valueNano, o);  // inserts by value
    }

    // nothing on this account is in HODL
    resultOutputs = allOutputs;
    *txnFee = getTxnFeeFromSpendableOutputs(nanoCoins, freeOuts, outputsNumber, totalNanoCoins, resultOutputs);
    return wasAnyOutputLocked;
}

//
// Populates a multimap with outputs available for spending using the output's value as the key.
// Return true if anything was locked
//
static bool
findSpendableOutputs(const QString& accountPath, wallet::Wallet* wallet, core::AppContext* appContext,
                     QMultiMap<qint64, wallet::WalletOutput>& spendableOutputs) {
    bool anythingWasLocked = false;
    QVector<wallet::WalletOutput>  outputs = wallet->getOutputs(accountPath, false);
    for ( wallet::WalletOutput o : outputs) {
        if ( o.status != "Unspent" ) // Interested only in Unspent outputs
            continue;
        // Skip mined that can't spend
        if (o.coinbase && o.numOfConfirms.toLong()<=mwc::COIN_BASE_CONFIRM_NUMBER )
            continue;
        if (!o.coinbase && o.numOfConfirms.toInt() < appContext->getSendCoinsParams().inputConfirmationNumber)
            continue;
        // ensure outputs locked by Qt Wallet are not used
        // !!!! Commented because isLockOutputEnabled  is about permanent user defined settings
        // For swap marketplace also there are temporary locks that we should process here
        if ( /*appContext->isLockOutputEnabled() &&*/ appContext->isLockedOutputs(o.outputCommitment).first ) {
            anythingWasLocked = true;
            continue;
        }
        spendableOutputs.insert(o.valueNano, o);
    }
    return anythingWasLocked;
}

static qint64
getTotalCoins(const QVector<wallet::WalletOutput> outputs) {
    qint64 total = 0;
    for ( wallet::WalletOutput o : outputs) {
        total += o.valueNano;
    }
    return total;
}

static qint64
getTotalCoinsFromMap(const QMultiMap<qint64, wallet::WalletOutput> outputs) {
    qint64 total = 0;
    for ( qint64 valueNano : outputs.keys()) {
        total += valueNano;
    }
    return total;
}

//
// Returns an array of the outputs to include, as inputs, in the transaction.
//
// Parameters:
//    amountNano - The amount in nano coins to spend. May or may not include the txn fee.
//    spendableOutputs - Vector of spendable outputs (sorted in the order to be used)
//
static void
retrieveTransactionInputs(qint64 amountNano, QMultiMap<qint64, wallet::WalletOutput> spendableOutputs,
                          quint64* totalNanoCoins, QVector<wallet::WalletOutput>& inputs)
{
    inputs.clear();
    if (*totalNanoCoins == 0) {
        *totalNanoCoins = getTotalCoinsFromMap(spendableOutputs);
    }

    if (amountNano < 0) {
        // send all coins
        inputs = spendableOutputs.values().toVector();
    }
    else {
        quint64 amount = amountNano;
        if (amount < *totalNanoCoins) {
            qint64 amountToSpend = amountNano;
            qint64 selectedAmount = 0L;
            for (wallet::WalletOutput o : spendableOutputs.values()) {
                if (selectedAmount < amountToSpend) {
                    inputs.push_back(o);
                    selectedAmount += o.valueNano;
                }
            }
        }
    }
}

//
// Even though you will find documentation which says the transaction fee is
// calculated as 4*(num_outputs + num_kernels) - num_inputs that is not what is actually
// in the grin code. The calculation they use is:
//     (4*num_outputs) + num_kernels - num_inputs
//
// Using more inputs lowers the fee.
//
quint64 calcTxnFee(quint64 numInputs, quint64 numOutputs, quint64 numKernels) {
    qint64 txnWeight = (4 * numOutputs) + numKernels - numInputs;
    if (1 > txnWeight) {
        // The minimum fee is 1000000 nano coin.
        txnWeight = 1;
    }
    return mwc::BASE_TRANSACTION_FEE * txnWeight;
}

//
// Calculates the transaction fee from the array of spendable outputs.
// If an error occurs, displays a message on the parent window and returns -1.
//
static
quint64 getTxnFeeFromSpendableOutputs(qint64 amount, const QMultiMap<qint64, wallet::WalletOutput> spendableOutputs,
                                       quint64 changeOutputs, quint64 totalNanoCoins, QStringList& txnOutputList) {

    quint64 totalCoins = totalNanoCoins;

    quint64 numKernels = 1;     // always 1 for now
    quint64 resultOutputs = 1;  // we always have at least 1 result output for the receiver's output

    // retrieval of the transaction inputs from the Map will result
    // in the inputs being sorted in ascending order by value
    QVector<wallet::WalletOutput> txnInputs;
    retrieveTransactionInputs(amount, spendableOutputs, &totalCoins, txnInputs);

    quint64 numInputs = txnInputs.size();
    if (numInputs == 0) {
        return 0;
    }

    quint64 txnFee = calcTxnFee(numInputs, resultOutputs, numKernels);

    quint64 amountWithFee = 0;
    if (amount < 0) {
        amountWithFee = totalCoins;
    }
    else {
       amountWithFee = amount + txnFee;
    }

    // don't recalculate the txn fee if we have spent all of our coins
    // user could have specified ALL or they could have entered the correct amount
    // which when added with the fee results in all coins being spent
    if (totalCoins != amountWithFee) {
        // find the fee where we expect change for the sender
        quint64 numOutputs = changeOutputs + resultOutputs;
        txnFee = calcTxnFee(numInputs, numOutputs, numKernels);
        amountWithFee = amount + txnFee;

        quint64 transactionTotal = 0;
        for (wallet::WalletOutput o : txnInputs) {
            transactionTotal += o.valueNano;
        }

        // check again to ensure we have enough outputs for the amount including the fee
        if (transactionTotal < amountWithFee) {
            retrieveTransactionInputs(amountWithFee, spendableOutputs, &totalCoins, txnInputs);
            numInputs = txnInputs.size();
            // only recalculate txnFee if we had inputs
            // otherwise pass out the latest txnFee so the caller can display
            // it in their error message
            if (numInputs > 0)
            {
                totalCoins = getTotalCoins(txnInputs);
                txnFee = calcTxnFee(numInputs, numOutputs, numKernels);
                amountWithFee = amount + txnFee;
            }
        }
        if (totalCoins < amountWithFee) {
            txnFee = 0;
        }
    }
    // if we were given an empty txnOutputList, populate it so that mwc713
    // will use the same outputs as we did when calculating the txn fee
    // and large number of outputs will not need to be scanned again
    if (txnFee != 0 && txnOutputList.size() == 0) {
        for (wallet::WalletOutput o : txnInputs) {
            txnOutputList.push_back(o.outputCommitment);
        }
    }

    return txnFee;
}

//
// Calculates the transaction fee for the amount specified.
// If txnOutputList is not empty, then those outputs will be used as the
// spendable outputs in the calculation. If txnOutputList is empty, then
// the spendable outputs will be retrieved from the wallet.
//
// The transaction fee is calculated based upon the default calculation
// used in mwc713 where smaller outputs are used first as the fee will
// be reduced if more spendable outputs are used as transaction inputs.
//
// The minimum transaction fee is: 1000000/1000 MWC nanocoin
//
// Parameters:
//     amount        - amount to spend in MWC nanocoins
//     txnOutputList - list of output commitments to be used in transaction (from getOutputsToSend)
//     changeOutputs - number of outputs to use for sender change outputs
// Return: <fee_amount, flag anything was locked>
//
QPair<quint64,bool> getTxnFee(const QString& accountPath, qint64 amount, wallet::Wallet* wallet,
                   core::AppContext* appContext, quint64 changeOutputs,
                   QStringList& txnOutputList) {
    // we should not have been called if the txn outputs have already been found
    if (txnOutputList.size() > 0)
        return QPair<quint64,bool>(0, false);

    quint64 txnFee = 0;
    QMultiMap<qint64, wallet::WalletOutput> spendableOutputs;
    bool anythingWasLocked = findSpendableOutputs(accountPath, wallet, appContext, spendableOutputs);

    if (spendableOutputs.size() > 0) {
        txnFee = getTxnFeeFromSpendableOutputs(amount, spendableOutputs, changeOutputs, 0, txnOutputList);
    }

    return QPair<quint64,bool>(txnFee, anythingWasLocked);
}

// Function to determine the number of significant digits
/*int getSignificantDigits(double value) {
    if (value == 0.0) return 1; // Handle zero case

    int digits = 0;
    double absValue = std::abs(value);
    while (absValue < 1.0 && absValue > 0.0) {
        absValue *= 10.0;
        digits++;
        if (digits>20)
          break;
    }
    return digits;
}*/

QString txnFeeToString(quint64 nanoTxnFee) {
    if (nanoTxnFee < mwc::BASE_TRANSACTION_FEE) {
        return "unknown";
    }
    return nano2one(nanoTxnFee);
}

QString getAllSpendableAmount2(const QString& accountPath, wallet::Wallet* wallet, core::AppContext* appContext) {
    QString allSpendableAmount = "";

    QMultiMap<qint64, wallet::WalletOutput> spendableOutputs;
    findSpendableOutputs(accountPath, wallet, appContext, spendableOutputs);
    qint64 numSpendableOutputs = spendableOutputs.size();
    if (numSpendableOutputs > 0) {
        qint64 totalSpendableCoins = getTotalCoinsFromMap(spendableOutputs);
        // we don't expect any change since we are spending all coin
        quint64 txnFee = calcTxnFee(numSpendableOutputs, 1, 1);
        if (qint64(txnFee)>=totalSpendableCoins) {
            core::getWndManager()->messageTextDlg("Too small amount", "Your total amount is not enougn to cover the transaction fees."
                            " Because of that you can't spend such a small amount. In order to spend all coins, please add more funds to this account.");
            return "";
        }
        qint64 allAmount = totalSpendableCoins - txnFee;
        allSpendableAmount = util::nano2one(allAmount);
    }
    return allSpendableAmount;
}

};
