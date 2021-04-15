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

#ifndef MWC_QT_WALLET_SWAP_S_H
#define MWC_QT_WALLET_SWAP_S_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QMap>
#include <QHash>
#include "../util/httpclient.h"
#include <QThread>
#include "s_mktswap.h"

namespace core {
class TimerThread;
}

namespace state {

struct AutoswapTask {
    QString swapId;
    QString tag;
    bool    isSeller;
    QString stateCmd;
    int64_t lastUpdatedTime = 0;

    void setData(QString _swapId, QString _tag, bool _isSeller, QString _stateCmd, int64_t _lastUpdatedTime) { swapId=_swapId; tag=_tag; isSeller=_isSeller; stateCmd=_stateCmd; lastUpdatedTime=_lastUpdatedTime; }
};

enum class SwapWnd {None, PageSwapList, PageSwapEdit, PageSwapTradeDetails, PageSwapNew1, PageSwapNew2, PageSwapNew3 };

class Swap : public util::HttpClient, public State {
Q_OBJECT
public:
    Swap(StateContext * context);
    virtual ~Swap() override;

    // Show first page with trade List
    void pageTradeList(bool selectIncoming, bool selectOutgoing, bool selectBackup);
    // Edit/View Trade Page
    void viewTrade(QString swapId, QString stateCmd);
    // Show trade details page
    void showTradeDetails(QString swapId);

    // Check if Trade is running
    bool isTradeRunning(const QString & swapId) const;

    // Number of trades that are in progress
    QVector<QString> getRunningTrades() const;
    QVector<QString> getRunningCriticalTrades() const;

    // Reset the new trade data and switch to the first page.
    void initiateNewTrade();
    // Show the trade page 1
    void showNewTrade1();
    // Show the trade page 2
    void showNewTrade2();

    // Apply params from trade1, not need to check
    void applyNewTrade11Params(bool mwcLockFirst);
    // Apply params from trade1 and switch to trade2 panel. Expected that params are validated by the wallet(bridge)
    void applyNewTrade12Params(QString acccount, QString secCurrency, QString mwcAmount, QString secAmount,
                                          QString secAddress, QString sendToAddress );
    // Apply part1 params from trade2 panel. Expected that params are validated by the windows
    void applyNewTrade21Params(QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks,
                                          double secTxFee);
    // Apply [part2 params from trade2 panel and switch to the review (panel3).
    // Expected that params are validated by the wallet(bridge)
    void applyNewTrade22Params(QString electrumXUrl);


    // Create and start a new swap. The data must be submitted by that moment
    // Response: onCreateStartSwap(bool ok, QString errorMessage)
    void createStartSwap();

    // Account that is related to this swap trade
    QString getAccount() const {return  newSwapAccount;}
    void setAccount(QString accountName) {newSwapAccount = accountName;}

    // List of the secondary currencies that wallet support
    QVector<QString> secondaryCurrencyList();

    // Check if this trade is created from accepted Marketplace offer
    bool isMktTrade() const {return !mktOfferId.isEmpty();}

    // Current selected currency to trade
    QString getCurrentSecCurrency() const {return newSwapCurrency;}
    void setCurrentSecCurrency(QString secCurrency);

    QString getCurrentSecCurrencyFeeUnits() const;

    // Current trade parameters.
    QString getMwc2Trade() const {return newSwapMwc2Trade;}
    QString getSec2Trade() const {return newSwapSec2Trade;}
    QString getSecAddress() const {return newSwapSecAddress;}
    bool isLockMwcFirst() const {return newSwapSellectLockFirst;}
    QString getBuyerAddress() const {return newSwapBuyerAddress;}

    // Get minimal Amount for the secondary currency
    double getSecMinAmount(QString secCurrency) const;

    // Return pairs of the expiration interval combo:
    // <Interval is string> <Value in minutes>
    QVector<QString> getExpirationIntervals() const;

    int getOfferExpirationInterval() const {return newSwapOfferExpirationTime;}
    int getSecRedeemTime() const {return newSwapRedeemTime;}
    double getSecTransactionFee() const {return newSwapSecTxFee;}
    double getSecMinTransactionFee() const {return newSwapMinSecTxFee;}
    double getSecMaxTransactionFee() const {return newSwapMaxSecTxFee;}
    int getMwcConfNumber() const {return newSwapMwcConfNumber;}
    int getSecConfNumber() const {return newSwapSecConfNumber;}
    QString getElectrumXprivateUrl() const {return newSwapElectrumXUrl;}

    double getSecTransactionFee(const QString & secCurrency) const;
    double getSecMinTransactionFee(const QString & secCurrency) const;
    double getSecMaxTransactionFee(const QString & secCurrency) const;

    int getMwcConfNumber(double mwcAmount) const;
    int getSecConfNumber(const QString & secCurrency) const;

    // Calculate the locking time for a NEW not yet created swap offer.
    static QVector<QString> getLockTime( QString secCurrency, int offerExpTime, int redeemTime, int mwcBlocks, int secBlocks );

