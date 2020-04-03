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
#include "../core/HodlStatus.h"
#include "../control/messagebox.h"

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


        runForTestSet( 31000L, outputs, {"c32000"} ); // Limited by triples... Should found non optimal solution...
        runForTestSet( 6875L, outputs, {"c1000", "c2000", "c4000"} );

        runForTestSet( 14001L, outputs, {"c16000"} ); // Limited by triples... Should found non optimal solution...

        // Must be all...
        runForTestSet( 33000L, outputs, {"c1000","c32000"} );
        runForTestSet( 56000L, outputs, {"c8000","c16000","c32000"} );
        runForTestSet( 57000L, outputs, {"c1000", "c2000","c4000","c8000","c16000","c32000"} );
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
        qsrand( static_cast<quint64>( QTime::currentTime().msecsSinceStartOfDay() ) );

        for ( int i=0; i<outputNum; i++ ) {
            int64_t amount = (qrand() % 1000) * 10000 + i;

            outputs.push_back( WalletOutput::create( QString::number(amount), "", "", "", "Unspent", false, "", amount, 1L));
        }


        // Run 1 test
        for (int k=0;k<testNum;k++) {
            int idx = qrand() % outputs.size();
            const wallet::WalletOutput & o1 = outputs[idx];
            runForTestSet2( o1.valueNano, outputs, 0 );
            runForTestSet2( o1.valueNano-1, outputs, 1 );
            runForTestSet2( o1.valueNano-2, outputs, 2 );
        }

        // 2 outputs...
        for (int k=0;k<testNum;k++) {
            int idx1 = qrand() % outputs.size();
            int idx2 = qrand() % outputs.size();
            if (idx1==idx2)
                continue;

            const wallet::WalletOutput & o1 = outputs[idx1];
            const wallet::WalletOutput & o2 = outputs[idx2];
            runForTestSet2( o1.valueNano+o2.valueNano, outputs, 0 );
            runForTestSet2( o1.valueNano+o2.valueNano-2, outputs, 2 );
        }

        // 3 outputs...
        for (int k=0;k<testNum;k++) {
            int idx1 = qrand() % outputs.size();
            int idx2 = qrand() % outputs.size();
            int idx3 = qrand() % outputs.size();
            if (idx1==idx2 || idx1==idx3 || idx2==idx3)
                continue;

            const wallet::WalletOutput & o1 = outputs[idx1];
            const wallet::WalletOutput & o2 = outputs[idx2];
            const wallet::WalletOutput & o3 = outputs[idx3];
            runForTestSet2( o1.valueNano+o2.valueNano+o3.valueNano, outputs,0);
            runForTestSet2( o1.valueNano+o2.valueNano+o3.valueNano - 3, outputs,3);
        }
    }
}


static void dotest_getOutputsToSend() {
    core::HodlStatus hodl(nullptr);

    const int64_t nano = 1000000000L;

    QVector<HodlOutputInfo> hodlOutputs{
        HodlOutputInfo::create("10", 10*nano, 10, "Class 1"),
        HodlOutputInfo::create("50", 50*nano, 11, "Class 2"), // not in wallet
        HodlOutputInfo::create("90", 90*nano, 12, "Class 3"),
    };

    QVector<wallet::WalletOutput> walletOutputs{
        WalletOutput::create("10", "", "", "", "Unspent", false, "", 10*nano, 1L), // in hodl
        WalletOutput::create("90", "", "", "", "Unspent", false, "", 90*nano, 1L), // in hodl
        WalletOutput::create("20", "", "", "", "Unspent", false, "", 20*nano, 1L),
        WalletOutput::create("30", "", "", "", "Unspent", false, "", 30*nano, 1L),
    };

    hodl.setHodlOutputs( true, hodlOutputs, "errKey" );
    hodl.setWalletOutputs( "Bob", walletOutputs, "errKey" ); //

    QStringList resultOutputs;

    // NOTE!!! getOutputsToSend reserve some transaction fees.

    // No info about this account. Ok to spend, mwc713 supose to handle that
    bool ok = getOutputsToSend( "John", 1000000*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT(resultOutputs.isEmpty());

    ok = getOutputsToSend( "Bob", 9*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20"} ) );

    ok = getOutputsToSend( "Bob", 19*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20"} ) );

    ok = getOutputsToSend( "Bob", 39*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20","30"} ) );

    ok = getOutputsToSend( "Bob", 49*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20","30"} ) );

    // Message box should be shown.  Expected user will press accept
    ok = getOutputsToSend( "Bob", 51*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20","30","10"} ) );

    // Message box should be shown.  Expected user will press reject
    control::MessageBox::messageText(nullptr, "TEST", "Reject the nect request!!!" );
    ok = getOutputsToSend( "Bob", 51*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(!ok);
    Q_ASSERT( compareStringLists(resultOutputs, {} ) );

    // Message box should be shown.  Expected user will press accept
    ok = getOutputsToSend( "Bob", 77*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20","30","90"} ) );

    // Message box should be shown.  Expected user will press accept
    ok = getOutputsToSend( "Bob", 139*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20","30","90"} ) );

    // Message box should be shown.  Expected user will press accept
    ok = getOutputsToSend( "Bob", 149*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {"20","30","90", "10"} ) );

    // Message box should be shown.  Expected user will press accept
    ok = getOutputsToSend( "Bob", 200*nano, &hodl, nullptr, resultOutputs );
    Q_ASSERT(ok);
    Q_ASSERT( compareStringLists(resultOutputs, {} ) );
}

void testCalcOutputsToSpend() {
    dotest_calcOutputsToSpend();
    dotest_getOutputsToSend();
}

}
