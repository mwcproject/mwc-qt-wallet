// Copyright 2021 The MWC Developers
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

#ifndef MWC_QT_WALLET_S_MKTSWAP_H
#define MWC_QT_WALLET_S_MKTSWAP_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QJsonObject>
#include <QHash>
#include <QSet>

namespace core {
class TimerThread;
}

namespace state {

enum class SwapMarketplaceWnd {None, Marketplace, NewOffer, TransactionFee };

struct MktSwapOffer {
    QString id;
    bool sell = true;
    double  mwcAmount = 0.0;
    double  secAmount = 0.0;
    QString secondaryCurrency;
    int     mwcLockBlocks = 0;
    int     secLockBlocks = 0;

    double mktFee = 0.0;
    QString walletAddress;
    int64_t timestamp = 0;

    MktSwapOffer() = default;
    MktSwapOffer(const MktSwapOffer & itm) = default;
    MktSwapOffer & operator = (const MktSwapOffer & itm) = default;

    MktSwapOffer(const QString & _id,
            bool _sell,
            double  _mwcAmount,
            double  _secAmount,
            const QString & _secondaryCurrency,
            int     _mwcLockBlocks,
            int     _secLockBlocks) :
            id(_id),
            sell(_sell),
            mwcAmount(_mwcAmount),
            secAmount(_secAmount),
            secondaryCurrency(_secondaryCurrency),
            mwcLockBlocks(_mwcLockBlocks),
            secLockBlocks(_secLockBlocks)
    {
    }

    MktSwapOffer( const QString & jsonStr );
    MktSwapOffer( const QJsonObject & json );
    QJsonObject toJson() const;
    QString toJsonStr() const;

    bool isEmpty() const {return id.isEmpty();}
    bool equal( const wallet::SwapTradeInfo & swap ) const;

    double getFeeLevel() const;

    QString calcMwcLockTime() const;
    QString calcSecLockTime() const;

    bool isValid() const;

    QString getKey() const {return walletAddress + "_" + id;}

    QString calcRateAsStr() const;
    double  calcRate() const;
};

enum class OFFER_STATUS { PENDING=1, STARTING=2, RUNNING=3 };

struct MySwapOffer {
    QString msgUuid;
    MktSwapOffer offer;
    QString  account;
    QString secAddress;
    double  secFee = 0.0;
    QString note;
    OFFER_STATUS status;
    QStringList outputs; // Locked output. Need for verification and reevaluation.

    wallet::IntegrityFees integrityFee;

    MySwapOffer() = default;
    MySwapOffer(const MySwapOffer & itm) = default;
    MySwapOffer & operator = (const MySwapOffer & itm) = default;

    MySwapOffer(MktSwapOffer _offer,
                QString  _account,
                QString _secAddress,
                double  _secFee,
                QString _note,
                QStringList _outputs) :
        offer(_offer),
        account(_account),
        secAddress(_secAddress),
        secFee(_secFee),
        note(_note),
        outputs(_outputs)
    {
        msgUuid = ""; // uuid will be assigned when wallet create it
        status = OFFER_STATUS::PENDING;
    }
    // build form Json string
    MySwapOffer(const QString jsonStr);
    MySwapOffer(const QJsonObject & json);

    double calcFee(double feeLevel) const;

    QJsonObject toJson() const;
    QString toJsonStr() const;

    QString getStatusStr(int tipHeight) const;

    // Offer description for user. Sell XX MWC for XX BTC
    QString getOfferDescription() const;
};

class Swap;

class SwapMarketplace : public QObject, public State {
Q_OBJECT
public:
    explicit SwapMarketplace(StateContext *context);

    virtual ~SwapMarketplace() override;

    // Return error message. Empty String on OK
    // if offerId is empty - create a new offer. Otherwise - updating exist offer.
    QString createNewOffer( QString offerId, QString  account,
            bool sell, double  mwcAmount, double  secAmount,
            QString secondaryCurrency,  int mwcLockBlocks, int secLockBlocks,
            QString secAddress, double  secFee, QString note );

    // change response with onMyOffersChanged
    // return Offer description if such offer was found. Otherwise - empty string
    QString withdrawMyOffer(QString offerId);

    // Request a new Marketplace offers from the wallet.
    // Respond: onMarketPlaceOffersChanged
    void requestMktSwapOffers();

    QVector<MySwapOffer> getMyOffers() const {return myOffers;}
    // Request marketplace offers with filtering
    // selling: 0 - buy, 1-sell, 2 - all
    // currency: empty value for all
    QVector<MktSwapOffer> getMarketOffers(double minFeeLevel, int selling, QString currency);
    // All marketplace offers that are published buy currency. Sorted by largest number
    QVector<QPair<QString,int>> getTotalOffers();

