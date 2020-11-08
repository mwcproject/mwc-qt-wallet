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

#include "s_swap.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../util/Log.h"
#include "../util/address.h"
#include "u_nodeinfo.h"
#include <QDateTime>

namespace state {

struct SecCurrencyInfo {
    QString currency;
    int     blockIntervalSec;
    int     confNumber;
    QString feeUnits;
    double  fxFee; // -1.0 - need to use API
    int64_t txFeeUpdateTime = 0;

    SecCurrencyInfo() = default;
    SecCurrencyInfo(const SecCurrencyInfo & itm) = default;
    SecCurrencyInfo & operator = (const SecCurrencyInfo & itm) = default;

    SecCurrencyInfo( const QString & _currency,
        int     _blockIntervalSec,
        int _confNumber,
        const QString & _feeUnits,
        const double & _fxFee ) :
            currency(_currency),
            blockIntervalSec(_blockIntervalSec),
            confNumber(_confNumber),
            feeUnits(_feeUnits),
            fxFee(_fxFee)
    {}
};

// Confirmations number
//  https://support.kraken.com/hc/en-us/articles/203325283-Cryptocurrency-deposit-processing-times
// Available BTC fees API
//  https://b10c.me/blog/003-a-list-of-public-bitcoin-feerate-estimation-apis/
// We selected this:  https://www.bitgo.com/api/v2/btc/tx/fee
static QVector<SecCurrencyInfo> SWAP_CURRENCY_LIST = {
        SecCurrencyInfo("BTC", 600, 6, "satoshi per byte", -1.0),
        SecCurrencyInfo("BCH", 600, 15, "satoshi per byte", 3.0 ),
};

static SecCurrencyInfo getCurrencyInfo(QString currency) {
    for (const auto & wcl : SWAP_CURRENCY_LIST) {
        if (wcl.currency == currency)
            return wcl;
    }
    Q_ASSERT(false); // NOT FOUND!!!
    return SWAP_CURRENCY_LIST[0];
}

Swap::Swap(StateContext * context) :
        State(context, STATE::SWAP)
{
    QObject::connect( context->wallet, &wallet::Wallet::onPerformAutoSwapStep, this, &Swap::onPerformAutoSwapStep, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onNewSwapTrade, this, &Swap::onNewSwapTrade, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onCreateNewSwapTrade, this, &Swap::onCreateNewSwapTrade, Qt::QueuedConnection );

    startTimer(1000); // 1 second timer is fine. Timer is for try.

    updateFeesIsNeeded();
}

Swap::~Swap() {

}

NextStateRespond Swap::execute() {
    if (context->appContext->getActiveWndState() != STATE::SWAP)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    core::getWndManager()->pageSwapList();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// Show first page with trade List
void Swap::pageTradeList() {
    core::getWndManager()->pageSwapList();
}

// Edit/View Trade Page
void Swap::viewTrade(QString swapId) {
    core::getWndManager()->pageSwapEdit(swapId);
}

// Show trade details page
void Swap::showTradeDetails(QString swapId) {
    core::getWndManager()->pageSwapTradeDetails(swapId);
}

bool Swap::isTradeRunning(const QString & swapId) const {
    return runningSwaps.contains(swapId);
}


// Run the trade
void Swap::runTrade(QString swapId) {
    AutoswapTask task;
    task.setData(swapId, 0);
    runningSwaps.insert(swapId, task);
}

void Swap::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event)

    if (runningSwaps.isEmpty() || !runningTask.isEmpty())
        return;

    int64_t curTime = QDateTime::currentSecsSinceEpoch();

    AutoswapTask nextTask;
    nextTask.lastUpdatedTime = curTime;

    for ( auto i = runningSwaps.constBegin(); i != runningSwaps.constEnd(); ++i ) {
        if ( i.value().lastUpdatedTime < nextTask.lastUpdatedTime ) {
            nextTask = i.value();
        }
    }

    // Let's run every minute, it should be enough
    if (curTime - nextTask.lastUpdatedTime > 60) {
        // starting the task
        runningTask = nextTask.swapId;
        runningSwaps[nextTask.swapId].lastUpdatedTime = curTime;
        context->wallet->performAutoSwapStep(nextTask.swapId);
    }
}

