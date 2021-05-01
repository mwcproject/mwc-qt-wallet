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

#include "swapmkt_b.h"
#include "../../wallet/wallet.h"
#include "../../state/state.h"
#include "../../state/s_mktswap.h"
#include "../../state/u_nodeinfo.h"
#include "../../core/appcontext.h"
#include "../../util/address.h"
#include "../../core/MessageMapper.h"

namespace bridge {

//static wallet::Wallet * getWallet() { return state::getStateContext()->wallet; }
//static core::AppContext * getAppContext() { return state::getStateContext()->appContext; }
static state::SwapMarketplace * getSwapMkt() {return (state::SwapMarketplace *) state::getState(state::STATE::SWAP_MKT); }

SwapMarketplace::SwapMarketplace(QObject *parent) : QObject(parent) {
    state::SwapMarketplace * swapMkt = getSwapMkt();

    QObject::connect(swapMkt, &state::SwapMarketplace::onRequestIntegrityFees,
                     this, &SwapMarketplace::onRequestIntegrityFees, Qt::QueuedConnection);
    QObject::connect(swapMkt, &state::SwapMarketplace::onWithdrawIntegrityFees,
                     this, &SwapMarketplace::onWithdrawIntegrityFees, Qt::QueuedConnection);
    QObject::connect(swapMkt, &state::SwapMarketplace::onMarketPlaceOffersChanged,
                     this, &SwapMarketplace::onMarketPlaceOffersChanged, Qt::QueuedConnection);
    QObject::connect(swapMkt, &state::SwapMarketplace::onMyOffersChanged,
                     this, &SwapMarketplace::onMyOffersChanged, Qt::QueuedConnection);
    QObject::connect(swapMkt, &state::SwapMarketplace::onMessagingStatusChanged,
                     this, &SwapMarketplace::onMessagingStatusChanged, Qt::QueuedConnection);
}

SwapMarketplace::~SwapMarketplace() {}

// Return error message. Empty String on OK
// offerId - empty string for a new offer, Otherwise expected that it is an update
QString SwapMarketplace::createNewOffer( QString offerId, QString  account,
                                    bool sell, double  mwcAmount, double  secAmount,
                                    QString secondaryCurrency,  int mwcLockBlocks, int secLockBlocks,
                                    QString secAddress, double  secFee, QString note ) {
    return getSwapMkt()->createNewOffer(offerId, account, sell, mwcAmount, secAmount,
            secondaryCurrency, mwcLockBlocks, secLockBlocks,
            secAddress, secFee, note);
}

// triggers sgnMyOffersChanged  event
void SwapMarketplace::withdrawMyOffer(QString offerId) {
    getSwapMkt()->withdrawMyOffer(offerId);
}

// Json string with MySwap Offers info
QVector<QString> SwapMarketplace::getMyOffers() {
    QVector<state::MySwapOffer> offers = getSwapMkt()->getMyOffers();
    QVector<QString> res;
    for (const auto & o : offers) {
        res.push_back(o.toJsonStr());
    }
    return res;
}

// JSon strings with MktSwapOffer. Requesting marketplace offers with filtering
// selling: 0 - buy, 1-sell, 2 - all
// currency: empty value for all
QVector<QString> SwapMarketplace::getMarketOffers(double minFeeLevel, int selling, QString currency) {
    QVector<state::MktSwapOffer> offers = getSwapMkt()->getMarketOffers(minFeeLevel, selling, currency);
    QVector<QString> res;
    for (const auto & o : offers) {
        res.push_back(o.toJsonStr());
    }
    return res;
}

// return JSon strings with MktSwapOffer value. Empty string if such offer is not found
QString SwapMarketplace::getMarketOffer(QString offerId, QString walletAddress) {
    state::MktSwapOffer offer = getSwapMkt()->getMarketOffer(offerId, walletAddress);
    if (offer.isValid())
        return offer.toJsonStr();

    return "";
}

static QString getEstimatedNumber(int n) {
    Q_ASSERT(n>0);
    if (n>=10000)
        return "10000+";
    else if (n>=5000)
        return "5000+";
    else if (n>=1000)
        return "1000+";
    else if (n>=500)
        return "500+";
    else if (n>=100)
        return "100+";
    else if (n>=10)
        return "10+";
    else if (n>=5)
        return "5+";
    else
        return "1+";
}

QVector<QString> SwapMarketplace::getTotalOffers() {
    QVector<QString> res;

    for (auto of : getSwapMkt()->getTotalOffers()) {
        res.push_back(of.first);
        Q_ASSERT(of.second>0);
        res.push_back(getEstimatedNumber(of.second));
    }

    return res;
}

// Response at: sgnRequestIntegrityFees(QString error, int64_t balance, QVector<QString> IntegrityFeesJsonStr);
void SwapMarketplace::requestIntegrityFees() {
    getSwapMkt()->requestIntegrityFees();
}

double SwapMarketplace::getFeeLevel() {
    return getSwapMkt()->getFeeLevel();
}
QString SwapMarketplace::getFeeDepositAccount() {
    return getSwapMkt()->getFeeDepositAccount();
}
double SwapMarketplace::getFeeReservedAmount() {
    return getSwapMkt()->getFeeReservedAmount();
}

void SwapMarketplace::setIntegritySettings(const double & feeLevel, QString feeDepositAccount, const double & feeReservedAmount) {
    getSwapMkt()->setIntegritySettings(feeLevel, feeDepositAccount, feeReservedAmount);
}

// Response at: sgnWithdrawIntegrityFees(QString error, double mwc, QString account);
void SwapMarketplace::withdrawIntegrityFees() {
    getSwapMkt()->withdrawIntegrityFees();
}

// Return the listening status
QString SwapMarketplace::getOffersListeningStatus() {
    return getSwapMkt()->getOffersListeningStatus();
}

// Switch to create a new offer page. For new offer myMsgId must be empty string.
// Otherwise - exist offer id
void SwapMarketplace::pageCreateNewOffer(QString myMsgId) {
    getSwapMkt()->pageCreateUpdateOffer(myMsgId);
}

// Show marketplace page
void SwapMarketplace::pageMktList() {
    getSwapMkt()->pageMktList(false, false);
}

// Show my offers at marketplace page
void SwapMarketplace::pageMktMyOffers() {
    getSwapMkt()->pageMktList(true, false);
}

void SwapMarketplace::pageFee() {
    return getSwapMkt()->pageMktList(false, true);
}

// Show integrity fee  transactions
void SwapMarketplace::pageTransactionFee() {
    getSwapMkt()->pageFeeTransactions();
}

void SwapMarketplace::requestMktSwapOffers() {
    getSwapMkt()->requestMktSwapOffers();
}

// Accept the offer from marketplace
bool SwapMarketplace::acceptMarketplaceOffer(QString offerId, QString walletAddress) {
    return getSwapMkt()->acceptMarketplaceOffer(offerId, walletAddress);
}

// Request number of running MyOffers
int SwapMarketplace::getMyOffersNum() {
    return getSwapMkt()->getMyOffersNum();
}

void SwapMarketplace::stashMyOffers() {
    getSwapMkt()->stashMyOffers();
}

int SwapMarketplace::getLastNodeHeight() {
    return getSwapMkt()->getLastNodeHeight();
}

void SwapMarketplace::onRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees) {
    QVector<QString> integrityFeesJsonStr;
    for (const auto & f : fees) {
        integrityFeesJsonStr.push_back(f.toJSonStr());
    }
    emit sgnRequestIntegrityFees(error, balance, integrityFeesJsonStr);
}

void SwapMarketplace::onWithdrawIntegrityFees(QString error, double mwc, QString account) {
    emit sgnWithdrawIntegrityFees(error, mwc, account);
}

void SwapMarketplace::onMarketPlaceOffersChanged() {
    emit sgnMarketPlaceOffersChanged();
}

void SwapMarketplace::onMyOffersChanged() {
    emit sgnMyOffersChanged();
}

void SwapMarketplace::onMessagingStatusChanged() {
    emit sgnMessagingStatusChanged();
}


}


