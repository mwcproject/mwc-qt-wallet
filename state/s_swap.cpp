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
#include "../bridge/wnd/swap_b.h"
#include <QThread>
#include "../core/TimerThread.h"
#include "s_mktswap.h"
#include <QDir>

namespace state {

struct SecCurrencyInfo {
    QString currency;
    int     blockIntervalSec;
    int     confNumber;
    QString feeUnits;
    double  txFee; // -1.0 - need to use API
    double  txFeeMin;
    double  txFeeMax;
    int64_t txFeeUpdateTime = 0;
    double  minAmount; // Minimal amount for secondary currency. We don't want dust transaction

    SecCurrencyInfo() = default;
    SecCurrencyInfo(const SecCurrencyInfo & itm) = default;
    SecCurrencyInfo & operator = (const SecCurrencyInfo & itm) = default;

    SecCurrencyInfo( const QString & _currency,
        int     _blockIntervalSec,
        int _confNumber,
        const double & _minAmount,
        const QString & _feeUnits,
        const double & _txFee,
        const double & _txFeeMin,
        const double & _txFeeMax
    ) :
            currency(_currency),
            blockIntervalSec(_blockIntervalSec),
            confNumber(_confNumber),
            feeUnits(_feeUnits),
            txFee(_txFee),
            txFeeMin(_txFeeMin),
            txFeeMax(_txFeeMax),
            minAmount(_minAmount)
    {}
};

// Confirmations number
//  https://support.kraken.com/hc/en-us/articles/203325283-Cryptocurrency-deposit-processing-times
// Available BTC fees API
//  https://b10c.me/blog/003-a-list-of-public-bitcoin-feerate-estimation-apis/
// We selected this:  https://www.bitgo.com/api/v2/btc/tx/fee
static QVector<SecCurrencyInfo> SWAP_CURRENCY_LIST = {
        SecCurrencyInfo("BTC", 600, 3,  0.001, "satoshi per byte", -1.0, 1.0, 500.0 ),
        SecCurrencyInfo("BCH", 600, 15,  0.001, "satoshi per byte", 3.0, 1.0, 50.0 ),
        SecCurrencyInfo("LTC", 60*2+30, 12, 0.01, "litoshi per byte", 100.0, 1.0, 1000.0 ),
        SecCurrencyInfo("ZCash", 75, 24, 0.01, "ZEC", 0.0001, 0.00005, 0.001 ),
        SecCurrencyInfo("Dash", 60 * 2 + 39, 6, 0.01, "duff per byte", 26.0, 1.0, 1000.0 ),
        SecCurrencyInfo("Doge", 60, 20, 100.0, "doge", 3.0, 0.1, 20.0 ),
};

static SecCurrencyInfo getCurrencyInfo(const QString & currency) {
    for (const auto & wcl : SWAP_CURRENCY_LIST) {
        if (wcl.currency == currency)
            return wcl;
    }
    Q_ASSERT(false); // NOT FOUND!!!
    return SWAP_CURRENCY_LIST[0];
}


/////////////////////////////////////////////////////////////

Swap::Swap(StateContext * context) :
        State(context, STATE::SWAP)
{
    QObject::connect( context->wallet, &wallet::Wallet::onPerformAutoSwapStep, this, &Swap::onPerformAutoSwapStep, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onNewSwapTrade, this, &Swap::onNewSwapTrade, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onCreateNewSwapTrade, this, &Swap::onCreateNewSwapTrade, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onLoginResult, this, &Swap::onLoginResult, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onLogout, this, &Swap::onLogout, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onRequestSwapTrades, this, &Swap::onRequestSwapTrades, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onCancelSwapTrade, this, &Swap::onCancelSwapTrade, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onRestoreSwapTradeData, this, &Swap::onRestoreSwapTradeData, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onRequestTradeDetails, this, &Swap::onRequestTradeDetails, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onAdjustSwapData, this, &Swap::onAdjustSwapData, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onBackupSwapTradeData, this, &Swap::onBackupSwapTradeData, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onNewSwapMessage, this, &Swap::onNewSwapMessage, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onSendMarketplaceMessage, this, &Swap::onSendMarketplaceMessage, Qt::QueuedConnection );

    // You get an offer to swap BCH to MWC. SwapID is ffa15dbd-85a9-4fc9-a3c0-4cfdb144862b
    // Listen to a new swaps...

    timer = new core::TimerThread(this, 1000);
    QObject::connect( timer, &core::TimerThread::onTimerEvent, this, &Swap::onTimerEvent, Qt::QueuedConnection );
    timer->start();

    updateFeesIsNeeded();
}

Swap::~Swap() {
    timer->stop();
    timer->wait();

    delete timer;
}

NextStateRespond Swap::execute() {
    selectedPage = SwapWnd::None;
    if (context->appContext->getActiveWndState() != STATE::SWAP)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (!context->appContext->pullCookie<QString>("SwapShowNewTrade1").isEmpty())
        showNewTrade1();
    else
        pageTradeList(false, false, false);

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// Show first page with trade List
void Swap::pageTradeList(bool selectIncoming, bool selectOutgoing, bool selectBackup) {
    selectedPage = SwapWnd::PageSwapList;
    core::getWndManager()->pageSwapList(selectIncoming, selectOutgoing, selectBackup);
}

// Edit/View Trade Page
void Swap::viewTrade(QString swapId, QString stateCmd) {
    selectedPage = SwapWnd::PageSwapEdit;
    core::getWndManager()->pageSwapEdit(swapId, stateCmd);
}

// Show trade details page
void Swap::showTradeDetails(QString swapId) {
    selectedPage = SwapWnd::PageSwapTradeDetails;
    core::getWndManager()->pageSwapTradeDetails(swapId);
}

bool Swap::isTradeRunning(const QString & swapId) const {
    return runningSwaps.contains(swapId);
}

QVector<QString> Swap::getRunningTrades() const {
    QVector<QString> res;
    for (const auto & sw : runningSwaps)
        res.append(sw.swapId);

    return res;
}

QVector<QString> Swap::getRunningCriticalTrades() const {
    QVector<QString> res;
    for (const auto & sw : runningSwaps) {
        if ( context->appContext->getMaxBackupStatus(sw.swapId, bridge::getSwapBackup(sw.stateCmd)) >=2 )
            res.append(sw.swapId);
    }
    return res;
}

// Run the trade
void Swap::runTrade(QString swapId, QString tag, bool isSeller, QString statusCmd) {
    if (swapId.isEmpty())
        return;

    AutoswapTask task;
    task.setData(swapId, tag, isSeller, statusCmd, 0);
    runningSwaps.insert(swapId, task);
}

void Swap::onTimerEvent() {
    updateFeesIsNeeded();

    if (runningSwaps.isEmpty() || !runningTask.isEmpty())
        return;

    int64_t curMsec = QDateTime::currentMSecsSinceEpoch();
    if (curMsec-lastProcessedTimerData < 500)
        return; // Skipping this event, Q migth be overloaded

    lastProcessedTimerData = curMsec;

    int64_t curTime = curMsec/1000;

    AutoswapTask nextTask;
    nextTask.lastUpdatedTime = curTime;

    for ( auto i = runningSwaps.constBegin(); i != runningSwaps.constEnd(); ++i ) {
        if ( i.value().lastUpdatedTime < nextTask.lastUpdatedTime ) {
            nextTask = i.value();
        }
    }

    // Let's run every 60 seconds, it should be enough
    if (curTime - nextTask.lastUpdatedTime > 60) {
        // starting the task
        runningSwaps[nextTask.swapId].lastUpdatedTime = curTime;

        // Let's check if the backup is needed..
        int taskBkId = bridge::getSwapBackup(nextTask.stateCmd);
        int expBkId = context->appContext->getSwapBackStatus(nextTask.swapId);
        if (taskBkId > expBkId) {
                // Note, we are in the eventing loop, so modal will create a new one and soon timer will be called!!!

                QString backupDir = context->appContext->getSwapBackupDir();
                // QDir::separator does return value that rust doesn't understand well. That will be corrected but still it looks bad.
                QString backupFn = backupDir + "/trade_" + nextTask.swapId + "_" + QString::number(taskBkId) + ".trade";
                context->wallet->backupSwapTradeData(nextTask.swapId, backupFn);

                // continue on onBackupSwapTradeData
                return;
        }

        runningTask = nextTask.swapId;

        bool waiting4backup = /*context->appContext->getSwapEnforceBackup() &&*/ expBkId==0;
        logger::logInfo( "SWAP", "Swap processing step for " + nextTask.swapId + ", " + nextTask.stateCmd + " ,waiting4backup=" + (waiting4backup?"true":"false") );
        context->wallet->performAutoSwapStep(nextTask.swapId, waiting4backup);
    }
    lastProcessedTimerData = QDateTime::currentMSecsSinceEpoch();
}

void Swap::onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage) {
    // accepting in any case
    if (runningSwaps.contains(swapId))
        runningSwaps[swapId].lastUpdatedTime = 0; // to trigger processing and update

    if (!errorMessage.isEmpty()) {
        pageTradeList(false, false, true);
        core::getWndManager()->messageTextDlg("Error", "Wallet is unable to backup atomic swap trade at\n\n" + exportedFileName +
                "\n\n" + errorMessage +
                "\n\nPlease setup your backup directory and backup this trade.");
    }
    else {
        // Updating backup id that is done.
        int taskBkId = bridge::getSwapBackup( runningSwaps[swapId].stateCmd );
        context->appContext->setSwapBackStatus(swapId, taskBkId);
    }
}


void Swap::onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                           QString lastProcessError,
                           QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                           QVector<wallet::SwapJournalMessage> tradeJournal,
                           QString error ) {

    // Checking if need to show deposit message for the buyer
    if (currentAction.contains("Please deposit exactly")) {
        // Let's  do reminder every 20 minutes.
        int64_t curTime = QDateTime::currentSecsSinceEpoch();
        int64_t timeLimit = curTime - 60*20;

        if (shownMessages.value(currentAction) < timeLimit) {
            shownMessages.insert( currentAction, curTime + 3600*24 );
            core::getWndManager()->messageTextDlg("Trade " + swapId, currentAction);
            curTime = QDateTime::currentSecsSinceEpoch();
            shownMessages.insert( currentAction, curTime );
        }
    }

    if (swapId != runningTask)
        return;

    runningTask = "";

    // Running task is executed, let's update it
    if (!error.isEmpty()) {
        //core::getWndManager()->messageTextDlg("Swap Processing Error", "Autoswap step is failed for swap " + swapId + "\n\n" + error );
        emit onSwapTradeStatusUpdated( swapId, stateCmd, currentAction, currentState, error, executionPlan, tradeJournal);
        return;
    }

    if (runningSwaps.contains(swapId)) {
        if (runningSwaps[swapId].stateCmd != stateCmd) {
            runningSwaps[swapId].stateCmd = stateCmd;
            runningSwaps[swapId].lastUpdatedTime = 0; // process to the next step now (Backup need to be asked quickly)
        }
    }

    if ( bridge::isSwapDone(stateCmd)) {
        runningSwaps.remove(swapId);

        // Trigger refresh with "SwapListWnd"
        // Note!!!! It is a hack, but it really reduce complexity of this case!!!
        context->wallet->requestSwapTrades("SwapListWnd");
    }

    logger::logEmit( "SWAP", "onSwapTradeStatusUpdated", swapId + ", " + stateCmd + ", " + currentAction + ", " + currentState + ", " + lastProcessError );
    emit onSwapTradeStatusUpdated( swapId, stateCmd, currentAction, currentState, lastProcessError, executionPlan, tradeJournal);
}

