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

#include "testCalcOutputsToSpend.h"
#include <QDebug>
#include "../util/ui.h"
#include "../wallet/wallet.h"
#include "../wallet/mwc713.h"
#include "../core/HodlStatus.h"
#include "../control_desktop/messagebox.h"
#include "../core/appcontext.h"
#include "../util/Log.h"

namespace test {

using namespace wallet;
using namespace util;
using namespace core;

// return true if equal
static bool compareStringLists(QStringList l1, QStringList l2) {
    if ( l1.size() != l2.size() )
        return false;

    l1.sort();
    l2.sort();
    for (int k=0;k<l1.size();k++) {
        if ( l1[k] != l2[k] )
            return false;
    }

    return true;
}

static void runForTestSet(int64_t nanoCoins, const QVector<wallet::WalletOutput> & outputs, QStringList expectedOutputs ) {

    QStringList resultOutputs;
    bool ok = calcOutputsToSpend( nanoCoins, outputs, resultOutputs );
    Q_ASSERT(ok);

    Q_ASSERT( compareStringLists(expectedOutputs, resultOutputs) );
}

static void runForTestSet2(int64_t nanoCoins, const QVector<wallet::WalletOutput> & outputs, int64_t maxDiff ) {

    QStringList resultOutputs;
    bool ok = calcOutputsToSpend( nanoCoins, outputs, resultOutputs );
    Q_ASSERT(ok);

    Q_ASSERT( resultOutputs.size()>0 );

    int64_t sum = 0;
    for ( const auto & s : resultOutputs) {
        sum += s.toLongLong();
    }

    Q_ASSERT( sum >= nanoCoins );
    Q_ASSERT( sum - nanoCoins <= maxDiff);
}

static void dotest_calcOutputsToSpend() {
    //
    {
        QVector<wallet::WalletOutput> outputs{
                WalletOutput::create("c1000", "", "", "", "Unspent", false, "", 1000L, 1L),
                WalletOutput::create("c2000", "", "", "", "Unspent", false, "", 2000L, 1L),
                WalletOutput::create("c4000", "", "", "", "Unspent", false, "", 4000L, 1L),
                WalletOutput::create("c8000", "", "", "", "Unspent", false, "", 8000L, 1L),
                WalletOutput::create("c16000", "", "", "", "Unspent", false, "", 16000L, 1L),
                WalletOutput::create("c32000", "", "", "", "Unspent", false, "", 32000L, 1L)
        };

        for (auto & o : outputs )
            o.weight = 1.0;

        runForTestSet( 100L, outputs, {"c1000"} );
        runForTestSet( 1000L, outputs, {"c1000"} );
        runForTestSet( 999L, outputs, {"c1000"} );
        runForTestSet( 1001L, outputs, {"c2000"} );
        runForTestSet( 1901L, outputs, {"c2000"} );
        runForTestSet( 2000L, outputs, {"c2000"} );
        runForTestSet( 2901L, outputs, {"c1000","c2000"} );
        runForTestSet( 2001L, outputs, {"c1000","c2000"} );

        runForTestSet( 7999L, outputs, {"c8000"} );
        runForTestSet( 8000L, outputs, {"c8000"} );
        runForTestSet( 8002L, outputs, {"c8000", "c1000"} );

        runForTestSet( 12000L, outputs, {"c8000", "c4000"} );
        runForTestSet( 11900L, outputs, {"c8000", "c4000"} );
        runForTestSet( 12001L, outputs, {"c8000", "c4000", "c1000"} );


        runForTestSet( 31000L, outputs, {"c1000","c2000","c4000","c8000","c16000"} );
        runForTestSet( 6875L, outputs, {"c1000", "c2000", "c4000"} );

        runForTestSet( 14001L, outputs, {"c1000","c2000","c4000","c8000"} );

        // Must be all...
        runForTestSet( 33000L, outputs, {"c1000","c32000"} );
        runForTestSet( 56000L, outputs, {"c8000","c16000","c32000"} );
        runForTestSet( 57000L, outputs, {"c1000", "c8000","c16000","c32000"} );
    }

    {
        // test for the bunch of same items
        QVector<wallet::WalletOutput> outputs;
        for (int t=0;t<1000;t++) {
            outputs.push_back( WalletOutput::create("c1000", "", "", "", "Unspent", false, "", 1000L, 1L));
        }
        runForTestSet( 1L, outputs, {"c1000"} );
        runForTestSet( 1900L, outputs, {"c1000","c1000"} );
        runForTestSet( 3000L, outputs, {"c1000","c1000","c1000"} );
        runForTestSet( 35891L, outputs, {"c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000",
                                         "c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000",
                                         "c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000",
                                         "c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000","c1000"} );
    }

    {
        QVector<wallet::WalletOutput> outputs;

        // stress test
        const int outputNum = 1000; // It is really a lot of outpts
        const int testNum = 30;

        // To debug and reproduce - make the seed contant...
        srand( static_cast<quint64>( QTime::currentTime().msecsSinceStartOfDay() ) );

        for ( int i=0; i<outputNum; i++ ) {
            int64_t amount = (rand() % 1000) * 10000 + i;

            outputs.push_back( WalletOutput::create( QString::number(amount), "", "", "", "Unspent", false, "", amount, 1L));
            outputs[i].weight = 1.0;
        }


        // Run 1 test
        for (int k=0;k<testNum;k++) {
            int idx = rand() % outputs.size();
            const wallet::WalletOutput & o1 = outputs[idx];
            runForTestSet2( o1.valueNano, outputs, 0 );
            runForTestSet2( o1.valueNano-1, outputs, 1 );
            runForTestSet2( o1.valueNano-2, outputs, 2 );
        }

        // many outputs, solution should be close to optimal value
        for (int k=0;k<testNum;k++) {
            int outNum = rand() % (outputs.size()-2) + 2;
            QSet<int> hasOutputs;

            int64_t amount = 0;

            for  (int u=0; u<outNum; u++ ) {
                if (!hasOutputs.contains(u)) {
                    hasOutputs += u;
                    amount += outputs[u].valueNano;
                }
            }

            runForTestSet2( amount, outputs, amount/1000 );
        }
    }
}

void testCalcOutputsToSpend() {
    dotest_calcOutputsToSpend();
}

}
