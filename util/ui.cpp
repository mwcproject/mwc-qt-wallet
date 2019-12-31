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

static QString generateMessageHtmlOutputsToSpend( const QVector<wallet::WalletOutput> & outputs ) {
    /*
  This transaction will include outputs registered for HODL<br>
<table>
  <tr>
    <th>Commitment</th>
    <th></th>
    <th>MWC</th>
  </tr>
  <tr>
    <td>089e22b5084906e5da629272d487eaeb67138366faf28865fcde7599d5ed5410af</td>
    <td></td>
    <td>0.154</td>
  </tr>
  <tr>
    <td>0932084ef0b3cf60c62cb65d054beafba5ae0e5cf8a7890208d714bf20b36d257d</td>
    <td></td>
    <td>0.154</td>
  </tr>
</table>
<br>Please press 'Continue' if you want to withdraw those outputs from HODL and spend them now.
*/
    QString result = "This transaction will include outputs registered for HODL<br>"
                     "<table>"
                     "  <tr>"
                     "    <th>Commitment</th>"
                     "    <th></th>"
                     "    <th>MWC</th>"
                     "  </tr>"
                     "  <tr>";

    int limit = 7;
    for (const wallet::WalletOutput & outpt : outputs) {
        limit--;
        if (limit==0) {
            result += "<tr><td> .......... </td><td></td><td> ... </td></tr>";
        }
        else {
            result += "<tr><td>" + outpt.outputCommitment + "</td><td></td><td>" + util::nano2one(outpt.valueNano) +
                      "</td></tr>";
        }

    }
    result += "</table><br>Please press 'Continue' if you want to withdraw those outputs from HODL and spend them now.";
    return result;
}

// return difference and result
// If no result was found, put amount is a big number
static QPair< int64_t, QVector<wallet::WalletOutput> > calcOutputsToSpend( int64_t nanoCoins, const QVector<wallet::WalletOutput> & inputOutputs, int outputsIdx0, int stackLimit ) {
    int64_t bestChange = LONG_MAX;
    QVector<wallet::WalletOutput> walletOutputs;

    for ( int i=outputsIdx0; i<inputOutputs.size(); i++ ) {
        const wallet::WalletOutput & o = inputOutputs[i];

        if (stackLimit>0) {
            if (o.valueNano < nanoCoins) {
                QPair<int64_t, QVector<wallet::WalletOutput> > nextRes = calcOutputsToSpend(nanoCoins - o.valueNano,
                                                           inputOutputs, i + 1, stackLimit - 1);
                if (!nextRes.second.isEmpty() && nextRes.first < bestChange) {
                    bestChange = nextRes.first;
                    walletOutputs = nextRes.second;
                    walletOutputs.push_back(o);
                }
            }
            else {
                // Done with stack
                return QPair< int64_t, QVector<wallet::WalletOutput> >(bestChange, walletOutputs);
            }
        }
        else {
            if (o.valueNano >= nanoCoins)
                return QPair< int64_t, QVector<wallet::WalletOutput> >( o.valueNano-nanoCoins,{o} );
        }
    }

    return QPair< int64_t, QVector<wallet::WalletOutput> >(bestChange, walletOutputs);
}

