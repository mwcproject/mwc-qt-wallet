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
#include "../util/address.h"
#include "../core/MessageMapper.h"

namespace bridge {

static wallet::Wallet * getWallet() { return state::getStateContext()->wallet; }
static core::AppContext * getAppContext() { return state::getStateContext()->appContext; }
static state::Swap * getSwap() {return (state::Swap *) state::getState(state::STATE::SWAP); }
//static state::NodeInfo * getNodeInfo() {return (state::NodeInfo *) state::getState(state::STATE::NODE_INFO); }

static QString mapMwc713Message(const QString & message) {
    static notify::MessageMapper mapper(":/resource/mwc713_mappers.txt");
    return mapper.processMessage(message);
}

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

    QObject::connect(wallet, &wallet::Wallet::onBackupSwapTradeData,
                     this, &Swap::onBackupSwapTradeData, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onRestoreSwapTradeData,
                     this, &Swap::onRestoreSwapTradeData, Qt::QueuedConnection);

    state::Swap * swap = getSwap();
    QObject::connect(swap, &state::Swap::onSwapTradeStatusUpdated,
                     this, &Swap::onSwapTradeStatusUpdated, Qt::QueuedConnection);
    QObject::connect(swap, &state::Swap::onCreateStartSwap,
                     this, &Swap::onCreateStartSwap, Qt::QueuedConnection);

}

Swap::~Swap() {}

// Return back to the trade list page
void Swap::pageTradeList() {
    getSwap()->pageTradeList();
}

// request the list of swap trades
void Swap::requestSwapTrades(QString cookie) {
    getWallet()->requestSwapTrades(cookie);
}

void Swap::onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error) {
    // Result comes in series of 11 item tuples:
    // < <bool is Seller>, <mwcAmount>, <sec+amount>, <sec_currency>, <Trade Id>, <StateCmd>, <State>, <initiate_time>, <expire_time>  <secondary_address> <last_process_error> >, ....
    QVector<QString> trades;
    for (const auto & st : swapTrades) {
        trades.push_back( st.isSeller ? "true" : "false" );
        trades.push_back( st.mwcAmount );
        trades.push_back( st.secondaryAmount );
        trades.push_back( st.secondaryCurrency );
        trades.push_back(st.swapId);
        trades.push_back(st.stateCmd);
        if (st.action.isEmpty() || st.action=="None")
            trades.push_back(st.state);
        else
            trades.push_back(st.action);

        trades.push_back(QString::number(st.startTime));
        trades.push_back(QString::number(st.expiration));
        trades.push_back(st.secondaryAddress);
        trades.push_back( mapMwc713Message(st.lastProcessError) );
    }

    emit sgnSwapTradesResult( cookie, trades, error );
}

// Cancel the trade. Send signal to the cancel the trade.
// Trade cancelling might take a while.
void Swap::cancelTrade(QString swapId) {
    getWallet()->cancelSwapTrade(swapId);
}


