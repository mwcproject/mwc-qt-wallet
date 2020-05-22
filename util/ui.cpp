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
#include "../core/HodlStatus.h"
#include "../core/appcontext.h"
#include "../core/global.h"
#include "../control/messagebox.h"
#include "../util/stringutils.h"
#include <QVector>
#include <climits>
#include <QSet>

namespace util {

static QString generateMessageHtmlOutputsToSpend( const QVector<core::HodlOutputInfo> & outputs ) {
    /*
  This transaction will include outputs registered for HODL<br>
<table>
  <tr>
    <th>Commitment</th>
    <th></th>
    <th>MWC</th>
    <th>&nbsp;&nbsp;&nbsp;</th>
  </tr>
  <tr>
    <td>089e22b5084906e5da629272d487eaeb67138366faf28865fcde7599d5ed5410af</td>
    <td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td>0.154</td>
    <th></th>
  </tr>
  <tr>
    <td>0932084ef0b3cf60c62cb65d054beafba5ae0e5cf8a7890208d714bf20b36d257d</td>
    <td></td>
    <td>0.154</td>
    <th></th>
  </tr>
</table>
<br><br>
<br>Please press 'Continue' if you want to withdraw those outputs from HODL and spend them now.
*/
    QString result = "This transaction will include outputs registered for HODL<br>"
                     "<table>"
                     "  <tr>"
                     "    <th>Commitment</th>"
                     "    <th></th>"
                     "    <th>Class</th>"
                     "    <th></th>"
                     "    <th>MWC</th>"
                     "    <th>&nbsp;&nbsp;&nbsp;</th>"
                     "  </tr>"
                     "  <tr>";

    int limit = 7;
    for (const core::HodlOutputInfo & outpt : outputs) {
        limit--;
        if (limit==0) {
            result += "<tr><td> .......... </td><td></td><td></td><td></td><td> ... </td></tr>";
            break;
        }
        else {
            result += "<tr><td>" + outpt.outputCommitment + "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td>" +
                    outpt.cls + "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td>" +
                    util::trimStrAsDouble( QString::number(outpt.value), 7) +
                      "</td><td></td></tr>";
        }

    }
    result += "</table><br><br><br>Please press 'Continue' if you want to withdraw those outputs from HODL and spend them now.";
    return result;
}

static int calcSubstituteIndex( QVector<wallet::WalletOutput> & resultBucket, const wallet::WalletOutput & testOutput, int64_t change ) {
    double bestWeightedGain = 0.0;
    int bestIndex = -1;

    const double testWeightedValue = testOutput.getWeightedValue();

    for (int k=0;k<resultBucket.size();k++) {
        const auto & out = resultBucket[k];
        if (out.valueNano - testOutput.valueNano > change)
            continue;

        double weightedGain = out.getWeightedValue() - testWeightedValue;
        if (weightedGain>bestWeightedGain) {
            bestWeightedGain = weightedGain;
            bestIndex = k;
        }
    }

    return bestIndex;
}

// Return resulting bucket weighted amount. Reason: we don't case about change, minimization target is weighted amount.
static double optimizeBucket( QVector<wallet::WalletOutput> & resultBucket, const QVector<wallet::WalletOutput> & inputOutputs, int64_t nanoCoins ) {

    QSet<QString> bucketCommits;
    int64_t change = -nanoCoins;
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
                Q_ASSERT( out.getWeightedValue() < delOutput.getWeightedValue() );
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
        resultWeightedSum += out.getWeightedValue();

    return resultWeightedSum;
}


// nanoCoins expected to include the fees. Here we are calculating the outputs that will produce minimal change
bool calcOutputsToSpend( int64_t nanoCoins, const QVector<wallet::WalletOutput> & inputOutputs, QStringList & resultOutputs ) {
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
        int64_t change = nanoCoins; // Expected to be negative
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
// return false if User cancel this action.
bool getOutputsToSend( const QString & accountName, int outputsNumber, int64_t nanoCoins,
        wallet::Wallet * wallet,
        core::HodlStatus * hodlStatus,
        core::AppContext * appContext,
        QWidget * parent, QStringList & resultOutputs ) {
    Q_ASSERT(hodlStatus);
    Q_ASSERT(appContext);
    Q_ASSERT(wallet);

    resultOutputs.clear();

    // mwc713 doesn't know about HODL outputs or locked outputs
    // if we don't have HODL or locked outputs, let mwc713 select the outputs
    if ( !hodlStatus->hasAnyOutputsInHODL() && !appContext->isLockOutputEnabled() )
        return true; // let mwc713 wallet handle it

    QVector<wallet::WalletOutput>  outputs = wallet->getwalletOutputs().value(accountName);

    QVector<QPair<wallet::WalletOutput, core::HodlOutputInfo>> hodlOuts;
    QVector<wallet::WalletOutput> freeOuts;

    int64_t freeNanoCoins = 0;

    QStringList allOutputs;

    for ( wallet::WalletOutput o : outputs) {
        // Keep unspent only
        if ( o.status != "Unspent" ) // Interesting only in Unspent outputs
            continue;
        // Skip mined that can't spend
        if (o.coinbase && o.numOfConfirms.toLong()<=1440 )
            continue;
        if (!o.coinbase && o.numOfConfirms.toInt() < appContext->getSendCoinsParams().inputConfirmationNumber)
            continue;
        // Skip locked
        if (appContext->isLockedOutputs(o.outputCommitment))
            continue;

        allOutputs.push_back(o.outputCommitment);

        core::HodlOutputInfo ho = hodlStatus->getHodlOutput("", o.outputCommitment);

        if ( ho.weight > 0.0 ) {
            o.weight = ho.weight;
            hodlOuts.push_back(QPair<wallet::WalletOutput, core::HodlOutputInfo>(o, ho));
        }
        else {
            o.weight = 0.01;
            freeOuts.push_back(o);
            freeNanoCoins += o.valueNano;
        }
    }

    if (hodlOuts.size()==0) {
        // nothing on this account is in HODL
        if (appContext->isLockOutputEnabled())
            resultOutputs = allOutputs;
        return true;
    }

    if (nanoCoins<0) {
        // handle spending all spendable outputs
        QVector<core::HodlOutputInfo> spentOuts;
        for (const auto & ho : hodlOuts )
            spentOuts.push_back(ho.second);

        // Ask user if he want ot spend all and continue...
        if (control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::questionHTML(parent, "HODL Output Spending",
                generateMessageHtmlOutputsToSpend( spentOuts ),
                "Cancel", "Continue", true, false, 1.4) ) {
            if (appContext->isLockOutputEnabled())
                resultOutputs = allOutputs;
            return true;
        }
        else {
            return false;
        }
    }

    std::sort( hodlOuts.begin(), hodlOuts.end(), [](const QPair<wallet::WalletOutput, core::HodlOutputInfo> & a, const QPair<wallet::WalletOutput, core::HodlOutputInfo> & b) {
        return a.second.weight < b.second.weight;
    });

    Q_ASSERT(nanoCoins>0);

    // Update with possible fee. QT wallet doens't know the fee amount, that is why put 0.02 fee that cover everything possible
    nanoCoins += 8000000L + std::max(0,outputsNumber-1) * 4000000L;

    // Calculate what outputs need to be selected...
    if (freeNanoCoins >= nanoCoins) {
        // Let's select optimal set of output to spend...
        bool res = calcOutputsToSpend( nanoCoins, freeOuts, resultOutputs );
        Q_ASSERT(res); // We have amount, no reasons to fail.
        if (!res) {
            QStringList outputs2print;
            for (const auto & o : freeOuts)
                outputs2print.push_back(o.toString());

            control::MessageBox::messageText(parent, "Internal Error", "calcOutputsToSpend internal error for available outputs. nanoCoins=" +
                QString::number(nanoCoins) + " freeNanoCoins=" + QString::number(freeNanoCoins) + " Number of outputs:" + QString::number(freeOuts.size()) +
                "\nOutputs:\n" + outputs2print.join("\n")
            );
        }

        // 1 - resulting output.  outputsNumber - change outputs
        // Limit 500 will be checked on the waalet side
        if (resultOutputs.size() + outputsNumber + 1 > 498 ) {
            control::MessageBox::messageText(parent, "Send Amount", "We can't send such large amount of the coins in a single transaction because your outputs are too small. Please send smaller amount.");
            return false;
        }

        return res;
    }

    // Need to spend some HODL outputs
    QStringList hodlResultOutputs;

    int64_t hodlCoins = nanoCoins - freeNanoCoins;
    QVector<wallet::WalletOutput> spentOuts;
    double lastW = 0.0;
    int hodlOutsIdx=0;
    for (;hodlOutsIdx<hodlOuts.size() && hodlCoins>0; hodlOutsIdx++) {
        spentOuts.push_back( hodlOuts[hodlOutsIdx].first );
        lastW = hodlOuts[hodlOutsIdx].second.weight;
        hodlCoins -= hodlOuts[hodlOutsIdx].first.valueNano;
    }
    for (;hodlOutsIdx<hodlOuts.size();hodlOutsIdx++) {
        if ( lastW < hodlOuts[hodlOutsIdx].second.weight)
            break;
        spentOuts.push_back( hodlOuts[hodlOutsIdx].first );
    }

    bool res = calcOutputsToSpend( nanoCoins - freeNanoCoins, spentOuts, hodlResultOutputs );
    if (!res) {
        // spend all case, very possible because we don't control the balance
        return getOutputsToSend( accountName, outputsNumber, -1, wallet, hodlStatus, appContext, parent, resultOutputs );
    }

    // Let's ask for outptus
    QVector<core::HodlOutputInfo> hodlOuts2ask;
    for (int i=0;i<hodlOutsIdx;i++) {
        core::HodlOutputInfo & ho = hodlOuts[i].second;
        if ( hodlResultOutputs.contains( ho.outputCommitment) )
            hodlOuts2ask.push_back(ho);
    }
    Q_ASSERT( hodlOuts2ask.size() == hodlResultOutputs.size() );

    if (control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::questionHTML(parent, "HODL Output Spending",
                        generateMessageHtmlOutputsToSpend( hodlOuts2ask ),
                        "Cancel", "Continue", true, false, 1.4) )
        return false;

    // 1 - resulting output.  outputsNumber - change outputs
    // Limit 500 will be checked on the waalet side
    if (hodlResultOutputs.size()+freeOuts.size() + outputsNumber + 1 > 498 ) {
        control::MessageBox::messageText(parent, "Send Amount", "We can't send such large amount of the coins in a single transaction because your outputs are too small. Please send smaller amount.");
        return false;
    }

    // User approve the spending, preparing the list of outputs...
    resultOutputs = hodlResultOutputs;
    for ( const auto & o : freeOuts )
        resultOutputs.push_back( o.outputCommitment );

    return true;
}

//
// Given a list of output commitments, returns an array of the wallet outputs associated with
// those commitments. The array is sorted in ascending order by coin value.
//
static QVector<wallet::WalletOutput>
getSortedWalletOutputs(const QString& accountName, const wallet::Wallet* wallet, const QStringList& outputList) {
    QVector<wallet::WalletOutput> walletOutputs;
    if (outputList.size() > 0) {
        QVector<wallet::WalletOutput> outputs = wallet->getwalletOutputs().value(accountName);
        for (wallet::WalletOutput o : outputs) {
            if (outputList.contains(o.outputCommitment)) {
                walletOutputs.push_back(o);
            }
        }
        // ensure the outputs are in ascending order so smaller outputs are spent first
        // as the fee will be less when more outputs are used
        std::sort(walletOutputs.begin(), walletOutputs.end(), [](const wallet::WalletOutput &o1, const wallet::WalletOutput &o2) {return o1.valueNano < o2.valueNano;});
    }
    return walletOutputs;
}

//
// Returns an array of outputs which are available for spending.
// The array is sorted in ascending order by coin value.
//
static QVector<wallet::WalletOutput>
getSpendableOutputs(const QString& accountName, const wallet::Wallet* wallet, core::AppContext* appContext) {
    QVector<wallet::WalletOutput> spendableOutputs;

    QVector<wallet::WalletOutput>  outputs = wallet->getwalletOutputs().value(accountName);
    for ( wallet::WalletOutput o : outputs) {
        if ( o.status != "Unspent" ) // Interested only in Unspent outputs
            continue;
        // Skip mined that can't spend
        if (o.coinbase && o.numOfConfirms.toLong()<=mwc::COIN_BASE_CONFIRM_NUMBER )
            continue;
        if (!o.coinbase && o.numOfConfirms.toInt() < appContext->getSendCoinsParams().inputConfirmationNumber)
            continue;
        if (appContext->isLockedOutputs(o.outputCommitment))
            continue;
        spendableOutputs.push_back(o);
    }
    std::sort(spendableOutputs.begin(), spendableOutputs.end(), [](const wallet::WalletOutput &o1, const wallet::WalletOutput &o2) {return o1.valueNano < o2.valueNano;});
    return spendableOutputs;
}

static int64_t
getTotalCoins(const QVector<wallet::WalletOutput> outputs) {
    int64_t total = 0;
    for ( wallet::WalletOutput o : outputs) {
        total += o.valueNano;
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
static QVector<wallet::WalletOutput>
getTransactionInputs(int64_t amountNano, QVector<wallet::WalletOutput> spendableOutputs)
{
    QVector<wallet::WalletOutput> inputs;

    int64_t total = getTotalCoins(spendableOutputs);
    if (total >= amountNano) {
        if (amountNano < 0) {
            // send all coins
            inputs = spendableOutputs;
        }
        else {
            int64_t amountToSpend = amountNano;
            int64_t selectedAmount = 0L;
            for ( wallet::WalletOutput o : spendableOutputs) {
                if (selectedAmount < amountToSpend) {
                    inputs.push_back(o);
                    selectedAmount += o.valueNano;
                }
            }
        }
    }
    return inputs;
}

//
// Even though you will find documentation which says the transaction fee is
// calculated as 4*(num_outputs + num_kernels) - num_inputs that is not what is actually
// in the grin code. The calculation they use is:
//     (4*num_outputs) + num_kernels - num_inputs
//
// Using more inputs lowers the fee.
//
static uint64_t
calcTxnFee(uint64_t numInputs, uint64_t numOutputs=1, uint64_t numKernels=1) {
    uint64_t txnWeight = (4 * numOutputs) + numKernels - numInputs;
    if (1 > txnWeight) {
        // The minimum fee is 1000000 nano coin.
        txnWeight = 1;
    }
    return mwc::BASE_TRANSACTION_FEE * txnWeight;
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
// The minimum transaction fee is: 1000000 MWC nanocoin
//
// Parameters:
//     amount        - amount to spend in MWC nanocoins
//     txnOutputList - list of output commitments to be used in transaction (from getOutputsToSend)
//     changeOutputs - number of outputs to use for sender change outputs
//
bool getTxnFee(QWidget* parent, const QString& accountName, int64_t amount,
               wallet::Wallet* wallet, core::AppContext* appContext,
               QStringList& txnOutputList, uint64_t changeOutputs, uint64_t* transactionFee) {

    *transactionFee = 0;
    bool feeResult = true;
    uint64_t numKernels = 1;     // always 1 for now
    uint64_t resultOutputs = 1;  // we always have at least 1 result output for the receiver's output

    QVector<wallet::WalletOutput> spendableOutputs;
    if (txnOutputList.size() > 0) {
        // the unspent outputs to use were already calculated prior to our being called
        spendableOutputs = getSortedWalletOutputs(accountName, wallet, txnOutputList);
    }
    else {
        spendableOutputs = getSpendableOutputs(accountName, wallet, appContext);
        // populate the txnOutputList so that mwc713 uses the same outputs as
        // we did when calculating the txn fee
        txnOutputList.clear();
        for (wallet::WalletOutput o : spendableOutputs) {
            txnOutputList.push_back(o.outputCommitment);
        }
    }
    QVector<wallet::WalletOutput> txnInputs = getTransactionInputs(amount, spendableOutputs);
    uint64_t numInputs = txnInputs.size();
    if (numInputs == 0) {
        control::MessageBox::messageText(parent, "Send Amount Error", "Cannot find enough spendable outputs to cover amount.");
        return false;
    }
    if (numInputs + resultOutputs + changeOutputs > mwc::MWC_MAX_OUTPUTS) {
        control::MessageBox::messageText(parent, "Send Amount Error", "Maximum outputs of 500 exceeded. Amount to send comes from more outputs than allowed. Please send a smaller amount.");
        return false;
    }

    uint64_t totalCoins = getTotalCoins(txnInputs);
    uint64_t txnFee = calcTxnFee(numInputs, resultOutputs, numKernels);

    uint64_t amountWithFee = 0;
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
        uint64_t numOutputs = changeOutputs + resultOutputs;
        txnFee = calcTxnFee(numInputs, numOutputs, numKernels);
        amountWithFee = amount + txnFee;

        // check again to ensure we have enough outputs for the amount including the fee
        if (totalCoins < amountWithFee) {
            txnInputs = getTransactionInputs(amountWithFee, spendableOutputs);
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
            feeResult = false;
        }
    }
    *transactionFee = txnFee;

    return feeResult;
}

//
// Returns a string with the transaction fee in the form: x.xxx
//
QString getTxnFeeString(QWidget* parent, const QString& accountName, int64_t amount,
                        wallet::Wallet* wallet, core::AppContext* appContext,
                        QStringList& txnOutputList, uint64_t changeOutputs) {

    uint64_t txnFee = 0;
    bool txnFeeResult = getTxnFee(parent, accountName, amount, wallet, appContext, txnOutputList, changeOutputs, &txnFee);
    double dTxnFee = (double)txnFee / (double)mwc::NANO_MWC;
    QString fee = QString::number(dTxnFee);
    if (!txnFeeResult && txnFee != 0) {
       control::MessageBox::messageText(parent, "Send Amount Error", "Cannot find enough outputs available for sending amount plus " + fee + " transaction fee.");
    }
    if (!txnFeeResult) {
        // reset fee string to be empty
        fee = "";
    }
    return fee;
}

};
