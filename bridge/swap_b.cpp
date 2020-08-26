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

#include "swap_b.h"
#include "../wallet/wallet.h"
#include "../state/state.h"
#include "../state/s_swap.h"
#include "../state/u_nodeinfo.h"
#include "../core/appcontext.h"

namespace bridge {

static wallet::Wallet * getWallet() { return state::getStateContext()->wallet; }
static core::AppContext * getAppContext() { return state::getStateContext()->appContext; }
static state::Swap * getSwap() {return (state::Swap *) state::getState(state::STATE::SWAP); }
static state::NodeInfo * getNodeInfo() {return (state::NodeInfo *) state::getState(state::STATE::NODE_INFO); }


Swap::Swap(QObject *parent) : QObject(parent) {
    wallet::Wallet *wallet = state::getStateContext()->wallet;

    QObject::connect(wallet, &wallet::Wallet::onRequestSwapTrades,
                     this, &Swap::onRequestSwapTrades, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onDeleteSwapTrade,
                     this, &Swap::onDeleteSwapTrade, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onCreateNewSwapTrade,
                     this, &Swap::onCreateNewSwapTrade, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onRequestTradeDetails,
                     this, &Swap::onRequestTradeDetails, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onAdjustSwapData,
                     this, &Swap::onAdjustSwapData, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onCancelSwapTrade,
                     this, &Swap::onCancelSwapTrade, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onNewSwapTrade,
                     this, &Swap::onNewSwapTrade, Qt::QueuedConnection);

    state::Swap * swap = getSwap();
    QObject::connect(swap, &state::Swap::onSwapTradeStatusUpdated,
                     this, &Swap::onSwapTradeStatusUpdated, Qt::QueuedConnection);
}

Swap::~Swap() {}

// Return back to the trade list page
void Swap::pageTradeList() {
    getSwap()->pageTradeList();
}

// request the list of swap trades
void Swap::requestSwapTrades() {
    getWallet()->requestSwapTrades();
}

void Swap::onRequestSwapTrades(QVector<wallet::SwapInfo> swapTrades) {
    // Result comes in series of 5 item tuples:
    // < <Info>, <Trade Id>, <State>, <Status>, <Date>, <secondary_address> >, ....
    QVector<QString> trades;
    for (const auto & st : swapTrades) {
        trades.push_back(st.info);
        trades.push_back(st.swapId);
        trades.push_back(st.state);
        trades.push_back(getSwap()->isTradeRunning(st.swapId) ? "Running" : ( st.done ? "Done" : "Not running" ));
        QDateTime startTime;
        startTime.setTime_t(st.startTime);
        trades.push_back(startTime.toString("MMMM d yyyy hh:mm"));
        trades.push_back(st.secondaryAddress);
    }
    emit sgnSwapTradesResult( trades );
}

// Switch to New Trade Window
void Swap::startNewTrade() {
    getSwap()->startNewTrade();
}

// Cancel the trade. Send signal to the cancel the trade.
// Trade cancelling might take a while.
void Swap::cancelTrade(QString swapId) {
    getWallet()->cancelSwapTrade(swapId);
}


// Switch to editTrade Window
void Swap::viewTrade(QString swapId) {
    getSwap()->viewTrade(swapId);
}

// Switch to trade Details Window
void Swap::showTradeDetails(QString swapId) {
    getSwap()->showTradeDetails(swapId);
}

// Deleting swapId
void Swap::deleteSwapTrade(QString swapId) {
    getWallet()->deleteSwapTrade(swapId);
}

void Swap::onDeleteSwapTrade(QString swapId, QString errMsg) {
    emit sgnDeleteSwapTrade(swapId, errMsg);
}


// Get the list of secondary currencies that we are supporting
QVector<QString> Swap::getSecondaryCurrencyList() {
    return QVector<QString>{"BCH"};
}

// Calculate recommended confirmations number for MWC.
int Swap::calcConfirmationsForMwcAmount(double mwcAmount) {
    int height = getNodeInfo()->getCurrentNodeHeight();
    if (height<=0)
        return -1;

    /* Schdule (from the block)
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

// Get a minimum/maximum possible number of confiormations for the secondary currency.
QVector<int> Swap::getConfirmationLimitForSecondary(QString secondaryName) {
    if (secondaryName == "BCH") {
        return QVector<int>{1, 1000};
    }

    Q_ASSERT(false);
    QVector<int>{1, 1000};
}

// Create a new trade. Responce will be send back with a signal
void Swap::createNewTrade( double mwc, double btc, QString secondary,
                                     QString redeemAddress,
                                     bool sellerLockFirst,
                                     int messageExchangeTimeMinutes,
                                     int redeemTimeMinutes,
                                     int mwcConfirmationNumber,
                                     int secondaryConfirmationNumber,
                                     QString communicationMethod,
                                     QString communicationAddress) {

    int minConf = getAppContext()->getSendCoinsParams().inputConfirmationNumber;

    getWallet()->createNewSwapTrade( minConf, mwc, btc, secondary,
            redeemAddress,
            sellerLockFirst,
            messageExchangeTimeMinutes,
            redeemTimeMinutes,
            mwcConfirmationNumber,
            secondaryConfirmationNumber,
            communicationMethod,
            communicationAddress );
}

void Swap::onCreateNewSwapTrade(QString swapId, QString errMsg) {
    emit sgnCreateNewTradeResult(swapId, errMsg);
}

void Swap::onCancelSwapTrade(QString swapId, QString error) {
    emit sgnCancelTrade(swapId, error);
}


// Requesting all details about the single swap Trade
// Respond will be with sent back with sgnRequestTradeDetails
void Swap::requestTradeDetails(QString swapId) {
    getWallet()->requestTradeDetails(swapId);
}

QString calcTimeLeft(int64_t time) {
    int64_t curTime = QDateTime::currentSecsSinceEpoch();
    if (curTime > time)
        return 0;

    int64_t secondsLeft = time - curTime;
    int hours = secondsLeft / 3600;
    int minutes = (secondsLeft % 3600) / 60;

    return QString::number(hours) + " hours and " + QString::number(minutes) + " minutes";
}

// executionPlan, array of triplets: <active: "true"|"false">, <end_time>, <Name> >, ....
static QVector<QString> convertExecutionPlan(const QVector<wallet::SwapExecutionPlanRecord> & executionPlan) {
    QVector<QString> res;
    for (const wallet::SwapExecutionPlanRecord & exPl : executionPlan ) {
        res.push_back( exPl.active ? "true" : "false");
        res.push_back(util::timestamp2ThisTime(exPl.end_time));
        res.push_back(exPl.name);
    }
    return res;
}

static QVector<QString> convertTradeJournal( const QVector<wallet::SwapJournalMessage> & tradeJournal ) {
    QVector<QString> res;
    for ( const wallet::SwapJournalMessage & tj : tradeJournal ) {
        res.push_back(tj.message);
        res.push_back( util::timestamp2ThisTime(tj.time));
    }
    return res;
}

void Swap::onRequestTradeDetails( wallet::SwapTradeInfo swap,
                                   QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                                   QString currentAction,
                                   QVector<wallet::SwapJournalMessage> tradeJournal,
                                   QString errMsg ) {
    QVector<QString> swapInfo;

    // Response from requestTradeDetails call
    // [0] - swapId
    swapInfo.push_back(swap.swapId);

    if (!errMsg.isEmpty()) {
        emit sgnRequestTradeDetails(swapInfo, {}, currentAction, {}, errMsg);
        return;
    }

    // [1] - Description in HTML format. Role can be calculated form here as "Selling ..." or "Buying ..."
    QString description;
    if (swap.isSeller) {
        description += "Selling <b>" + QString::number( swap.mwcAmount ) + "</b> MWC for <b>" + QString::number(swap.secondaryAmount) + "</b> " +
                swap.secondaryCurrency + ". " + swap.secondaryCurrency + " redeem address: " + swap.secondaryAddress + "<br/>";
    }
    else {
        description += "Buying <b>" + QString::number( swap.mwcAmount ) + "</b> MWC for <b>" + QString::number(swap.secondaryAmount) + "</b> " + swap.secondaryCurrency + "<br/>";
    }

    description += "Requied lock confirmations: <b>" + QString::number(swap.mwcConfirmations)+"</b> for MWC and <b>"+ QString::number(swap.secondaryConfirmations) +"</b> for " + swap.secondaryCurrency + "<br/>";
    description += "Time limits: <b>"+QString::number(swap.messageExchangeTimeLimit/60)+"</b> minutes for messages exchange and <b>"+QString::number(swap.redeemTimeLimit/60)+"</b> minutes for redeem or refund<br/>";
    description += "Locking order: " + (swap.sellerLockingFirst ? "Seller lock MWC first" : ("Buyer lock " + swap.secondaryCurrency + " first")) + "<br/>";
    QString lockTime = calcTimeLeft(swap.mwcLockTime);
    if (lockTime.size()>0)
        description += "MWC funds locked until block " + QString::number(swap.mwcLockHeight) + ", expected to be mined in " + lockTime + "<br/>";
    else
        description += "MWC Lock expired.<br/>";

    QString secLockTime = calcTimeLeft(swap.secondaryLockTime);
    if (secLockTime.size()>0) {
        description += swap.secondaryCurrency + " funds locked for " + secLockTime + "<br/>";
    }
    else {
        description += swap.secondaryCurrency + " lock expired<br/>";
    }
    description += (swap.isSeller ? "Buyer" : "Seller" ) + QString(" address: ") + swap.communicationMethod + ", " + swap.communicationAddress + "<br/>";
    swapInfo.push_back(description);

    // [2] - Redeem address
    swapInfo.push_back(swap.secondaryAddress);
    // [3] - secondary currency name
    swapInfo.push_back(swap.secondaryCurrency);
    // [4] - secondary fee
    swapInfo.push_back(QString::number(swap.secondaryFee));
    // [5] - secondary fee units
    swapInfo.push_back(swap.secondaryFeeUnits);
    // [6] - communication method: mwcmqc|tor
    swapInfo.push_back(swap.communicationMethod);
    // [7] - Communication address
    swapInfo.push_back(swap.communicationAddress);

    emit sgnRequestTradeDetails( swapInfo, convertExecutionPlan(executionPlan), currentAction, convertTradeJournal(tradeJournal), errMsg );
}


// Check if this Trade is running in auto mode now
bool Swap::isRunning(QString swapId) {
    return getSwap()->isTradeRunning(swapId);
}

// Update communication method.
// Respond will be at sgnUpdateCommunication
void Swap::updateCommunication(QString swapId, QString communicationMethod, QString communicationAddress) {
    getWallet()->adjustSwapData(swapId, "destination", communicationMethod, communicationAddress);
}

// Update redeem adress.
// Respond will be at sgnUpdateRedeemAddress
void Swap::updateSecondaryAddress(QString swapId, QString secondaryAddress) {
    getWallet()->adjustSwapData(swapId, "secondary_address", secondaryAddress );
}

// Update secondary fee value for the transaction.
// Respond will come with sgnUpdateSecondaryFee
void Swap::updateSecondaryFee(QString swapId, double fee) {
    getWallet()->adjustSwapData(swapId, "secondary_fee", QString::number(fee) );
}

void Swap::onAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg) {
    if (adjustCmd == "destination") {
        emit sgnUpdateCommunication(swapId, errMsg);
    }
    else if (adjustCmd=="secondary_address") {
        emit sgnUpdateSecondaryAddress(swapId,errMsg);
    }
    else if (adjustCmd=="secondary_fee") {
        emit sgnUpdateSecondaryFee(swapId,errMsg);
    }
}


// Start swap processing
void Swap::startAutoSwapTrade(QString swapId) {
    getSwap()->runTrade(swapId);
}

// Stop swap processing
void Swap::stopAutoSwapTrade(QString swapId) {
    getSwap()->stopTrade(swapId);
}

void Swap::onSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
                              QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                              QVector<wallet::SwapJournalMessage> tradeJournal) {
    emit sgnSwapTradeStatusUpdated( swapId, currentAction, currentState,
            convertExecutionPlan(executionPlan),
            convertTradeJournal(tradeJournal));
}

void Swap::onNewSwapTrade(QString currency, QString swapId) {
    emit sgnNewSwapTrade(currency, swapId);
}


}