// Switch to editTrade Window
void Swap::viewTrade(QString swapId, QString stateCmd) {
    getSwap()->viewTrade(swapId, stateCmd);
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


    QString reportStr = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                        "<html><body style=\"font-family:'Open Sans'; font-size:medium; font-weight:normal; font-style:normal; color:white; background-color:transparent;\">";


    reportStr += QString("<p>") + (swap.isSeller ? "Selling" : "Buying") + " <b style=\"color:yellow;\">" + util::trimStrAsDouble(QString::number( swap.mwcAmount, 'f' ), 10) +
            " MWC</b> for <b style=\"color:yellow;\">" + util::trimStrAsDouble(QString::number( swap.secondaryAmount, 'f' ), 10) + " " + swap.secondaryCurrency + "</b>.</p>";

    reportStr += "<p>";
    reportStr += "Required confirmations: <b style=\"color:yellow;\">" + QString::number(swap.mwcConfirmations) +
                 " for MWC</b> and <b style=\"color:yellow;\">" + QString::number(swap.secondaryConfirmations) + " for " + swap.secondaryCurrency + "</b>.</p>";


    reportStr += "<p>Time limits: <b style=\"color:yellow;\">" + util::interval2String(swap.messageExchangeTimeLimit, false, 2) +
            "</b> for messages exchange and <b style=\"color:yellow;\">" +  util::interval2String(swap.redeemTimeLimit, false, 2)+"</b> for redeem or refund<p/>";

    reportStr += "<p>Locking order: " + (swap.sellerLockingFirst ? "<b style=\"color:yellow;\">Lock MWC</b> first" : ("<b style=\"color:yellow;\">Lock " + swap.secondaryCurrency + "</b> first")) + "</p>";

    QString lockTime = calcTimeLeft(swap.mwcLockTime);
    reportStr += "<p>";
    if (lockTime.size()>0)
        reportStr += "MWC funds locked until block <b style=\"color:yellow;\">" + QString::number(swap.mwcLockHeight) + "</b>, expected to be mined in <b style=\"color:yellow;\">" + lockTime + "</b>.<br/>";
    else
        reportStr += "MWC Lock expired.<br/>";

    QString secLockTime = calcTimeLeft(swap.secondaryLockTime);
    if (secLockTime.size()>0) {
        reportStr += swap.secondaryCurrency + " funds locked for <b style=\"color:yellow;\">" + secLockTime + "</b>.";
    }
    else {
        reportStr += swap.secondaryCurrency + " lock expired";
    }
    reportStr += "</p>";

    QString address = swap.communicationAddress;
    if (!address.contains("://")) {
        if (swap.communicationMethod == "mwcmqs")
            address = "mwcmqs://" + address;
        else if (swap.communicationMethod == "tor")
            address = "http://" + address + ".onion";
        else {
            Q_ASSERT(false); // QT wallet doesn't expect anything else.
        }
    }
    reportStr += "<p>Trading with : <b style=\"color:yellow;\">" +  address + "</b></p>";
    reportStr += "</body></html>";
    swapInfo.push_back(reportStr);

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
    // [8] - private ElectrumX uri
    swapInfo.push_back(swap.electrumNodeUri);

    emit sgnRequestTradeDetails( swapInfo, convertExecutionPlan(executionPlan), currentAction, convertTradeJournal(tradeJournal), errMsg );
}


// Check if this Trade is running in auto mode now
bool Swap::isRunning(QString swapId) {
    return getSwap()->isTradeRunning(swapId);
}

// Number of trades that are in progress
QVector<QString> Swap::getRunningTrades() {
    return getSwap()->getRunningTrades();
}

QVector<QString> Swap::getRunningCriticalTrades() {
    return getSwap()->getRunningCriticalTrades();
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

// Update electrumX private node URI
// Respond will come with sgnUpdateElectrumX
void Swap::updateElectrumX(QString swapId, QString electrumXnodeUri ) {
    getWallet()->adjustSwapData(swapId, "electrumx_uri", electrumXnodeUri );
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
    else if (adjustCmd=="electrumx_uri") {
        emit sgnUpdateElectrumX(swapId,errMsg);
    }
}

void Swap::onSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                              QString lastProcessError,
                              QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                              QVector<wallet::SwapJournalMessage> tradeJournal) {

    int64_t expirationTime = 0;
    for (const auto & ep : executionPlan) {
        if (ep.active) {
            expirationTime = ep.end_time;
            break;
        }
    }

    for (auto & tj : tradeJournal) {
        tj.message = mapMwc713Message(tj.message);
    }

    emit sgnSwapTradeStatusUpdated( swapId, stateCmd, currentAction, currentState, expirationTime, mapMwc713Message(lastProcessError),
            convertExecutionPlan(executionPlan),
            convertTradeJournal(tradeJournal));
}

void Swap::onNewSwapTrade(QString currency, QString swapId) {
    emit sgnNewSwapTrade(currency, swapId);
}

// Backup/export swap trade data into the file
// Respond with sgnBackupSwapTradeData
void Swap::backupSwapTradeData(QString swapId, QString backupFileName) {
    getWallet()->backupSwapTradeData(swapId, backupFileName);
}