    // Accept a new trade and start run it. By that moment the trade must abe reviews and all set
    void acceptTheTrade(QString swapId);

    // Get Tx fee for secondary currency
    double getSecondaryFee(QString secCurrency);

    QString getNote() const {return newSwapNote;}
    void setNote(QString note) {newSwapNote = note;}

    // Notify about failed bidding. If it is true, we need to cancel and show the message about that
    // Note, mwc-wallet does cancellation as well.
    void failBidding(QString wallet_tor_address, QString offer_id);

    // Start trading for my offer - automatic operation. For Sell the offer will be created.
    // For Buy - offer will be accepted automatically.
    void startTrading(const MySwapOffer & offer, QString wallet_tor_address);

    // Start trading for Marketplace offer. It will start for sell only. For Buy the normal workflow should be fine, we will only press "Accept" automatically.
    // For Buy - offer will be accepted automatically.
    void acceptOffer(const MktSwapOffer & offer, QString wallet_tor_address, int running_num);

    // Reject any offers from this address. We don't want them, we are likely too late
    void rejectOffer(const MktSwapOffer & offer, QString wallet_tor_address);

    // check if swap with this tag is exist
    bool isSwapExist(QString tag) const;

    // Verify if trade backup dir is valid. If not, then ask user to fix that
    bool verifyBackupDir();

private:
signals:

    void onSwapTradeStatusUpdated(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                               QString lastProcessError,
                               QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                               QVector<wallet::SwapJournalMessage> tradeJournal);

    void onCreateStartSwap(bool ok, QString errorMessage);

protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override;

    virtual void onProcessHttpResponse(bool requestOk, const QString & tag, QJsonObject & jsonRespond,
                                       const QString & param1,
                                       const QString & param2,
                                       const QString & param3,
                                       const QString & param4) override;

    virtual QString getHelpDocName() override {return "swap.html";}

private:
    void resetNewSwapData();

    // Start trade to run
    void runTrade(QString swapId, QString tag, bool isSeller, QString statusCmd);

    int calcConfirmationsForMwcAmount(double mwcAmount) const;

    // Request latest fees for the coins
    void updateFeesIsNeeded();

    void runSwapIfNeed(const wallet::SwapInfo & sw);

private
slots:
    // Login/logot from the wallet. Need to start/stop swaps
    void onLoginResult(bool ok);
    void onLogout();

    // Response from requestSwapTrades
    // Also we are watching to accasional requests to wallet. The state can be changed, so the 'running '
    void onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error);

    void onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                               QString lastProcessError,
                               QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                               QVector<wallet::SwapJournalMessage> tradeJournal,
                               QString error );

    void onNewSwapTrade(QString currency, QString swapId);
    void onNewSwapMessage(QString swapId);

    // Response from createNewSwapTrade, SwapId on OK,  errMsg on failure
    void onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg);

    // Wallet just cancelled the swap. We need to stop execute it.
    void onCancelSwapTrade(QString swapId, QString error);
    // Just restore the swap. We need to run it.
    void onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    void onTimerEvent();

    void onRequestTradeDetails( wallet::SwapTradeInfo swap,
                                      QVector<wallet::SwapExecutionPlanRecord> executionPlan,
                                      QString currentAction,
                                      QVector<wallet::SwapJournalMessage> tradeJournal,
                                      QString error,
                                      QString cookie );

    void onAdjustSwapData(QString swapId, QString call_tag, QString errMsg);

    void onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage);

    void onSendMarketplaceMessage(QString error, QString response, QString offerId, QString walletAddress, QString cookie);
private:
    core::TimerThread * timer = nullptr;

    // Key: swapId,  Value: running Task
    QMap<QString, AutoswapTask> runningSwaps;
    QString  runningTask;

    // key: message.  value: time
    QHash<QString, int64_t> shownMessages;

    long lastProcessedTimerData = 0;

    // New trade data.
    QString mktOfferId; // true if it is marketplace. In this case most of those values we can't change
    int     mktRunningNum = 0; // Number of running offers when we started and that we were agree on
    QString newSwapAccount;
    QString newSwapCurrency;
    QString newSwapMwc2Trade;
    QString newSwapSec2Trade;
    QString newSwapSecAddress;
    bool    newSwapSellectLockFirst = true;
    QString newSwapBuyerAddress;
    int     newSwapOfferExpirationTime = -1;
    int     newSwapRedeemTime = -1;
    double  newSwapSecTxFee = 0.0;
    double  newSwapMinSecTxFee = 0.0;
    double  newSwapMaxSecTxFee = 0.0;
    int     newSwapMwcConfNumber = -1;
    int     newSwapSecConfNumber = -1;
    QString newSwapElectrumXUrl;
    QString newSwapNote;

    QString newSwapCurrency2recalc;
    SwapWnd selectedPage = SwapWnd::None;

    QHash< QString, QVector<MktSwapOffer> > acceptedOffers;
    QHash< QString, QVector<MySwapOffer> >  expectedOffers;
};

}

#endif //MWC_QT_WALLET_SWAP_H
