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

#ifndef MWC_QT_WALLET_SWAP_H
#define MWC_QT_WALLET_SWAP_H

#include <QObject>
#include "../../wallet/wallet.h"

namespace bridge {

class Swap : public QObject {
Q_OBJECT
public:
    explicit Swap(QObject *parent);
    ~Swap();

    // Return back to the trade list page
    Q_INVOKABLE void pageTradeList();

    // request the list of swap trades
    // Response send back with a signal:   sgnSwapTradesResult( QString cookie, QVector<QString> trades, QString error );
    Q_INVOKABLE void requestSwapTrades(QString cookie);

    // Cancel the trade. Send signal to the cancel the trade.
    // Trade cancelling might take a while.
    // Response send back with a signal  sgnCancelTrade
    Q_INVOKABLE void cancelTrade(QString swapId);

    // Switch to editTrade Window
    Q_INVOKABLE void viewTrade(QString swapId, QString stateCmd);

    // Switch to trade Details Window
    Q_INVOKABLE void showTradeDetails(QString swapId);

    // Deleting swapId. Response will be send back at sgnDeleteSwapTrade
    Q_INVOKABLE void deleteSwapTrade(QString swapId);

    // Requesting all details about the single swap Trade
    // Respond will be with sent back with sgnRequestTradeDetails
    Q_INVOKABLE void requestTradeDetails(QString swapId, QString cookie);

    // Requesting Eth Info
    // Respond will be with sent back with sgnRequestEthInfo
    Q_INVOKABLE void requestEthInfo();

    // Requesting Eth Send
    // Respond will be with sent back with sgnRequestEthSend
    Q_INVOKABLE void requestEthSend(QString dest, QString currency, QString amount);

    // Check if this Trade is running in auto mode now
    Q_INVOKABLE bool isRunning(QString swapId);

    // trade Ids that are in progress
    Q_INVOKABLE QVector<QString> getRunningTrades();

    // Critical Trade Ids that are in progress
    Q_INVOKABLE QVector<QString> getRunningCriticalTrades();

    // Update communication method.
    // Respond will be at sgnAdjustSwapTrade(QString swapId, QString call_tag, QString errMsg)
    Q_INVOKABLE void adjustSwapData( QString swapId, QString call_tag,
            QString destinationMethod, QString destinationDest,
            QString secondaryAddress,
            QString secondaryFee,
            QString electrumUri1,
            QString tag );

    // Backup/export swap trade data into the file
    // Respond with sgnBackupSwapTradeData
    Q_INVOKABLE void backupSwapTradeData(QString swapId, QString backupFileName);

    // Import/restore swap trade data from the file
    // Respond with sgnRestoreSwapTradeData
    Q_INVOKABLE void restoreSwapTradeData(QString filename);


    // Initiate a new trade. So prepare the data and switch to the first new trade panel
    Q_INVOKABLE void initiateNewTrade();

    // Switch to New Trade Windows
    Q_INVOKABLE void showNewTrade1();
    Q_INVOKABLE void showNewTrade2();
    // Apply params from trade1 and switch to trade2 panel
    // Response: sgnApplyNewTrade1Params(bool ok, QString errorMessage)
    Q_INVOKABLE void applyNewTrade1Params(QString account, QString secCurrency, QString mwcAmount, QString secAmount,
                                          QString secAddress, QString sendToAddress, bool lockMwcFirst );
    // Apply params from trade2 panel and switch to the review (panel3)
    // Response: sgnApplyNewTrade2Params(bool ok, QString errorMessage)
    Q_INVOKABLE void applyNewTrade2Params(QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks,
                                          double secTxFee, QString electrumXUrl);

    // Create and start a new swap. The data must be submitted by that moment
    // Response: sgnCreateStartSwap(bool ok, QString errorMessage)
    Q_INVOKABLE void createStartSwap();


    // Account that is related to this swap trade
    Q_INVOKABLE QString getAccount();
    Q_INVOKABLE void setAccount(QString accountName);