// Import/restore swap trade data from the file
// Respond with sgnRestoreSwapTradeData
void Swap::restoreSwapTradeData(QString filename) {
    getWallet()->restoreSwapTradeData(filename);
}

void Swap::onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage) {
    emit sgnBackupSwapTradeData(swapId, exportedFileName, errorMessage);
}

void Swap::onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage) {
    emit sgnRestoreSwapTradeData(swapId, importedFilename, errorMessage);
}


// Switch to New Trade Window
void Swap::initiateNewTrade() {
    getSwap()->initiateNewTrade();
}

void Swap::showNewTrade1() {
    getSwap()->showNewTrade1();
}
void Swap::showNewTrade2() {
    getSwap()->showNewTrade2();
}

// Apply params from trade1 and switch to trade2 panel
// Response: sgnApplyNewTrade1Params(bool ok, QString errorMessage)
void Swap::applyNewTrade1Params(QString account, QString secCurrency, QString mwcAmount, QString secAmount,
                                      QString secAddress, QString sendToAddress, bool lockMwcFirst ) {

    QPair< bool, util::ADDRESS_TYPE > addressRes = util::verifyAddress(sendToAddress);
    if ( !addressRes.first ) {
        emit sgnApplyNewTrade1Params (false, "Unable to parse the other wallet address " + sendToAddress);
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
            emit sgnApplyNewTrade1Params(false, "Automated swaps working only with MQS and TOR addresses.");
            return;

        }
    }

    getSwap()->applyNewTrade11Params( lockMwcFirst );

    // We need to use wallet for verification. start new with a dry run will work great
    getWallet()->createNewSwapTrade(account,
                                    getAppContext()->getSendCoinsParams().inputConfirmationNumber,
                                    mwcAmount, secAmount, secCurrency,
                                    secAddress,
                                    1.0,
                                    true,
                                    60,
                                    60,
                                    60,
                                    6,
                                    addrType,
                                    sendToAddress,
                                    "",
                                    "",
                                    true,
                                    "applyNewTrade1Params",
                                    {account, secCurrency, mwcAmount, secAmount, secAddress, sendToAddress} );


    // Respond at onCreateNewSwapTrade, tag applyNewTrade1Params
}

// Apply params from trade2 panel and switch to the review (panel3)
// Response: sgnApplyNewTrade2Params(bool ok, QString errorMessage)
void Swap::applyNewTrade2Params(QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks,
                                      double secTxFee, QString electrumXUrl) {

    QString sendToAddress = getSwap()->getBuyerAddress();
    QPair< bool, util::ADDRESS_TYPE > addressRes = util::verifyAddress(sendToAddress);
    Q_ASSERT(addressRes.first);

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
            Q_ASSERT(false);
            return;
        }
    }


    // We need to use wallet for verification. start new with a dry run will work great
    getWallet()->createNewSwapTrade( getSwap()->getAccount(),
                                    getAppContext()->getSendCoinsParams().inputConfirmationNumber,
                                    getSwap()->getMwc2Trade(), getSwap()->getSec2Trade(), getSwap()->getCurrentSecCurrency(),
                                    getSwap()->getSecAddress(),
                                     secTxFee,
                                    true,
                                    offerExpTime,
                                    redeemTime,
                                    mwcBlocks,
                                    secBlocks,
                                    addrType,
                                    sendToAddress,
                                    electrumXUrl,
                                    "",
                                    true,
                                    "applyNewTrade2Params",
                                    {electrumXUrl} );

    getSwap()->applyNewTrade21Params(secCurrency, offerExpTime, redeemTime, mwcBlocks, secBlocks,
            secTxFee);

    // Respond at onCreateNewSwapTrade, tag applyNewTrade2Params
}