void Swap::onNewSwapTrade(QString currency, QString swapId) {
    Q_UNUSED(currency)
    // requesting swap details to view
    context->wallet->requestTradeDetails(swapId, true );
}

void Swap::onNewSwapMessage(QString swapId) {
    // Let's try to process now
    if (runningSwaps.contains(swapId))
        runningSwaps[swapId].lastUpdatedTime = 0; // next tick will be ours to move forward
}

// Response from requestTradeDetails
void Swap::onRequestTradeDetails( wallet::SwapTradeInfo swap,
                            QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                            QString currentAction,
                            QVector<wallet::SwapJournalMessage> tradeJournal,
                            QString error ) {
    Q_UNUSED(executionPlan)
    Q_UNUSED(currentAction)
    Q_UNUSED(tradeJournal)

    if (!error.isEmpty()) {
        qDebug() << "onRequestTradeDetails get an error: " << error;
        // In case of error not much what we want to do. The error will be shown in notifications logs
        return;
    }

    // Let's check if it is something what we want to process automatically.
    if (!swap.isSeller) {
        QVector<MySwapOffer> offers = expectedOffers.value(swap.communicationAddress);
        MySwapOffer foundOffer;

        for (const auto & o : offers) {
            if (!o.offer.sell && o.offer.equal(swap)) {
                foundOffer = o;
                break;
            }
        }

        if (!foundOffer.offer.isEmpty()) {
            // This offer looks good we can accept it and assign a tag...
            context->wallet->adjustSwapData( swap.swapId, "marketplace_adjustment;" + foundOffer.offer.id,
                    "", "",
                    foundOffer.secAddress,
                    QString::number(foundOffer.secFee),
                    "",
                    foundOffer.offer.id);

            // Here we can say congrats. We are on the track to accept it. Unless no errors happens, we should be good.
            core::getWndManager()->messageTextDlg("Offer is accepted", "Congratulations, you offer " + foundOffer.getOfferDescription() +
                    " is accepted, the swap trade is started. Please note, we will keep this offer active until some of your partners will lock the coins" );
        }
        else {
            QVector<MktSwapOffer> accOffers = acceptedOffers.value(swap.communicationAddress);
            MktSwapOffer mktFoundOffer;
            for (const auto & o : accOffers) {
                if (!o.sell && o.equal(swap)) {
                    mktFoundOffer = o;
                    break;
                }
            }

            // It is a regular swap offer
            const QString title = "New Swap Offer";
            QString msg = "You have received a new Swap Offer to exchange MWC for your " + swap.secondaryCurrency + ".\n\nTrade SwapId: " + swap.swapId +
                                "\n\nPlease review this offer before you accept it. Check if the amounts, lock order, and confirmation number meet your expectations.\n\n"
                                "Double check that the number of confirmations are match the amount.";

            if (!mktFoundOffer.isEmpty()) {
                // Accepted Sell order (I am buyer)
                // Don't need to ask for acceptance, just need go to review

                // Adjusting tag and nothing else. We will review it...
                swap.tag = swap.communicationAddress + "_" + mktFoundOffer.id;
                context->wallet->adjustSwapData( swap.swapId, "XXX",
                        "", "",
                        "",
                        "",
                        "",
                        swap.tag);

                msg = "You have received a response from Swap Marketplace to exchange MWC for your " + swap.secondaryCurrency + ".\n\nTrade SwapId: " + swap.swapId +
                                    "\n\nPlease review this offer before you accept it. Check if the amounts, lock order, and confirmation number meet your expectations.\n\n"
                                    "Double check that the number of confirmations are match the amount.\n\n"
                                    "Please note, that if several peers accetp the offer, whoever lock funds first will continue to atomic swap trade, the rest will get a message that offer will be dropped. "
                                    "If you get such message, please don't deposit your coins to the lock account.";

            }

            // Normal P2P swap usecase.
            if (mwc::isWalletLocked()) {
                core::getWndManager()->messageTextDlg(title, msg);
            }
            else {
               if (core::WndManager::RETURN_CODE::BTN2 == core::getWndManager()->questionTextDlg( title, msg,
                                             "Check Later", "Review and Accept",
                                             "Later I will switch to the swap page and check it", "Review and Accept the trade now",
                                             false, true) ) {
                        // Switching to the review page...
                        viewTrade(swap.swapId, "BuyerOfferCreated");
               }
            }
        }
    }
}

