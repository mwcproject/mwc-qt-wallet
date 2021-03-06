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

#ifndef MWC_QT_WALLET_SWAPMKT_B_H
#define MWC_QT_WALLET_SWAPMKT_B_H


#include <QObject>
#include "../../wallet/wallet.h"

namespace bridge {

class SwapMarketplace : public QObject {
Q_OBJECT
public:
    explicit SwapMarketplace(QObject *parent);
    ~SwapMarketplace();

    // Return error message. Empty String on OK
    // offerId - empty string for a new offer, Otherwise expected that it is an update
    Q_INVOKABLE QString createNewOffer( QString offerId, QString  account,
                            bool sell, double  mwcAmount, double  secAmount,
                            QString secondaryCurrency,  int mwcLockBlocks, int secLockBlocks,
                            QString secAddress, double  secFee, QString note );

    // triggers sgnMyOffersChanged  event
    Q_INVOKABLE void withdrawMyOffer(QString offerId);

    // Json string with MySwap Offers info
    Q_INVOKABLE QVector<QString> getMyOffers();

    // JSon strings with MktSwapOffer. Requesting marketplace offers with filtering
    // selling: 0 - buy, 1-sell, 2 - all
    // currency: empty value for all
    Q_INVOKABLE QVector<QString> getMarketOffers(double minFeeLevel, int selling, QString currency);

    // return JSon strings with MktSwapOffer value. Empty string if such offer is not found
    Q_INVOKABLE QString getMarketOffer(QString offerId, QString walletAddress);

    // pair of Currency, number of offers
    Q_INVOKABLE QVector<QString> getTotalOffers();

    // Response at: sgnRequestIntegrityFees(QString error, int64_t balance, QVector<QString> IntegrityFeesJsonStr);
    Q_INVOKABLE void requestIntegrityFees();

    Q_INVOKABLE double getFeeLevel();
    Q_INVOKABLE QString getFeeDepositAccount();
    Q_INVOKABLE double getFeeReservedAmount();

    Q_INVOKABLE void setIntegritySettings(const double & feeLevel, QString feeDepositAccount, const double & feeReservedAmount);

    // Response at: sgnWithdrawIntegrityFees(QString error, double mwc, QString account);
    Q_INVOKABLE void withdrawIntegrityFees();

    // Return the listening status
    Q_INVOKABLE QString getOffersListeningStatus();

    // Switch to create a new offer page. For new offer myMsgId must be empty string.
    // Otherwise - exist offer id
    Q_INVOKABLE void pageCreateNewOffer(QString myMsgId);
    // Show marketplace page
    Q_INVOKABLE void pageMktList();
    // Show my offers at marketplace page
    Q_INVOKABLE void pageMktMyOffers();
    // Show fee page
    Q_INVOKABLE void pageFee();
    // Show integrity fee  transactions
    Q_INVOKABLE void pageTransactionFee();

    // Refresh mkt place offers. Response with sgnMarketPlaceOffersChanged.
    Q_INVOKABLE void requestMktSwapOffers();

    // Accept the offer from marketplace
    Q_INVOKABLE bool acceptMarketplaceOffer(QString offerId, QString walletAddress);

    // Request number of running MyOffers
    Q_INVOKABLE int getMyOffersNum();

    Q_INVOKABLE void stashMyOffers();

    // Request last known node height. 0 mean it is offline
    Q_INVOKABLE int getLastNodeHeight();
private:
signals:
    void sgnRequestIntegrityFees(QString error, int64_t balance, QVector<QString> IntegrityFeesJsonStr);
    void sgnWithdrawIntegrityFees(QString error, double mwc, QString account);

    void sgnMarketPlaceOffersChanged();
    void sgnMyOffersChanged();

    void sgnMessagingStatusChanged();

private slots:
    void onRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees);
    void onWithdrawIntegrityFees(QString error, double mwc, QString account);

    void onMarketPlaceOffersChanged();
    void onMyOffersChanged();
    void onMessagingStatusChanged();
};

}

#endif //MWC_QT_WALLET_SWAPMKT_B_H