void Swap::onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg) {
    Q_UNUSED(swapId)
    if (tag == "applyNewTrade1Params") {
        Q_ASSERT(dryRun);
        Q_ASSERT(params.size()==6);
        emit sgnApplyNewTrade1Params(errMsg.isEmpty(), errMsg);
        if (errMsg.isEmpty()) {
            getSwap()->applyNewTrade12Params( params[0], params[1], params[2], params[3], params[4], params[5] );
        }
        return;
    }
    else if (tag == "applyNewTrade2Params") {
        Q_ASSERT(dryRun);
        Q_ASSERT(params.size()==1);
        emit sgnApplyNewTrade2Params(errMsg.isEmpty(), errMsg);
        if (errMsg.isEmpty()) {
            getSwap()->applyNewTrade22Params( params[0] );
        }
        return;
    }
    // Just a message from different request.
}


// Create and start a new swap. The data must be submitted by that moment
// Response: sgnCreateStartSwap(bool ok, QString errorMessage)
void Swap::createStartSwap() {
    getSwap()->createStartSwap();
}

void Swap::onCreateStartSwap(bool ok, QString errorMessage) {
    emit sgnCreateStartSwap(ok, errorMessage);
}

// Account that is related to this swap trade
QString Swap::getAccount() {
    return getSwap()->getAccount();
}
void Swap::setAccount(QString accountName) {
    getSwap()->setAccount(accountName);
}

// List of the secondary currencies that wallet support
QVector<QString> Swap::secondaryCurrencyList() {
    return getSwap()->secondaryCurrencyList();
}

QString Swap::getCurrentSecCurrency() {
    return getSwap()->getCurrentSecCurrency();
}

QString Swap::getCurrentSecCurrencyFeeUnits() {
    return getSwap()->getCurrentSecCurrencyFeeUnits();
}

void Swap::setCurrentSecCurrency(QString secCurrency) {
    getSwap()->setCurrentSecCurrency(secCurrency);
}

QString Swap::getMwc2Trade() {
    return getSwap()->getMwc2Trade();
}

QString Swap::getSec2Trade() {
    return getSwap()->getSec2Trade();
}

QString Swap::getSecAddress() {
    return getSwap()->getSecAddress();
}

bool    Swap::isLockMwcFirst() {
    return getSwap()->isLockMwcFirst();
}

QString Swap::getBuyerAddress() {
    return getSwap()->getBuyerAddress();
}

// Return pairs of the expiration interval combo:
// <Interval is string> <Value in minutes>
QVector<QString> Swap::getExpirationIntervals() {
    return getSwap()->getExpirationIntervals();
}

int Swap::getOfferExpirationInterval() {
    return getSwap()->getOfferExpirationInterval();
}
int Swap::getSecRedeemTime() {
    return getSwap()->getSecRedeemTime();
}
double Swap::getSecTransactionFee() {
    return getSwap()->getSecTransactionFee();
}

int Swap::getMwcConfNumber() {
    return getSwap()->getMwcConfNumber();
}

int Swap::getSecConfNumber() {
    return getSwap()->getSecConfNumber();
}

QString Swap::getElectrumXprivateUrl() {
    return getSwap()->getElectrumXprivateUrl();
}

// Calculate the locking time for a NEW not yet created swap offer.
QVector<QString> Swap::getLockTime( QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks ) {
    return getSwap()->getLockTime( secCurrency, offerExpTime, redeemTime, mwcBlocks, secBlocks );
}

bool isSwapDone(const QString & stateCmd) {
    return stateCmd.endsWith("Cancelled") || stateCmd.endsWith("Refunded")  || stateCmd.endsWith("Complete");
}
bool Swap::isSwapDone(QString stateCmd) {
    return bridge::isSwapDone(stateCmd);
}


static QSet<QString> nonCancelableStates{ "SellerWaitingForBuyerToRedeemMwc", "SellerRedeemSecondaryCurrency", "SellerWaitingForRedeemConfirmations","SellerSwapComplete",
                                                "SellerWaitingForRefundHeight", "SellerPostingRefundSlate", "SellerWaitingForRefundConfirmations", "SellerCancelledRefunded", "SellerCancelled",
                                          "BuyerRedeemMwc", "BuyerWaitForRedeemMwcConfirmations", "BuyerSwapComplete",
                                                "BuyerWaitingForRefundTime", "BuyerPostingRefundForSecondary", "BuyerWaitingForRefundConfirmations", "BuyerCancelledRefunded", "BuyerCancelled" };