void Swap::onPerformAutoSwapStep(QString swapId, bool swapIsDone, QString currentAction, QString currentState,
                           QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                           QVector<wallet::SwapJournalMessage> tradeJournal,
                           QString error ) {

    // Checking if need to show deposit message for the buyer
    if (currentAction.contains("Please deposit exactly")) {
        if (!shownMessages.contains(currentAction)) {
            shownMessages.insert(currentAction);
            core::getWndManager()->messageTextDlg("Trade " + swapId, currentAction);
        }
    }

    if (swapId != runningTask)
        return;

    runningTask = "";

    // Running task is executed, let's update it
    if (!error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Swap Processing Error", "Autoswap step is failed for swap " + swapId + "\n\n" + error );
        return;
    }

    if (swapIsDone) {
        runningSwaps.remove(swapId);
    }

    logger::logEmit( "SWAP", "onSwapTradeStatusUpdated", swapId + ", " + currentAction + ", " + currentState );
    emit onSwapTradeStatusUpdated( swapId, currentAction, currentState, executionPlan, tradeJournal);
}

void Swap::onNewSwapTrade(QString currency, QString swapId) {
    core::getWndManager()->messageTextDlg("New Swap Offer",
             "You get a new Swap Offer to Buy " + currency + " coins for your MWC.\nTrade SwapId: " + swapId +
             "\n\nPlease reviews this offer befo run is and accept. Please check if all details like amounts, lock order, confirmation number are meet your expectations.\n\n"
             "Please remember that number of confirmations should match the amount.\n"
             "During the trade process the funds are locked. If other party abandon the trade, your funds will be locked for some time.\n\n"
             "During the whole wap trade process please keep your wallet online and this trade 'Running'");
}

// Request latest fees for the coins
void Swap::updateFeesIsNeeded() {
    int64_t curTime = QDateTime::currentSecsSinceEpoch();
    int64_t timeLimit = curTime - 60*15; // doesn't make sense more than once on 15 minutes
    for (const auto & sc : SWAP_CURRENCY_LIST) {
        if (sc.fxFee<=0.0 || sc.txFeeUpdateTime>0) {
            if ( sc.txFeeUpdateTime < timeLimit ) {
                // making request...
                if (sc.currency == "BTC") {
                    sendRequest(HTTP_CALL::GET, "https://www.bitgo.com/api/v2/btc/tx/fee",
                                "BTC", {}, "");
                }
                else {
                    Q_ASSERT(false); // New currency, update me with URL
                }
            }
        }
    }
}

void Swap::onProcessHttpResponse(bool requestOk, const QString & tag, QJsonObject & jsonRespond,
                                   const QString & param1,
                                   const QString & param2,
                                   const QString & param3,
                                   const QString & param4) {
    Q_UNUSED(param1)
    Q_UNUSED(param2)
    Q_UNUSED(param3)
    Q_UNUSED(param4)
    if (requestOk) {
        if (tag=="BTC") {
            // Example
            //   {"feePerKb":208814,"cpfpFeePerKb":208814,"numBlocks":2,"confidence":80,"multiplier":1,
            //   "feeByBlockTarget":{"1":307420,"2":208814,"3":65312,"4":50266,"6":48466,"7":34580,"9":28365,"265":25683,"313":21093,"361":14154,"601":10063,"625":6015,"673":3307,"781":3016,"994":1504,"995":1000}}
            int feePerKb = jsonRespond["cpfpFeePerKb"].toInt();
            Q_ASSERT(feePerKb>0);
            if (feePerKb>0) {
                for (auto & wcl : SWAP_CURRENCY_LIST) {
                    if (wcl.currency == "BTC") {
                        wcl.txFeeUpdateTime = QDateTime::currentSecsSinceEpoch();
                        wcl.fxFee = double(feePerKb) / 1024.0;
                        return;
                    }
                }
                Q_ASSERT(false);
            }
        }
        else {
            Q_ASSERT(false); // Unknown currency. Update me.
        }
    }
}


// New Trade Page
void Swap::initiateNewTrade() {
    updateFeesIsNeeded();
    resetNewSwapData();
    showNewTrade1();
}