void Swap::onAdjustSwapData(QString swapId, QString call_tag, QString errMsg) {
    if (!errMsg.isEmpty())
        return; // will be printed ion notifocations.

    if (call_tag.startsWith("marketplace_adjustment")) {
        // Now accept the offer
        context->appContext->setTradeAcceptedFlag(swapId, true);
        // Start monitoring
        int idx = call_tag.indexOf(';');
        Q_ASSERT(idx>0);
        runTrade(swapId, call_tag.mid(idx+1), false, "BuyerOfferCreated");
    }
}



// Request latest fees for the coins
void Swap::updateFeesIsNeeded() {
    int64_t curTime = QDateTime::currentSecsSinceEpoch();
    int64_t timeLimit = curTime - 60*15; // doesn't make sense more than once on 15 minutes
    for (const auto & sc : SWAP_CURRENCY_LIST) {
        if (sc.txFee<=0.0 || sc.txFeeUpdateTime>0) {
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
                        wcl.txFee = double(feePerKb) / 1024.0;
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

    QDir("Folder").exists();
    QString backupDir = context->appContext->getSwapBackupDir();
    if (!verifyBackupDir())
        return;
    showNewTrade1();
}

bool Swap::verifyBackupDir() {
    QString backupDir = context->appContext->getSwapBackupDir();
    if (backupDir.isEmpty() || !QDir(backupDir).exists()) {
        pageTradeList(false, false, true);
        core::getWndManager()->messageTextDlg("Warning", "Atomic swap trade backup directory is not properly set. Please set it up before start trading.");
        return false;
    }

    return true;
}

// Show the trade page 1
void Swap::showNewTrade1() {
    selectedPage = SwapWnd::PageSwapNew1;
    core::getWndManager()->pageSwapNew1();
}
// Show the trade page 2
void Swap::showNewTrade2() {
    selectedPage = SwapWnd::PageSwapNew2;
    core::getWndManager()->pageSwapNew2();
}

// Apply params from trade1, not need to check
void Swap::applyNewTrade11Params(bool mwcLockFirst) {
    newSwapSellectLockFirst = mwcLockFirst;
}

// Apply params from trade1 and switch to trade2 panel. Expected that params are validated by the wallet(bridge)
void Swap::applyNewTrade12Params(QString acccount, QString secCurrency, QString mwcAmount, QString secAmount,
                          QString secAddress, QString sendToAddress ) {

    bool need2recalc1 = (newSwapMwc2Trade != mwcAmount);
    bool need2recalc2 = (newSwapCurrency2recalc != secCurrency);

    newSwapAccount = acccount;
    newSwapCurrency = secCurrency;
    newSwapMwc2Trade = mwcAmount;
    newSwapSec2Trade = secAmount;
    newSwapSecAddress = secAddress;
    newSwapBuyerAddress = sendToAddress;

    if (need2recalc1) {
        newSwapMwcConfNumber = calcConfirmationsForMwcAmount(newSwapMwc2Trade.toDouble());
    }

    if (newSwapRedeemTime<=0 || newSwapOfferExpirationTime <= 0 || need2recalc1) {
        if (newSwapMwcConfNumber > 3000)
            newSwapOfferExpirationTime = 300;
        else if (newSwapMwcConfNumber > 1800)
            newSwapOfferExpirationTime = 180;
        else if (newSwapMwcConfNumber > 1200)
            newSwapOfferExpirationTime = 120;
        else if (newSwapMwcConfNumber > 900)
            newSwapOfferExpirationTime = 90;
        else
            newSwapOfferExpirationTime = 60;

        newSwapRedeemTime = newSwapOfferExpirationTime;
    }

    if (need2recalc2) {
        SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
        newSwapSecConfNumber = sci.confNumber;
        newSwapSecTxFee = sci.txFee; // Expected that txFee is already request by API. If not, user will need to input it manually.
        newSwapMinSecTxFee = sci.txFeeMin;
        newSwapMaxSecTxFee = sci.txFeeMax;
        newSwapCurrency2recalc = secCurrency;
    }
    selectedPage = SwapWnd::PageSwapNew2;
    core::getWndManager()->pageSwapNew2();
}

double Swap::getSecTransactionFee(const QString & secCurrency) const {
    SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
    return sci.txFee;
}


double Swap::getSecMinTransactionFee(const QString & secCurrency) const {
    SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
    return sci.txFeeMin;
}

double Swap::getSecMaxTransactionFee(const QString & secCurrency) const {
    SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
    return sci.txFeeMax;
}

int Swap::getMwcConfNumber(double mwcAmount) const {
    return std::min(1000, calcConfirmationsForMwcAmount(mwcAmount));
}
int Swap::getSecConfNumber(const QString & secCurrency) const {
    SecCurrencyInfo sci = getCurrencyInfo(secCurrency);
    return sci.confNumber;
}


// Apply part1 params from trade2 panel. Expected that params are validated by the windows
void Swap::applyNewTrade21Params(QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks,
                           double secTxFee) {
    newSwapCurrency = secCurrency;
    newSwapOfferExpirationTime = offerExpTime;
    newSwapRedeemTime = redeemTime;
    newSwapMwcConfNumber = mwcBlocks;
    newSwapSecConfNumber = secBlocks;
    newSwapSecTxFee = std::max( newSwapMinSecTxFee, std::min(secTxFee, newSwapMaxSecTxFee) );
}

// Apply [part2 params from trade2 panel and switch to the review (panel3).
// Expected that params are validated by the wallet(bridge)
void Swap::applyNewTrade22Params(QString electrumXUrl) {
    newSwapElectrumXUrl = electrumXUrl;
    selectedPage = SwapWnd::PageSwapNew3;
    core::getWndManager()->pageSwapNew3();
}

// Create and start a new swap. The data must be submitted by that moment
// Response: onCreateStartSwap(bool ok, QString errorMessage)
void Swap::createStartSwap() {
    // By that moment it is expected that the data is verified and valid.
    // So we can start with a new swap trade. If some data is wrong, mwc713 wallet will report about that.

    int minConf = context->appContext->getSendCoinsParams().inputConfirmationNumber;

    QPair<QString, util::ADDRESS_TYPE> addressRes = util::verifyAddress(newSwapBuyerAddress);
    if (!addressRes.first.isEmpty()) {
        emit onCreateStartSwap(false, "Unable to parse buyer address " + newSwapBuyerAddress + ", " + addressRes.first);
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

    if (!mktOfferId.isEmpty())
    {
        context->wallet->sendMarketplaceMessage("accept_offer", newSwapBuyerAddress, mktOfferId, "Swap");
        // continue at onSendMarketplaceMessage
        return;
    }

    context->wallet->createNewSwapTrade( newSwapAccount, {}, minConf,
                                   newSwapMwc2Trade, newSwapSec2Trade, newSwapCurrency,
            newSwapSecAddress,
            newSwapSecTxFee,
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
            mktOfferId.isEmpty() ? "" : newSwapBuyerAddress + "_" + mktOfferId,
            {});
    // Continue at onCreateNewSwapTrade
}


// Response from sendMarketplaceMessage
void Swap::onSendMarketplaceMessage(QString error, QString response, QString offerId, QString walletAddress, QString cookie) {
    if (cookie!="Swap")
        return;

    if (!error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Unable to accept offer from "+walletAddress+" because of the error:\n" + error);
        return;
    }

    // Check the response. It is a Json, but we can parse it manually. We need to find how many offers are in process.
    // "{"running":2}"
    int idx1 = response.indexOf(':');
    int idx2 = response.indexOf('}', idx1+1);
    int running_num = 100;
    bool ok = false;
    if (idx1<idx2 && idx1>0) {
        running_num = response.mid(idx1+1, idx2-idx1-1).trimmed().toInt(&ok);
    }

    if (!ok) {
        core::getWndManager()->messageTextDlg("Error", "Unable to accept offer from "+walletAddress+" because of unexpected response:\n" + response);
        return;
    }

    if (running_num<0) { // The offer is taken
        core::getWndManager()->messageTextDlg("Not on the market", "Sorry, this offer is not on the market any more, recently it was fulfilled.");
        return;
    }

    if (running_num> mktRunningNum) {
        // There are something already going, let's report it.
        if ( core::WndManager::RETURN_CODE::BTN1 != core::getWndManager()->questionTextDlg("Warning",
                                       "Wallet "+walletAddress+" already has " + QString::number(running_num) + " accepted trades. Only one trade that lock "
                                       "coins first will continue, the rest will be cancelled. As a result your trade might be cancelled even you lock the coins.\n\n"
                                       "You can wait for some time, try to accept this offer later. Or you can continue, you trade might win.\n\n "
                                       "Do you want to continue and start trading?",
                                       "Yes", "No",
                                       "I understand the risk and I want to continue", "No, I will better wait",
                                       false, true) )
        {
            return;
        }
    }

    int minConf = context->appContext->getSendCoinsParams().inputConfirmationNumber;

    context->wallet->createNewSwapTrade( newSwapAccount, {}, minConf,
                                         newSwapMwc2Trade, newSwapSec2Trade, newSwapCurrency,
                                         newSwapSecAddress,
                                         newSwapSecTxFee,
                                         newSwapSellectLockFirst,
                                         newSwapOfferExpirationTime,
                                         newSwapRedeemTime,
                                         newSwapMwcConfNumber,
                                         newSwapSecConfNumber,
                                         "tor",
                                         newSwapBuyerAddress,
                                         newSwapElectrumXUrl,
                                         "",
                                         false,
                                         "createNewSwap",
                                         mktOfferId.isEmpty() ? "" : newSwapBuyerAddress + "_" + mktOfferId,
                                         {});
    // Continue at onCreateNewSwapTrade
}


void Swap::onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg) {
    Q_UNUSED(params)
    Q_UNUSED(dryRun)

    if (tag.startsWith("createNewSwap")) {
        Q_ASSERT(!dryRun);
        Q_ASSERT(params.isEmpty());
        emit onCreateStartSwap(errMsg.isEmpty(), errMsg);

        if (errMsg.isEmpty()) {
            // Updating the note
            if (!newSwapNote.isEmpty())
                context->appContext->updateNote("swap_" + swapId, newSwapNote);

            runTrade(swapId, tag, true, "SellerOfferCreated");
            if (tag == "createNewSwap") {
                showTradeDetails(swapId);
                core::getWndManager()->messageTextDlg("Swap Trade", "Congratulation! Your swap trade with ID\n" + swapId +
                                                                "\nwas successfully created.");
            }
        }
    }
}

// Wallet just cancelled the swap. We need to stop execute it.
void Swap::onCancelSwapTrade(QString swapId, QString error) {
    if (error.isEmpty()) {
       runningSwaps.remove(swapId);
    }
}

void Swap::onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage) {
    Q_UNUSED(importedFilename)
    if (errorMessage.isEmpty()) {
        AutoswapTask task;
        task.setData(swapId, "", true, "", 0);
        runningSwaps.insert(swapId, task);
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
    // Add 5 blocks that we reserve because of 'non-final' tx pool feature on BTC side.
    int secLockTimeInterval = mwcLockTimeInterval + redeemTime*60 + redeemTime*60 + mwcLockTime + secLockTime + sci.blockIntervalSec * 5;

    return QVector<QString>{
            util::interval2String(mwcLockTimeInterval, false, 2),
            util::interval2String(secLockTimeInterval, false, 2),
    };
}

// Accept a new trade and start run it. By that moment the trade must abe reviews and all set
void Swap::acceptTheTrade(QString swapId) {
    if (!verifyBackupDir())
        return;

    // Update config...
    context->appContext->setTradeAcceptedFlag(swapId, true);

    // Start monitoring
    runTrade(swapId,"", false, "BuyerOfferCreated");

    // Switch to the trades lists
    pageTradeList(true, false, false);
}

// Get Tx fee for secondary currency
double Swap::getSecondaryFee(QString secCurrency) {
    for (const SecCurrencyInfo & ci : SWAP_CURRENCY_LIST ) {
        if (ci.currency == secCurrency)
            return ci.txFee;
    }

    return -1.0;
}


// Calculate recommended confirmations number for MWC.
int Swap::calcConfirmationsForMwcAmount(double mwcAmount) const {
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
    mktOfferId = "";
    mktRunningNum = 0;
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
    newSwapMinSecTxFee = 0.0;
    newSwapMaxSecTxFee = 0.0;
    newSwapMwcConfNumber = -1;
    newSwapSecConfNumber = -1;
    newSwapElectrumXUrl = "";
    newSwapCurrency2recalc = "";
    newSwapNote = "";
}

void Swap::onLoginResult(bool ok) {
    if (ok) {
        qDebug() << "Requesting swap trades...";
        context->wallet->requestSwapTrades("SwapInitRequest");
    }
}

// Logout event
void Swap::onLogout() {
    if (!runningSwaps.isEmpty()) {
        int sz = runningSwaps.size();
        runningSwaps.clear();
        core::getWndManager()->messageTextDlg("WARNING", "Because of the logout, " + QString::number(sz) +
                    " swap trade"+ (sz>1 ? "s":"") +" are stopped. Please login back into your wallet as soon as possible. "
                    "The wallet need to be active until the swap trade is finished. Otherwise you can loose the monet involved in this trade");
    }
}

void Swap::runSwapIfNeed(const wallet::SwapInfo & sw) {
    if (runningSwaps.contains(sw.swapId))
        return;

    bool need2accept = false;
    if (bridge::isSwapWatingToAccept(sw.stateCmd))
        need2accept = !context->appContext->isTradeAccepted(sw.swapId);

    if (!bridge::isSwapDone(sw.stateCmd) && !need2accept)
        runTrade(sw.swapId, sw.tag, sw.isSeller, sw.stateCmd);
}

// Response from requestSwapTrades
void Swap::onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error) {
    if (cookie!="SwapInitRequest") {
        // Watching case. Let's remove the finished trades and reviews if we are running non finished.
        for (const wallet::SwapInfo & sw : swapTrades) {
            if (bridge::isSwapDone(sw.stateCmd)) {
                runningSwaps.remove(sw.swapId);
            }
            else {
                runSwapIfNeed(sw);
            }
        }

        return;
    }

    if (!error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Unable to request swap trades. If you have running trades, please solve this problem as soon as possible. If your wallet is not online, you might loose your trade funds.\n\n" + error);
        return;
    }

    // Now starting the swaps
    runningSwaps.clear();

    for (const wallet::SwapInfo & sw : swapTrades) {
        runSwapIfNeed(sw);
    }
}

bool Swap::mobileBack() {
    switch (selectedPage) {
        case SwapWnd::None :
        case SwapWnd::PageSwapList:
            return false;
        case SwapWnd::PageSwapEdit:
        case SwapWnd::PageSwapTradeDetails:
        case SwapWnd::PageSwapNew1: {
            pageTradeList(false,false,false);
            return true;
        }
        case SwapWnd::PageSwapNew2: {
            core::getWndManager()->pageSwapNew1();
            selectedPage = SwapWnd::PageSwapNew1;
            return true;
        }
        case SwapWnd::PageSwapNew3: {
            core::getWndManager()->pageSwapNew2();
            selectedPage = SwapWnd::PageSwapNew2;
            return true;
        }
    }
}

// Get minimal Amount for the secondary currency
double Swap::getSecMinAmount(QString secCurrency) const {
    return getCurrencyInfo(secCurrency).minAmount;
}

// Notify about failed bidding. If it is true, we need to cancel and show the message about that
// Note, mwc-wallet does cancellation as well.
void Swap::failBidding(QString wallet_tor_address, QString offer_id) {
    QString tag = wallet_tor_address + "_" + offer_id;

    for (const AutoswapTask & val : runningSwaps.values() ) {
        if (val.tag == tag) {
            QString message = "Trade " + val.swapId + " is dropped by your peer because there was several trades and another one locked the coins first.";
            if (val.isSeller) {
                message += " This trade is cancelled.";
            }
            else {
                message += " Please don't deposit any coins to Locking account and cancel this trade manually.\n"
                           "If you already lock you coins please wait when they will be mined and then cancel your trade";
            };
            core::getWndManager()->messageTextDlg("Warning", message);
        }
    }
}

// Start trading for my offer - automatic operation. For Sell the offer will be created.
// For Buy - offer will be accepted automatically.
void Swap::startTrading(const MySwapOffer & offer, QString wallet_tor_address) {

    // Creating a new trade. My Swap offer should have all needed info...
    // Output are must be selected and locked when we created this trade
    if (offer.offer.sell) {
        QVector<QString> outputs = context->appContext->getLockedOutputsById( offer.offer.id );
        Q_ASSERT(!outputs.isEmpty());

        newSwapNote = offer.note;
        context->wallet->createNewSwapTrade( offer.account, outputs , 1,
                                             QString::number(offer.offer.mwcAmount),
                                             QString::number(offer.offer.secAmount),
                                             offer.offer.secondaryCurrency,
                                             offer.secAddress,
                                             offer.secFee,
                                             !offer.offer.sell, // Lock must be done by another party
                                             60,
                                             60,
                                             offer.offer.mwcLockBlocks,
                                             offer.offer.secLockBlocks,
                                             "tor",
                                             wallet_tor_address,
                                             "",
                                             "",
                                             false,
                                             "createNewSwapMkt", // we will start silently.
                                             offer.offer.id,
                                             {});

        // Continue at onCreateNewSwapTrade
    }
    else {
        // If it is Buy, we are expecting other party to start

        QVector<MySwapOffer> expected = expectedOffers.value(wallet_tor_address);
        QString offerId = offer.offer.id;
        std::remove_if( expected.begin(), expected.end(), [offerId](const MySwapOffer & off) {return off.offer.id == offerId;});
        expected.push_back(offer);
        expectedOffers.insert(wallet_tor_address, expected);
    }
}

// Start trading for Marketplace offer. It will start for sell only. For Buy the normal workflow should be fine, we will only press "Accept" automatically.
// For Buy - offer will be accepted automatically.
void Swap::acceptOffer(const MktSwapOffer & offer, QString wallet_tor_address, int running_num) {
    QVector<MktSwapOffer> accepted = acceptedOffers.value(wallet_tor_address);
    QString offerId = offer.id;
    std::remove_if( accepted.begin(), accepted.end(), [offerId](const MktSwapOffer & off) {return off.id == offerId;});
    accepted.push_back(offer);

    acceptedOffers.insert(wallet_tor_address, accepted);

    if (!offer.sell) {
        // It is buy offer, so I am a seeler. Let's create it manually...
        resetNewSwapData();
        mktRunningNum = running_num;
        mktOfferId = offer.id;
        newSwapCurrency = offer.secondaryCurrency;
        newSwapCurrency2recalc = newSwapCurrency;
        newSwapMwc2Trade = QString::number(offer.mwcAmount);
        newSwapSec2Trade = QString::number(offer.secAmount);
        newSwapSellectLockFirst = true;
        newSwapBuyerAddress = wallet_tor_address;
        newSwapOfferExpirationTime = 60;
        newSwapRedeemTime = 60;
        SecCurrencyInfo sci = getCurrencyInfo(newSwapCurrency);
        newSwapSecTxFee = sci.txFee; // Expected that txFee is already request by API. If not, user will need to input it manually.
        newSwapMinSecTxFee = sci.txFeeMin;
        newSwapMaxSecTxFee = sci.txFeeMax;
        newSwapMwcConfNumber = offer.mwcLockBlocks;
        newSwapSecConfNumber = offer.secLockBlocks;

        // because it it different state and we don't want to switch it back,
        // we need to chenge the satet to SWAP
        context->appContext->pushCookie<QString>("SwapShowNewTrade1", "yes");
        context->stateMachine->setActionWindow( state::STATE::SWAP );

        //showNewTrade1();
        return;
    }

    // For Buy let's check if there is already a request...
    // TODO Implement auto accept code, acceptedOffers  has all accepted
//    core::getWndManager()->messageTextDlg("Waiting for offer", "We notify Wallet " + wallet_tor_address + " should initate atomic swap trade soon. "
//        "This trade will be accepted atomatically. Please be online to lock your " + offer.secondaryCurrency + " coins.\n\n"
//        "Please note, if several traders accept the same offers, trader who lock funds first will be able to finish the trade. You will be notified if your offer will be rejected because of that.");
}

// Reject any offers from this address. We don't want them, we are likely too late
void Swap::rejectOffer(const MktSwapOffer & offer, QString wallet_tor_address) {
    Q_UNUSED(offer)
    acceptedOffers.remove(wallet_tor_address);
}

// check if swap with this tag is exist
bool Swap::isSwapExist(QString tag) const {
    for (const AutoswapTask & val : runningSwaps.values() ) {
        if (val.tag == tag)
            return true;
    }
    return false;
}


}