bool isSwapCancellable(const QString & stateCmd) {
    return !nonCancelableStates.contains(stateCmd);
}
bool Swap::isSwapCancellable(QString stateCmd) {
    return bridge::isSwapCancellable(stateCmd);
}

static QMap<QString, int> buildBackupMapping() {
    QMap<QString, int> backups;
    backups.insert("SellerOfferCreated", 0);
    backups.insert("SellerSendingOffer", 0);
    backups.insert("SellerWaitingForAcceptanceMessage",0);
    backups.insert("SellerWaitingForBuyerLock",1);
    backups.insert("SellerPostingLockMwcSlate",1);
    backups.insert("SellerWaitingForLockConfirmations",1);
    backups.insert("SellerWaitingForInitRedeemMessage",1);
    backups.insert("SellerSendingInitRedeemMessage",2);
    backups.insert("SellerWaitingForBuyerToRedeemMwc",2);
    backups.insert("SellerRedeemSecondaryCurrency",2);
    backups.insert("SellerWaitingForRedeemConfirmations",2);
    backups.insert("SellerSwapComplete",0);
    backups.insert("SellerWaitingForRefundHeight",1);
    backups.insert("SellerPostingRefundSlate",1);
    backups.insert("SellerWaitingForRefundConfirmations",1);
    backups.insert("SellerCancelledRefunded",0);
    backups.insert("SellerCancelled",0);

    backups.insert("BuyerOfferCreated",0);
    backups.insert("BuyerSendingAcceptOfferMessage",1);
    backups.insert("BuyerWaitingForSellerToLock",1);
    backups.insert("BuyerPostingSecondaryToMultisigAccount",1);
    backups.insert("BuyerWaitingForLockConfirmations",1);
    backups.insert("BuyerSendingInitRedeemMessage",1);
    backups.insert("BuyerWaitingForRespondRedeemMessage",1);
    backups.insert("BuyerRedeemMwc",2);
    backups.insert("BuyerWaitForRedeemMwcConfirmations",2);
    backups.insert("BuyerSwapComplete",0);

    backups.insert("BuyerWaitingForRefundTime",1);
    backups.insert("BuyerPostingRefundForSecondary",1);
    backups.insert("BuyerWaitingForRefundConfirmations",1);
    backups.insert("BuyerCancelledRefunded",0);
    backups.insert("BuyerCancelled", 0);
    return backups;
}

static QMap<QString, int> backupMapping = buildBackupMapping();

int  getSwapBackup(const QString & stateCmd) {
    if (stateCmd.isEmpty())
        return 0;
    Q_ASSERT(backupMapping.contains(stateCmd));
    return backupMapping.value(stateCmd, 0);
}

int Swap::getSwapBackup(QString stateCmd) {
    return bridge::getSwapBackup(stateCmd);
}

bool isSwapWatingToAccept(const QString & stateCmd) {
    return stateCmd=="BuyerOfferCreated" || stateCmd=="BuyerSendingAcceptOfferMessage";
}

bool Swap::isSwapWatingToAccept(QString stateCmd) {
    return bridge::isSwapWatingToAccept(stateCmd);
}

// Accept a new trade and start run it. By that moment the trade must abe reviews and all set
void Swap::acceptTheTrade(QString swapId) {
    getSwap()->acceptTheTrade(swapId);
}

QString Swap::getSecondaryFee(QString secCurrency) {
    double fee = getSwap()->getSecondaryFee(secCurrency);
    if (fee<=0.0)
        return "";

    return util::trimStrAsDouble(QString::number( fee, 'f' ), 10);
}

// Tweak the swap tarde state. ONLY for dev help usage.
void Swap::adjustTradeState(QString swapId, QString newState) {
    getWallet()->adjustTradeState(swapId, newState);
}


}