    // Response at: onRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees);
    void requestIntegrityFees();

    double getFeeLevel() const;
    QString getFeeDepositAccount() const;
    double getFeeReservedAmount() const;

    void setIntegritySettings(const double & feeLevel, QString feeDepositAccount, const double & feeReservedAmount);

    // Response at:  onWithdrawIntegrityFees(QString error, double mwc, QString account);
    void withdrawIntegrityFees();

    // Start/stop listening for the offers
    // Return the listening progress
    QString getOffersListeningStatus() const;
    // start/stop offers publishing
    void setListeningForOffers(bool start);

    // Switch to create a new offer page. For new offer myMsgId must be empty string.
    // Otherwise - exist offer id
    void pageCreateUpdateOffer(QString myMsgId);

    // Switch to swap marketplace first page
    void pageMktList(bool selectMyOffers, bool selectFee);

    // Show integrity transaction fees
    void pageFeeTransactions();

    // Accept the offer from marketplace
    bool acceptMarketplaceOffer(QString offerId, QString walletAddress);

    int getMyOffersNum() const {return myOffers.size();}

    void stashMyOffers();

    int getLastNodeHeight() const {return lastNodeHeight;}
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override;

    void updateIntegrityFeesAndStart();
    //void startBroadcastMessages( QVector<wallet::IntegrityFees> fees );

    void cleanMarketOffers();

    // Restore my swap offers.
    void restoreMySwapTrades();

    // Creating and start the swap trade from this offer
    // Node, several offers can be run in parallel until one of them will reach the locking stage.
    // Then the rest will be cancelled.
    void createNewSwapTrade( MySwapOffer offer, QString wallet_tor_address);

    // Lock outputs for my offer. Return Error or result
    QPair<QString, QStringList> lockOutputsForSellOffer(const QString & account, double mwcAmount, QString offerId);

    QString getMyOfferStashFileName() const;
private:
signals:
    void onMarketPlaceOffersChanged();
    void onMyOffersChanged();
    void onMessagingStatusChanged();
    void onRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees);
    void onWithdrawIntegrityFees(QString error, double mwc, QString account);

private
slots:
    void respCreateIntegrityFee(QString err, QVector<wallet::IntegrityFees> result);
    void respRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees);
    void respWithdrawIntegrityFees(QString error, double mwc, QString account );
    void respRequestMessagingStatus(QString error, wallet::MessagingStatus status);
    void respMessagingPublish(QString id, QString uuid, QString error);
    void respCheckIntegrity(QString error, QVector<QString> expiredMsgUuid);

    void respReceiveMessages(QString error, QVector<wallet::ReceivedMessages> msgs);

    void onTimerEvent();

    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    void onListeningStartResults( bool mqTry, bool tor, // what we try to start
                                  QStringList errorMessages, bool initialStart ); // error messages, if get some

    void onStartListenOnTopic(QString error);
    void onStopListenOnTopic(QString error);

    // Notification that nee Swap trade offer was received.
    // messageId: S_MKT_ACCEPT_OFFER or S_MKT_FAIL_BIDDING
    // wallet_tor_address: address from what we get a message
    // offer_id: offer_i from the swap marketplace. It is expected to be known
    void onNewMktMessage(int messageId, QString wallet_tor_address, QString offer_id);

    // Offer is full filled
    void onMktGroupWinner(QString swapId, QString tag);

    // Response from sendMarketplaceMessage
    void onSendMarketplaceMessage(QString error, QString response, QString offerId, QString walletAddress, QString cookie);
private:
    core::TimerThread * timer = nullptr;
    Swap * swap = nullptr;

    // Key: <wallet>_<id>
    QHash<QString, MktSwapOffer> marketOffers;
    int64_t lastMarketOffersCleaning = 0;
    QVector<MySwapOffer>    myOffers;

    QSet<QString> acceptedOffers;

    // Integrity fees
    double integrityFeeLevel = 0.0001; // Fee in fraction of the trade. Min value still apply

    SwapMarketplaceWnd selectedPage = SwapMarketplaceWnd::None;
    int currentOfferId = 0;

    int64_t startMktListening = 0;
    int64_t lastMessagingStatusRequest = 0;
    wallet::MessagingStatus messagingStatus;
    // Available fees
    QVector<wallet::IntegrityFees> fees;

    int64_t lastCheckIntegrity = 0;
    bool marketplaceActivated = false;
    int lastNodeHeight = 0;
};

}



#endif //MWC_QT_WALLET_S_MKTSWAP_H
