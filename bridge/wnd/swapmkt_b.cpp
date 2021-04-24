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
QVector<QString> SwapMarketplace::getMarketOffers(double minFeeLevel, int selling, QString currency) const {
    QVector<state::MktSwapOffer> offers = getSwapMkt()->getMarketOffers(minFeeLevel, selling, currency);
    QVector<QString> res;
    for (const auto & o : offers) {
        res.push_back(o.toJsonStr());
    }
    return res;
}

QVector<QString> SwapMarketplace::getTotalOffers() {
    QVector<QString> res;

    for (auto of : getSwapMkt()->getTotalOffers()) {
        res.push_back(of.first);
        Q_ASSERT(of.second>0);
        res.push_back(QString::number(of.second));
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
    getSwapMkt()->pageMktList(false);
}

// Show my offers at marketplace page
void SwapMarketplace::pageMktMyOffers() {
    getSwapMkt()->pageMktList(false);
}

void SwapMarketplace::requestMktSwapOffers() {
    getSwapMkt()->requestMktSwapOffers();
}

// Accept the offer from marketplace
bool SwapMarketplace::acceptMarketplaceOffer(QString offerId, QString walletAddress) {
    return getSwapMkt()->acceptMarketplaceOffer(offerId, walletAddress);
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