// Show the trade page 1
void Swap::showNewTrade1() {
    core::getWndManager()->pageSwapNew1();
}
// Show the trade page 2
void Swap::showNewTrade2() {
    core::getWndManager()->pageSwapNew2();
}

// Apply params from trade1 and switch to trade2 panel. Expected that params are validated by the wallet(bridge)
void Swap::applyNewTrade1Params(QString acccount, QString secCurrency, QString mwcAmount, QString secAmount,
                          QString secAddress, QString sendToAddress ) {
    newSwapAccount = acccount;
    newSwapCurrency = secCurrency;
    newSwapMwc2Trade = mwcAmount;
    newSwapSec2Trade = secAmount;
    newSwapSecAddress = secAddress;
    newSwapBuyerAddress = sendToAddress;

    // Calculating the step 2 parameters ONCE.
    if (newSwapOfferExpirationTime <= 0)
        newSwapOfferExpirationTime = 60;
    if (newSwapRedeemTime<=0)
        newSwapRedeemTime = 60;

    newSwapMwcConfNumber = calcConfirmationsForMwcAmount( newSwapMwc2Trade.toDouble() );
    SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
    newSwapSecConfNumber = sci.confNumber;
    newSwapSecTxFee = sci.fxFee; // Expected that txFee is already request by API. If not, user will need to input it manually.

    core::getWndManager()->pageSwapNew2();
}

// Apply part1 params from trade2 panel. Expected that params are validated by the windows
void Swap::applyNewTrade21Params(QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks,
                           double secTxFee) {
    newSwapCurrency = secCurrency;
    newSwapOfferExpirationTime = offerExpTime;
    newSwapRedeemTime = redeemTime;
    newSwapMwcConfNumber = mwcBlocks;
    newSwapSecConfNumber = secBlocks;
    newSwapSecTxFee = secTxFee;
}

// Apply [part2 params from trade2 panel and switch to the review (panel3).
// Expected that params are validated by the wallet(bridge)
void Swap::applyNewTrade22Params(QString electrumXUrl) {
    newSwapElectrumXUrl = electrumXUrl;
    core::getWndManager()->pageSwapNew3();
}

// Create and start a new swap. The data must be submitted by that moment
// Response: onCreateStartSwap(bool ok, QString errorMessage)
void Swap::createStartSwap() {
    // By that moment it is expected that the data is verified and valid.
    // So we can start with a new swap trade. If some data is wrong, mwc713 wallet will report about that.

    int minConf = context->appContext->getSendCoinsParams().inputConfirmationNumber;

    QPair< bool, util::ADDRESS_TYPE > addressRes = util::verifyAddress(newSwapBuyerAddress);
    if ( !addressRes.first ) {
        emit onCreateStartSwap(false, "Unable to parse buyer address " + newSwapBuyerAddress);
        return;
    }

    QString addrType;

    switch (addressRes.second) {
        case util::ADDRESS_TYPE::MWC_MQ: {
            addrType = "mwcmqs";
            break;
        }
        case util::ADDRESS_TYPE::TOR: {
            addrType = "tor";
            break;
        }
        default: {
            emit onCreateStartSwap(false, "Automated swaps working only with MQS and TOR addresses.");
            return;

        }
    }

    context->wallet->createNewSwapTrade( newSwapAccount, minConf,
                                   newSwapMwc2Trade, newSwapSec2Trade, newSwapCurrency,
            newSwapSecAddress,
            newSwapSellectLockFirst,
            newSwapOfferExpirationTime,
            newSwapRedeemTime,
            newSwapMwcConfNumber,
            newSwapSecConfNumber,
            addrType,
            newSwapBuyerAddress,
            newSwapElectrumXUrl,
            "",
            false,
            "createNewSwap",
            {});

    // Continue at onCreateNewSwapTrade
}

