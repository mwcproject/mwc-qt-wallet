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
bool getOutputsToSend( const QString & accountName, int outputsNumber, int64_t nanoCoins, core::HodlStatus * hodlStatus,
        QWidget * parent, QStringList & resultOutputs ) {
    Q_ASSERT(hodlStatus);
    resultOutputs.clear();

    if ( !hodlStatus->hasAnyOutputsInHODL() )
        return true; // Nothing in HODL, let's wallet handle it

    QVector<wallet::WalletOutput>  outputs = hodlStatus->getWalltOutputsForAccount(accountName);

    QVector<QPair<wallet::WalletOutput, core::HodlOutputInfo>> hodlOuts;
    QVector<wallet::WalletOutput> freeOuts;

    int64_t freeNanoCoins = 0;

    for ( wallet::WalletOutput o : outputs) {
        if ( o.status != "Unspent" ) // Interesting only in Unspent outputs
            continue;
        if (o.coinbase && o.numOfConfirms.toLong()<=1440 )
            continue;

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
        // nothing on this account in HODL
        return true;
    }

    if (nanoCoins<0) {
        QVector<core::HodlOutputInfo> spentOuts;
        for (const auto & ho : hodlOuts )
            spentOuts.push_back(ho.second);

        // Ask user if he want ot spend all and continue...
        return control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::questionHTML(parent, "HODL Output spending",
                generateMessageHtmlOutputsToSpend( spentOuts ),
                "Cancel", "Continue", true, false, 1.4);
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
        return getOutputsToSend( accountName, outputsNumber, -1, hodlStatus, parent, resultOutputs );
    }

    // Let's ask for outptus
    QVector<core::HodlOutputInfo> hodlOuts2ask;
    for (int i=0;i<hodlOutsIdx;i++) {
        core::HodlOutputInfo & ho = hodlOuts[i].second;
        if ( hodlResultOutputs.contains( ho.outputCommitment) )
            hodlOuts2ask.push_back(ho);
    }
    Q_ASSERT( hodlOuts2ask.size() == hodlResultOutputs.size() );

    if (control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::questionHTML(parent, "HODL Output spending",
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


};