// nanoCoins expected to include the fees. Here we are calculating the outputs that will produce minimal change
bool calcOutputsToSpend( int64_t nanoCoins, const QVector<wallet::WalletOutput> & inputOutputs, QStringList & resultOutputs ) {
    // Try 1,2,3 and 4 outputs to spend (sorry, different workflows)
    // Than - first smallest commits

    QVector<wallet::WalletOutput> outputs = inputOutputs;
    std::sort(outputs.begin(), outputs.end(), [](const wallet::WalletOutput &o1, const wallet::WalletOutput &o2) {return o1.valueNano < o2.valueNano;});

    QPair< int64_t, QVector<wallet::WalletOutput> >  oneCommitRes = calcOutputsToSpend( nanoCoins, outputs, 0, 0 );
    QPair< int64_t, QVector<wallet::WalletOutput> >  twoCommitRes = calcOutputsToSpend( nanoCoins, outputs, 0, 1 );
    QPair< int64_t, QVector<wallet::WalletOutput> >  threeCommitRes = calcOutputsToSpend( nanoCoins, outputs, 0, 2 );

    // It is a case with bunch of small outptus.
    if ( oneCommitRes.second.isEmpty() && twoCommitRes.second.isEmpty()  && threeCommitRes.second.isEmpty() ) {
        // add outputs one by one
        for ( const auto & o1 : outputs ) {
            if (nanoCoins<=0)
                break;

            resultOutputs += o1.outputCommitment;
            nanoCoins -= o1.valueNano;
        }
        // false if not enough outputs. Edge case. Expected that mwc713 will process it
        return nanoCoins<=0;
    }

    QVector<wallet::WalletOutput> * resOutputs = nullptr;

    if ( oneCommitRes.first <= twoCommitRes.first && oneCommitRes.first <= threeCommitRes.first ) {
        resOutputs = &oneCommitRes.second;
    } else if ( twoCommitRes.first <= oneCommitRes.first && twoCommitRes.first <= threeCommitRes.first ) {
        resOutputs = &twoCommitRes.second;
    } else if ( threeCommitRes.first <= twoCommitRes.first && threeCommitRes.first <= oneCommitRes.first ) {
        resOutputs = &threeCommitRes.second;
    }
    else {
        Q_ASSERT(false); // expected to be full covered
    }

    Q_ASSERT(resOutputs!= nullptr && resOutputs->size()>0);

    if (resOutputs == nullptr || resOutputs->size()==0) {
        Q_ASSERT(false);
        return false;
    }

    for (const auto & o : *resOutputs) {
        resultOutputs += o.outputCommitment;
    }
    return true;

}


// in: nanoCoins < 0 - ALL
// out: resultOutputs - what we want include into transaction. If
// return false if User cancel this action.
bool getOutputsToSend( const QString & accountName, int64_t nanoCoins, core::HodlStatus * hodlStatus, QWidget * parent, QStringList & resultOutputs ) {
    Q_ASSERT(hodlStatus);
    resultOutputs.clear();

    if ( !hodlStatus->hasAnyOutputsInHODL() )
        return true; // Nothing in HODL, let's wallet handle it

    QVector<wallet::WalletOutput>  outputs = hodlStatus->getWalltOutputsForAccount(accountName);

    QVector<wallet::WalletOutput> hodlOuts;
    QVector<wallet::WalletOutput> freeOuts;

    int64_t freeNanoCoins = 0;

    for ( const auto & o : outputs) {
        if ( o.status != "Unspent" ) // Interesting only in Unspent outputs
            continue;

        if ( hodlStatus->isOutputInHODL(o.outputCommitment) )
            hodlOuts.push_back(o);
        else {
            freeOuts.push_back(o);
            freeNanoCoins += o.valueNano;
        }
    }

    if (hodlOuts.size()==0) {
        // nothing on this account in HODL
        return true;
    }

    if (nanoCoins<0) {
        // Ask user if he want ot spend all and continue...
        return control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::questionHTML(parent, "HODL Output spending",
                generateMessageHtmlOutputsToSpend( hodlOuts ),
                "Cancel", "Continue", true, false);
    }

    Q_ASSERT(nanoCoins>0);

    // Update with possible fee. QT wallet doens't know the fee amount, that is why put 0.02 fee that cover everything possible
    nanoCoins += util::one2nano("0.02").second;

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
        return res;
    }

    // Need to spend some HODL outputs
    QStringList hodlResultOutputs;
    bool res = calcOutputsToSpend( nanoCoins - freeNanoCoins, hodlOuts, hodlResultOutputs );
    if (!res) {
        // spend all case, very possible because we don't control the balance
        return getOutputsToSend( accountName, -1, hodlStatus, parent, resultOutputs );
    }

    // Let's ask for outptus
    QVector<wallet::WalletOutput> hodlOuts2ask;
    for (const auto & o : hodlOuts) {
        if ( hodlResultOutputs.contains(o.outputCommitment) )
            hodlOuts2ask.push_back(o);
    }
    Q_ASSERT( hodlOuts2ask.size() == hodlResultOutputs.size() );

    if (control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::questionHTML(parent, "HODL Output spending",
                        generateMessageHtmlOutputsToSpend( hodlOuts2ask ),
                        "Cancel", "Continue", true, false) )
        return false;

    // User approve the spending, preparing the list of outputs...
    resultOutputs = hodlResultOutputs;
    for ( const auto & o : freeOuts )
        resultOutputs.push_back( o.outputCommitment );

    return true;
}


};
