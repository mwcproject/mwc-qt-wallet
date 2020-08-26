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

    // Switch to New Trade Window
    Q_INVOKABLE void startNewTrade();

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

    // Get the list of secondary currencies that we are supporting
    Q_INVOKABLE QVector<QString> getSecondaryCurrencyList();

    // Calculate recommended confirmations number for MWC.
    Q_INVOKABLE int calcConfirmationsForMwcAmount(double mwcAmount);

    // Get a minimum/maximum possible number of confiormations for the secondary currency.
    Q_INVOKABLE QVector<int> getConfirmationLimitForSecondary(QString secondaryName);

    // Create a new trade. Response will be send back with a signal
    Q_INVOKABLE void createNewTrade( double mwc, double btc, QString secondary,
                                     QString redeemAddress,
                                     bool sellerLockFirst,
                                     int messageExchangeTimeMinutes,
                                     int redeemTimeMinutes,
                                     int mwcConfirmationNumber,
                                     int secondaryConfirmationNumber,
                                     QString communicationMethod,
                                     QString communicationAddress);

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

    // Start swap processing
    Q_INVOKABLE void startAutoSwapTrade(QString swapId);

    // Stop swap processing
    Q_INVOKABLE void stopAutoSwapTrade(QString swapId);
signals:
    // Result of deleteSwapTrade call.
    void sgnDeleteSwapTrade(QString swapId, QString error);

    // Result of cancelTrade. Ok - empty error.
    void sgnCancelTrade(QString swapId, QString error);

    // Result comes in series of 6 item tuples:
    // < <Info>, <Trade Id>, <State>, <Status>, <Date>, <secondary_address> >, ....
    void sgnSwapTradesResult( QVector<QString> trades );

    // Response from createNewSwapTrade, SwapId on OK,  errMsg on failure
    void sgnCreateNewTradeResult( QString swapId, QString errMsg);

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

private slots:
    void onRequestSwapTrades(QVector<wallet::SwapInfo> swapTrades);
    void onDeleteSwapTrade(QString swapId, QString errMsg);
    void onCreateNewSwapTrade(QString swapId, QString errMsg);
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

};

}

#endif //MWC_QT_WALLET_SWAP_H
