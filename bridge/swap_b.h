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
#include "../wallet/wallet.h"

namespace bridge {

class Swap : public QObject {
Q_OBJECT
public:
    explicit Swap(QObject *parent = nullptr);
    ~Swap();

    // Return back to the trade list page
    Q_INVOKABLE void pageTradeList();

    // request the list of swap trades
    Q_INVOKABLE void requestSwapTrades();

    // Cancel the trade. Send signal to the cancel the trade.
    // Trade cancelling might take a while.
    // Response send back with a signal  sgnCancelTrade
    Q_INVOKABLE void cancelTrade(QString swapId);

    // Switch to editTrade Window
    Q_INVOKABLE void viewTrade(QString swapId);

    // Switch to trade Details Window
    Q_INVOKABLE void showTradeDetails(QString swapId);

    // Deleting swapId. Response will be send back at sgnDeleteSwapTrade
    Q_INVOKABLE void deleteSwapTrade(QString swapId);

    // Requesting all details about the single swap Trade
    // Respond will be with sent back with sgnRequestTradeDetails
    Q_INVOKABLE void requestTradeDetails(QString swapId);

    // Check if this Trade is running in auto mode now
    Q_INVOKABLE bool isRunning(QString swapId);

    // Update communication method.
    // Respond will be at sgnUpdateCommunication
    Q_INVOKABLE void updateCommunication(QString swapId, QString communicationMethod, QString communicationAddress);

    // Update secondary refund/redeem address.
    // Respond will be at sgnUpdateSecondaryAddress
    Q_INVOKABLE void updateSecondaryAddress(QString swapId, QString secondaryAddress);

    // Update secondary fee value for the transaction.
    // Respond will come with sgnUpdateSecondaryFee
    Q_INVOKABLE void updateSecondaryFee(QString swapId, double fee);

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
                                          QString secAddress, QString sendToAddress );
    // Apply params from trade2 panel and switch to the review (panel3)
    // Response: sgnApplyNewTrade2Params(bool ok, QString errorMessage)
    Q_INVOKABLE void applyNewTrade2Params(QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks,
                                          double secTxFee, QString electrumXUrl);

    // Create and start a new swap. The data must be submitted by that moment
    // Response: sgnCreateStartSwap(bool ok, QString errorMessage)
    Q_INVOKABLE void createStartSwap();


    // Account that is related to this swap trade
    Q_INVOKABLE QString getAccount();

    // List of the secondary currencies that wallet support
    Q_INVOKABLE QVector<QString> secondaryCurrencyList();

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

    // Return pairs of the expiration interval combo:
    // <Interval is string> <Value in minutes>
    Q_INVOKABLE QVector<QString> getExpirationIntervals();

    Q_INVOKABLE int getOfferExpirationInterval();
    Q_INVOKABLE int getSecRedeemTime();
    Q_INVOKABLE double getSecTransactionFee();
    Q_INVOKABLE int getMwcConfNumber();
    Q_INVOKABLE int getSecConfNumber();
    Q_INVOKABLE QString getElectrumXprivateUrl();

    // Calculate the locking time for a NEW not yet created swap offer.
    Q_INVOKABLE QVector<QString> getLockTime( QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks );
signals:
    // Result of deleteSwapTrade call.
    void sgnDeleteSwapTrade(QString swapId, QString error);

    // Result of cancelTrade. Ok - empty error.
    void sgnCancelTrade(QString swapId, QString error);

    // Result comes in series of 9 item tuples:
    // < <bool is Seller>, <mwcAmount>, <sec+amount>, <sec_currency>, <Trade Id>, <State>, <initiate_time_interval>, <expire_time_interval>  <secondary_address> >, ....
    void sgnSwapTradesResult( QVector<QString> trades );

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
                                 QString errMsg );

    // Response from updateCommunication. OK - empty error message
    void sgnUpdateCommunication(QString swapId, QString errorMsg);
    // Response from updateSecondaryAddress
    void sgnUpdateSecondaryAddress(QString swapId, QString errorMsg);
    // Response from updateSecondaryFee
    void sgnUpdateSecondaryFee(QString swapId, QString errorMsg);

    // Notification about the update of swap trade. Normally it comes from the autoswap
    // executionPlan, array of triplets: <active: "true"|"false">, <end_time>, <Name> >, ....
    // tradeJournal, array of duplets: < <message>, <time> >, ...
    void sgnSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
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
    void onRequestSwapTrades(QVector<wallet::SwapInfo> swapTrades);
    void onDeleteSwapTrade(QString swapId, QString errMsg);
    void onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg);
    void onCancelSwapTrade(QString swapId, QString error);
    void onRequestTradeDetails( wallet::SwapTradeInfo swap,
                                QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                                QString currentAction,
                                QVector<wallet::SwapJournalMessage> tradeJournal,
                                QString errMsg );
    void onAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg);

    void onSwapTradeStatusUpdated(QString swapId, QString currentAction, QString currentState,
                                  QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                                  QVector<wallet::SwapJournalMessage> tradeJournal);

    void onNewSwapTrade(QString currency, QString swapId);

    void onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage);
    void onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    void onCreateStartSwap(bool ok, QString errorMessage);
};

}

#endif //MWC_QT_WALLET_SWAP_H