void Swap::onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg) {
    Q_UNUSED(params)
    Q_UNUSED(dryRun)

    if (tag=="createNewSwap") {
        Q_ASSERT(!dryRun);
        Q_ASSERT(params.isEmpty());
        emit onCreateStartSwap(errMsg.isEmpty(), errMsg);

        if (errMsg.isEmpty()) {
            runTrade(swapId);
            core::getWndManager()->pageSwapList();
            core::getWndManager()->messageTextDlg("Swap Trade", "Congratulation! Your swap trade with ID " + swapId +
                                                                " is sucessfully created.");
        }
    }
}

// List of the secondary currencies that wallet support
QVector<QString> Swap::secondaryCurrencyList() {
    QVector<QString> res;
    for (const auto & scl : SWAP_CURRENCY_LIST ) {
        res.push_back(scl.currency);
    }
    return res;
}

void Swap::setCurrentSecCurrency(QString secCurrency) {
    if (secCurrency == newSwapCurrency)
        return;
    newSwapCurrency = secCurrency;
    context->appContext->setLastUsedSwapCurrency(secCurrency);
}

QString Swap::getCurrentSecCurrencyFeeUnits() const {
    return getCurrencyInfo(newSwapCurrency).feeUnits;
}


QVector<QString> Swap::getExpirationIntervals() const {
    return QVector<QString>{
        "10 minutes",           "10",
        "20 minutes",           "20",
        "30 minutes",           "30",
        "45 minutes",           "45",
        "1 hour",               "60",
        "1 hour 30 minutes",    "90",
        "2 hours",              "120",
        "3 hours",              "180",
        "5 hours",              "300",
        "8 hours",              "480",
        "12 hours",             "720",
        "18 hours",             "1080",
        "24 hours",             "1440"
    };
}

// Calculate the locking time for a NEW not yet created swap offer.
QVector<QString> Swap::getLockTime( QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks ) {
    int mwcLockTime = mwcBlocks * 60 * 11 / 10;
    SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
    int secLockTime = secBlocks * sci.blockIntervalSec * 11 / 10;

    int mwcLockTimeInterval = offerExpTime*60 + std::max( mwcLockTime, secLockTime) + offerExpTime*60 + redeemTime*60 + mwcLockTime;
    int secLockTimeInterval = mwcLockTimeInterval + redeemTime*60 + redeemTime*60 + mwcLockTime + secLockTime;

    return QVector<QString>{
            util::interval2String(mwcLockTimeInterval, false, 2),
            util::interval2String(secLockTimeInterval, false, 2),
    };
}


// Calculate recommended confirmations number for MWC.
int Swap::calcConfirmationsForMwcAmount(double mwcAmount) {
    int height = ((state::NodeInfo*)getState( STATE::NODE_INFO ))->getCurrentNodeHeight();

    if (height<=0)
        return -1;

    /* Schedule (from the block)
    202500, 2.38095238
    212580, 0.6
    385380, 0.45
    471780, 0.3
    644580, 0.25
    903780, 0.2
    1162980, 0.15
    1687140, 0.1
    2211300, 0.05
    5356260, 0.025
    */

    double gain; // Assume that Fees as such large
    if (height<385380)
        gain = 0.6;
    else if (height<471780)
        gain = 0.45;
    else if (height<644580)
        gain = 0.3;
    else if (height<903780)
        gain = 0.25;
    else if (height<1162980)
        gain = 0.2;
    else if (height<1687140)
        gain = 0.15;
    else if (height<2211300)
        gain = 0.1;
    else if (height<5356260)
        gain = 0.05;
    else
        gain = 0.025;

    double confirmations = mwcAmount / gain;
    if (confirmations<10.0)
        return 10;

    if (confirmations>10000.0)
        return 10000;

    return int(confirmations + 0.5);
}


void Swap::resetNewSwapData() {
    newSwapAccount = context->wallet->getCurrentAccountName();
    newSwapCurrency = context->appContext->getLastUsedSwapCurrency();
    newSwapMwc2Trade = "";
    newSwapSec2Trade = "";
    newSwapSecAddress = "";
    newSwapSellectLockFirst = true;
    newSwapBuyerAddress = "";
    newSwapOfferExpirationTime = -1;
    newSwapRedeemTime = -1;
    newSwapSecTxFee = 0.0;
    newSwapMwcConfNumber = -1;
    newSwapSecConfNumber = -1;
    newSwapElectrumXUrl = "";
}




}