    // List of the secondary currencies that wallet support
    Q_INVOKABLE QVector<QString> secondaryCurrencyList();

    // Check if this trade is created from accepted Marketplace offer
    Q_INVOKABLE bool isMktTrade();

    // Current selected currency to trade
    Q_INVOKABLE QString getCurrentSecCurrency();
    Q_INVOKABLE QString getCurrentSecCurrencyFeeUnits();
    Q_INVOKABLE void setCurrentSecCurrency(QString secCurrency);

    // Current trade parameters.
    Q_INVOKABLE QString getMwc2Trade();
    Q_INVOKABLE QString getSec2Trade();
    Q_INVOKABLE QString getSecAddress();
    Q_INVOKABLE bool isLockMwcFirst();
    Q_INVOKABLE QString getBuyerAddress();
    Q_INVOKABLE double getSecMinAmount(QString secCurrency);

    // Return pairs of the expiration interval combo:
    // <Interval is string> <Value in minutes>
    Q_INVOKABLE QVector<QString> getExpirationIntervals();

    Q_INVOKABLE int getOfferExpirationInterval();
    Q_INVOKABLE int getSecRedeemTime();
    Q_INVOKABLE double getSecTransactionFee();
    Q_INVOKABLE double getSecMinTransactionFee();
    Q_INVOKABLE double getSecMaxTransactionFee();
    Q_INVOKABLE double getSecTransactionFee(QString secCurrency);
    Q_INVOKABLE double getSecMinTransactionFee(QString secCurrency);
    Q_INVOKABLE double getSecMaxTransactionFee(QString secCurrency);
    Q_INVOKABLE int getMwcConfNumber();
    Q_INVOKABLE int getSecConfNumber();
    Q_INVOKABLE QString getElectrumXprivateUrl();

    Q_INVOKABLE int getMwcConfNumber(double mwcAmount);
    Q_INVOKABLE int getSecConfNumber(QString secCurrency);

    Q_INVOKABLE QString getNote();
    Q_INVOKABLE void setNote(QString note);

    // Calculate the locking time for a NEW not yet created swap offer.
    Q_INVOKABLE QVector<QString> getLockTime( QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks );

    ////////////////////////////////////////////////////////
    // Bunch of swap utils. We want to mainatin swap Cmd mapping in a single place
    // See 'Methods for core callers' below
    Q_INVOKABLE bool isSwapDone(QString stateCmd);
    Q_INVOKABLE bool isSwapCancellable(QString stateCmd, bool marketplaceTrade);
    Q_INVOKABLE int  getSwapBackup(QString stateCmd);
    Q_INVOKABLE bool isSwapWatingToAccept(QString stateCmd);

    // Accept a new trade and start run it. By that moment the trade must abe reviews and all set
    Q_INVOKABLE void acceptTheTrade(QString swapId);

    // Request secondary fee for the currency
    Q_INVOKABLE QString getSecondaryFee(QString secCurrency);

    // Tweak the swap trade state. ONLY for dev help usage.
    Q_INVOKABLE void adjustTradeState(QString swapId, QString newState);

    // Verify swap trades backup dir. In case of error, it shows the message.
    // Return true if current backup dir is valid. Otherwise - false
    Q_INVOKABLE bool verifyBackupDir(QString backupDir, bool showErrorMessage);
signals:
    // Result of deleteSwapTrade call.
    void sgnDeleteSwapTrade(QString swapId, QString error);

    // Result of cancelTrade. Ok - empty error.
    void sgnCancelTrade(QString swapId, QString error);

    // Result comes in series of 11 item tuples:
    // < <bool is Seller>, <mwcAmount>, <sec+amount>, <sec_currency>, <Trade Id>, <StateCmd>, <status>, <initiate_time>, <expire_time>  <secondary_address> <last_process_error> >, ....
    // error is empty on success
    void sgnSwapTradesResult( QString cookie, QVector<QString> trades, QString error );

    // Response from requestTradeDetails call
    // swapInfo data
    // [0] - swapId
    // [1] - Description in HTML format. Role can be calculated form here as "Selling ..." or "Buying ..."
    // [2] - Redeem address
    // [3] - secondary currency name
    // [4] - secondary fee
    // [5] - secondary fee units
    // [6] - communication method: mwcmqc|tor
    // [7] - Communication address
    // [8] - private ElectrumX uri
    // [9] - MWC amount
    // [10] - Secondary currency amount
    // ------ Error response
    // [0] - swapId
    // ---------------
    // executionPlan, array of triplets: <active: "true"|"false">, <end_time>, <Name> >, ....
    // currentAction - name of the current action
    // tradeJournal, array of duplets: < <message>, <time> >, ...
    void sgnRequestTradeDetails( QVector<QString> swapInfo,
                                 QVector<QString> executionPlan,
                                 QString currentAction,
                                 QVector<QString> tradeJournal,
                                 QString errMsg,
                                 QString cookie );

    // Response from requestEthInfo call
    // ethAddr, ethereum wallet address
    // currency, Ether or Erc20 token 
    // balance,
    void sgnRequestEthInfo(QString ethAddr, QString currency, QString balance);

    // Response from requestEthSend call
    // status, result status
    void sgnRequestEthSend(QString dest, QString currency, QString amount);

    // Response from sgnAdjustSwapTrade. OK - empty error message
    void sgnAdjustSwapTrade(QString swapId, QString call_tag, QString errMsg);

    // Notification about the update of swap trade. Normally it comes from the autoswap
    // executionPlan, array of triplets: <active: "true"|"false">, <end_time>, <Name> >, ....
    // tradeJournal, array of duplets: < <message>, <time> >, ...
    void sgnSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                                   int64_t expirationTime,
                                   QString lastProcessError,
                                   QVector<QString> executionPlan,
                                   QVector<QString> tradeJournal);

    // The wallet get a new trade. You don't need to show a message box about that. But you will need to take
    // needed action on the page level
    void sgnNewSwapTrade(QString currency, QString swapId);

    // Respond from backupSwapTradeData
    // On OK will get exportedFileName
    void sgnBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage);

    // Import/restore swap trade data from the file
    // Respond from restoreSwapTradeData
    void sgnRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    // Response from ApplyNewTrade1Params
    void sgnApplyNewTrade1Params(bool ok, QString errorMessage);
    // Response from ApplyNewTrade2Params
    void sgnApplyNewTrade2Params(bool ok, QString errorMessage);
    // Response from ApplyNewTrade2Params
    void sgnCreateStartSwap(bool ok, QString errorMessage);

private slots:
    void onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error);
    void onDeleteSwapTrade(QString swapId, QString errMsg);
    void onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg);
    void onCancelSwapTrade(QString swapId, QString error);
    void onRequestTradeDetails( wallet::SwapTradeInfo swap,
                                QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                                QString currentAction,
                                QVector<wallet::SwapJournalMessage> tradeJournal,
                                QString errMsg,
                                QString cookie );
    void onRequestEthInfo(QString ethAddr, QString currency, QString balance);
    void onRequestEthSend(QString dest, QString currency, QString amount);
    void onAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg);

    void onSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                                  QString lastProcessError,
                                  QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                                  QVector<wallet::SwapJournalMessage> tradeJournal);

    void onNewSwapTrade(QString currency, QString swapId);

    void onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage);
    void onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    void onCreateStartSwap(bool ok, QString errorMessage);
};

// Methods for core callers
bool isSwapDone(const QString & stateCmd);
bool isSwapCancellable(const QString & stateCmd, bool marketplaceTrade);
int  getSwapBackup(const QString & stateCmd);
bool isSwapWatingToAccept(const QString & stateCmd);


}

#endif //MWC_QT_WALLET_SWAP_H
