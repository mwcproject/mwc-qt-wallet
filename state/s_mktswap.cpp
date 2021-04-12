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

#include "s_mktswap.h"
#include "s_swap.h"
#include "statemachine.h"
#include "../core/TimerThread.h"
#include "../core/appcontext.h"
#include "../core/WndManager.h"
#include "../wallet/wallet.h"
#include <QTime>
#include <QJsonDocument>
#include "../util/Log.h"
#include <QDateTime>
#include "s_swap.h"

namespace state {

const int OFFER_PUBLISHING_INTERVAL_SEC = 120;
const QString SWAP_TOPIC = "SwapMarketplace";

const int MIN_PEERS_NUMBER = 5;

Swap * getSwap() {
    return (Swap *)getState(STATE::SWAP);
}

////////////////////////////////////////////////////////////////////////////////
//  MktSwapOffer
////////////////////////////////////////////////////////////////////////////////

static QJsonObject str2json(const QString & jsonStr) {
    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    // It is internal data, no errors expected
    Q_ASSERT(error.error == QJsonParseError::NoError);
    Q_ASSERT(jsonDoc.isObject());

    return jsonDoc.object();
}

MktSwapOffer::MktSwapOffer( const QString & jsonStr ) : MktSwapOffer(str2json(jsonStr)) {
}

MktSwapOffer::MktSwapOffer( const QJsonObject & json ) {
    int version = json["version"].toInt();
    Q_ASSERT(version==1); // Currently no needs to handle versions, there is only one

    id = json["id"].toString();
    sell = json["sell"].toBool();
    mwcAmount = json["mwcAmount"].toDouble();
    secAmount = json["secAmount"].toDouble();
    secondaryCurrency = json["secondaryCurrency"].toString();
    mwcLockBlocks = json["mwcLockBlocks"].toInt();
    secLockBlocks = json["secLockBlocks"].toInt();
    mktFee = json["mktFee"].toDouble();
    walletAddress = json["walletAddress"].toString();
}


QJsonObject MktSwapOffer::toJson() const {
    QJsonObject object
            {
                    {"version",           1},
                    {"id",                id},
                    {"sell",              sell},
                    {"mwcAmount",         mwcAmount},
                    {"secAmount",         secAmount},
                    {"secondaryCurrency", secondaryCurrency},
                    {"mwcLockBlocks",     mwcLockBlocks},
                    {"secLockBlocks",     secLockBlocks},
                    {"mktFee",            mktFee},
                    {"walletAddress",     walletAddress}
            };
    return object;
}

QString MktSwapOffer::toJsonStr() const {
    QJsonDocument doc(toJson());
    QString offerStrJson( doc.toJson(QJsonDocument::Compact));
    return offerStrJson;
}


double MktSwapOffer::getFeeLevel() const {
    return mktFee / mwcAmount;
}

QString MktSwapOffer::calcMwcLockTime() const {
    QVector<QString> lockTime = Swap::getLockTime( secondaryCurrency, 60, 60, mwcLockBlocks, secLockBlocks );
    Q_ASSERT(lockTime.size()==2);
    return lockTime[0];
}

QString MktSwapOffer::calcSecLockTime() const {
    QVector<QString> lockTime = Swap::getLockTime( secondaryCurrency, 60, 60, mwcLockBlocks, secLockBlocks );
    Q_ASSERT(lockTime.size()==2);
    return lockTime[1];
}

bool MktSwapOffer::isValid() const {
    Swap * swap = (Swap*) getStateMachine()->getState(STATE::SWAP);
    Q_ASSERT(swap!= nullptr);

    if (!swap->secondaryCurrencyList().contains(secondaryCurrency))
        return false;

    return  !id.isEmpty() && mwcAmount>0.0 && secAmount>0.0 &&
        mwcLockBlocks>0 && secLockBlocks>0;
}


////////////////////////////////////////////////////////////////////////////////
//  MySwapOffer
////////////////////////////////////////////////////////////////////////////////



// build form Json string
MySwapOffer::MySwapOffer(const QString jsonStr) : MySwapOffer(str2json(jsonStr)) {}

MySwapOffer::MySwapOffer(const QJsonObject & json) {
    msgUuid = json["msgUuid"].toString();
    offer = MktSwapOffer( json["offer"].toObject() );
    account = json["account"].toString();
    secAddress = json["secAddress"].toString();
    secFee = json["secFee"].toDouble();
    note = json["note"].toString();
    status = OFFER_STATUS(json["status"].toInt());
    integrityFee = wallet::IntegrityFees(json["integrityFee"].toObject());
}

double MySwapOffer::calcFee(double feeLevel) const {
    // 0.01  - minimal network fee for now.
    return std::max( offer.mwcAmount * feeLevel, 0.01 ); // add some small extra for rounding issue
}

QJsonObject MySwapOffer::toJson() const {
    QJsonObject res {
            { "msgUuid", msgUuid },
            { "offer", offer.toJson() },
            { "account", account },
            { "secAddress", secAddress },
            { "secFee", secFee },
            { "note", note },
            { "status", int(status) },
            { "integrityFee", integrityFee.toJSon() }
    };
    return res;
}

QString MySwapOffer::toJsonStr() const {
    QJsonDocument doc( toJson() );
    return doc.toJson(QJsonDocument::Compact);
};

QString MySwapOffer::getStatusStr() const {
    switch (status) {
        case OFFER_STATUS::PENDING: {
            return "Preparing...";
        }
        case OFFER_STATUS::STARTING: {
            return "Starting...";
        }
        case OFFER_STATUS::RUNNING: {
            return "Broadcasting";
        }
        default:
            Q_ASSERT(false);
            return "";
    }
}

// Offer description for user. Sell XX MWC for XX BTC
QString MySwapOffer::getOfferDescription() const {
    return (offer.sell ? "Selling " : "Buying ") + QString::number(offer.mwcAmount) +
            " MWC for " + QString::number(offer.secAmount) + " " + offer.secondaryCurrency;
}


////////////////////////////////////////////////////////////////////////////////
//  SwapMarketplace
////////////////////////////////////////////////////////////////////////////////

SwapMarketplace::SwapMarketplace(StateContext * context) :
        State(context, STATE::SWAP_MKT) {

    timer = new core::TimerThread(this, 5000);
    QObject::connect( timer, &core::TimerThread::onTimerEvent, this, &SwapMarketplace::onTimerEvent, Qt::QueuedConnection );
    timer->start();

    QObject::connect( context->wallet, &wallet::Wallet::onCreateIntegrityFee, this, &SwapMarketplace::respCreateIntegrityFee, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onRequestIntegrityFees, this, &SwapMarketplace::respRequestIntegrityFees, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onWithdrawIntegrityFees, this, &SwapMarketplace::respWithdrawIntegrityFees, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onRequestMessagingStatus, this, &SwapMarketplace::respRequestMessagingStatus, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onMessagingPublish, this, &SwapMarketplace::respMessagingPublish, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onCheckIntegrity, this, &SwapMarketplace::respCheckIntegrity, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onReceiveMessages, this, &SwapMarketplace::respReceiveMessages, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onStartListenOnTopic, this, &SwapMarketplace::onStartListenOnTopic, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onStopListenOnTopic, this, &SwapMarketplace::onStopListenOnTopic, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onNewMktMessage, this, &SwapMarketplace::onNewMktMessage, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onSendMarketplaceMessage, this, &SwapMarketplace::onSendMarketplaceMessage, Qt::QueuedConnection );

    swap = (Swap*) context->stateMachine->getState(STATE::SWAP);
}

SwapMarketplace::~SwapMarketplace() {
    timer->stop();
    timer->wait();
    delete timer;
}


NextStateRespond SwapMarketplace::execute() {
    selectedPage = SwapMarketplaceWnd::None;
    if (context->appContext->getActiveWndState() != STATE::SWAP_MKT)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    pageMktList(false);

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

bool SwapMarketplace::mobileBack() {
    switch (selectedPage) {
        case SwapMarketplaceWnd::None :
            return false;
        case SwapMarketplaceWnd::Marketplace:
            return false;
        case SwapMarketplaceWnd::NewOffer: {
            pageMktList(false);
            return true;
        }
    }
}

// Return error message. Empty String on OK
QString SwapMarketplace::createNewOffer( QString offerId, QString  account,
                        bool sell, double  mwcAmount, double  secAmount,
                        QString secondaryCurrency,  int mwcLockBlocks, int secLockBlocks,
                        QString secAddress, double  secFee, QString note ) {

    if (!swap->secondaryCurrencyList().contains(secondaryCurrency) || mwcAmount < 0.1 ||
        secAmount < swap->getSecMinAmount(secondaryCurrency) ||
        mwcLockBlocks <= 0 || secLockBlocks <= 0 || secFee < swap->getSecMinTransactionFee(secondaryCurrency) ||
        secFee > swap->getSecMaxTransactionFee(secondaryCurrency)) {
        return "Invalid offer parameters";
    }

    if (!offerId.isEmpty()) {
        for (auto & mf : myOffers) {
            if (mf.offer.id == offerId) {
                // Updating the offer
                mf.secAddress = secAddress;
                mf.secFee = secFee;
                mf.note = note;
                return "";
            }
        }
        return "Not found offer with Id " + offerId;
    }

    QString offerKey = "SwapOffer_" + QString::number(currentOfferId++);

    // Let's check if we have some funds and lock the funds.
    QPair<QString, QStringList> lockedOutputs;
    if (sell) {
        lockedOutputs = lockOutputsForSellOffer(account, mwcAmount, offerKey);
    }

    if (!lockedOutputs.first.isEmpty())
        return lockedOutputs.first;

    // Now we can create the offer and publish it...
    MySwapOffer offer(MktSwapOffer(offerKey,
                                   sell,
                                   mwcAmount,
                                   secAmount,
                                   secondaryCurrency,
                                   mwcLockBlocks,
                                   secLockBlocks),
                      account,
                      secAddress,
                      secFee,
                      note,
                      lockedOutputs.second);

    myOffers.push_back(offer);
    emit onMyOffersChanged();
    return "";
}

QPair<QString, QStringList> SwapMarketplace::lockOutputsForSellOffer(const QString & account, double mwcAmount, QString offerId) {
    const QMap<QString, QVector<wallet::WalletOutput> > &outputs = context->wallet->getwalletOutputs();
    QVector<wallet::WalletOutput> outs = outputs.value(account);

    std::sort(outs.begin(), outs.end(), [](const wallet::WalletOutput &o1, const wallet::WalletOutput &o2) {
        return o1.valueNano < o2.valueNano;
    });
    int64_t needAmount = int64_t((mwcAmount + 0.05) * 1000000000.0 + 0.5);
    int64_t reservedAmounts = needAmount;
    int64_t foundAmount = 0;
    QStringList output2lock;
    for (auto &o : outs) {
        if (!o.isUnspent())
            continue;
        if (context->appContext->isLockedOutputs(o.outputCommitment).first)
            continue;
        foundAmount += o.valueNano;
        reservedAmounts -= o.valueNano;
        output2lock += o.outputCommitment;
        if (reservedAmounts <= 0)
            break;
    }

    if (reservedAmounts > 0) {
        return QPair<QString, QStringList>("There is not enough funds at account " + account + ". You need " + util::nano2one(needAmount) +
               " MWC, but have available " + util::nano2one(foundAmount) + " MWC", {});
    }

    for (auto &s : output2lock)
        context->appContext->setLockedOutput(s, true, offerId);

    return QPair<QString, QStringList>("", output2lock);
}

void SwapMarketplace::withdrawMyOffer(QString offerId) {
    for ( int i=myOffers.length()-1; i>=0; i-- ) {
        if (myOffers[i].offer.id == offerId) {
            if ( !myOffers[i].msgUuid.isEmpty() ) {
                qDebug() << "calling messageWithdraw for " << myOffers[i].msgUuid;
                context->wallet->messageWithdraw(myOffers[i].msgUuid);
            }
            context->appContext->unlockOutputsById( myOffers[i].offer.id );
            myOffers.remove(i);
            break;
        }
    }

    emit onMyOffersChanged();
}


// Request marketplace offers with filtering
QVector<MktSwapOffer> SwapMarketplace::getMarketOffers(double minFeeLevel, bool selling, QString currency, double minMwcAmount, double maxMwcAmount ) {
    cleanMarketOffers();

    int64_t timeLimit = QDateTime::currentSecsSinceEpoch() - OFFER_PUBLISHING_INTERVAL_SEC*2;

    QVector<MktSwapOffer> result;
    for ( auto & ofr : marketOffers ) {
        if (ofr.timestamp < timeLimit)
            continue;

        if ( ofr.sell == selling && ofr.getFeeLevel() >= minFeeLevel && currency==ofr.secondaryCurrency &&
             (minMwcAmount<=0.0 || ofr.mwcAmount >= minMwcAmount) &&
             (maxMwcAmount<=0.0 || ofr.mwcAmount <= maxMwcAmount) )
            result.push_back(ofr);
    }
    return result;
}

// All marketplace offers that are published buy currency. Sorted by largest number
QVector<QPair<QString,int>> SwapMarketplace::getTotalOffers() {
    QHash<QString, int> offers;
    for ( auto & mo : marketOffers ) {
        offers.insert( mo.secondaryCurrency, offers.value(mo.secondaryCurrency, 0) + 1);
    }

    QVector<QPair<QString,int>> result;
    for (auto oi = offers.begin(); oi!=offers.end(); oi++)
        result.push_back( QPair<QString,int>(oi.key(), oi.value()) );

    std::sort( result.begin(), result.end(), [](const QPair<QString,int> &o1, const QPair<QString,int> &o2) {
        return o1.second > o2.second;
    } );

    return result;
}


// Response at: onIntegrityFees(QVector<IntegrityFees> fees)
void SwapMarketplace::requestIntegrityFees() {
    context->wallet->requestIntegrityFees();
}

double SwapMarketplace::getFeeLevel() const {
    return context->appContext->getMktFeeLevel();
}

double SwapMarketplace::getFeeReservedAmount() const {
    return context->appContext->getMktFeeReservedAmount();
}


QString SwapMarketplace::getFeeDepositAccount() const {
    QString account = context->appContext->getMktFeeDepositAccount();
    for ( auto & acc : context->wallet->getWalletBalance() ) {
        if (acc.accountName == account)
            return account;
    }
    return "default";
}

void SwapMarketplace::setIntegritySettings(const double & feeLevel, QString feeDepositAccount, const double & feeReservedAmount) {
    context->appContext->setMktFeeLevel(feeLevel);
    context->appContext->setMktFeeDepositAccount(feeDepositAccount);
    context->appContext->setMktFeeReserveAmount(feeReservedAmount);
}

// Response at: onWithdrawIntegrityFees(QString error)
void SwapMarketplace::withdrawIntegrityFees() {
    context->wallet->withdrawIntegrityFees( getFeeDepositAccount() );
    // continue at respWithdrawIntegrityFees
}

void SwapMarketplace::onTimerEvent() {
    int64_t curTime = QDateTime::currentSecsSinceEpoch();

    // checking the libp2p status periodically
    if (context->wallet->isWalletRunningAndLoggedIn()) {
        if (lastMessagingStatusRequest + 20 < curTime ) {
            context->wallet->requestMessagingStatus();
        }
    }

    // It is a main timer that triggers whole workflow
    if (startMktListening==0)
        return;

    // FIX ME!!!!   It is ok for initial testing while there are no peers!!!!
/*    if (messagingStatus.gossippub_peers.size() < 3) {
        startMktListening = QDateTime::currentSecsSinceEpoch();
        return; // waiting for more connections
    }*/

    requestMktSwapOffers();

    // Check if we need to start anything
    bool hasPending = false;
    bool hasStarting = false;
    for (MySwapOffer ms : myOffers) {
        switch (ms.status) {
            case OFFER_STATUS::PENDING: {
                hasPending = true;
                break;
            }
            case OFFER_STATUS::STARTING: {
                hasStarting = true;
                break;
            }
            default: {}
        }
    }
    if (hasStarting) {
        context->wallet->requestIntegrityFees();
    }
    else if (hasPending) {
        updateIntegrityFeesAndStart();
    }

    // Check if need to update
    if (!myOffers.isEmpty() && lastCheckIntegrity + 60*5 < curTime ) {
        lastCheckIntegrity = curTime;
        context->wallet->checkIntegrity();
        // continue at respCheckIntegrity...
    }

    // Validating locked output...
    const QMap<QString, QVector<wallet::WalletOutput> > & outputs = context->wallet->getwalletOutputs();
    QSet<QString> allOutputs;
    for (auto oi = outputs.begin(); oi != outputs.end(); oi++ ) {
        for (auto out : oi.value()) {
            if (out.isUnspent()) {
                allOutputs += out.outputCommitment;
            }
        }
    }

    for (int i=myOffers.size()-1; i>=0; i--) {
        auto & mo = myOffers[i];
        bool outputGone = false;
        for (auto & o : mo.outputs) {
            if (!allOutputs.contains(o)) {
                outputGone = true;
            }
        }

        if (outputGone) {
            qDebug() << "Need to recalculate outputs for my offer " << mo.offer.id;
            context->appContext->unlockOutputsById(mo.offer.id);

            auto lockedOutputs = lockOutputsForSellOffer(mo.account, mo.offer.mwcAmount, mo.offer.id);
            if (!lockedOutputs.first.isEmpty()) {
                myOffers.remove(i);
                core::getWndManager()->messageTextDlg("Warning", "You sell offer is rejected because outputs that it depend on are spent. It can happen because of integrity fee payment.\n\n" + lockedOutputs.first);
                // return because of the message box delay. The data can be changed by that time.
                return;
            }

            mo.outputs = lockedOutputs.second;
        }
    }
}

void SwapMarketplace::updateIntegrityFeesAndStart() {
    QVector<double> fees;
    double feeLevel = getFeeLevel();
    for (MySwapOffer ms : myOffers) {
        fees.push_back( ms.calcFee( feeLevel ) + 0.0000001 ); // add some for rounding
    }

    context->wallet->createIntegrityFee( getFeeDepositAccount(), getFeeReservedAmount(), fees );
    // continue at respCreateIntegrityFee
}


void SwapMarketplace::respCreateIntegrityFee(QString error, QVector<wallet::IntegrityFees> fees) {
    if (!error.isEmpty()) {
        logger::logInfo("SwapMarketplace", "Unable to create Integrity fees, " + error);

        if (error.contains("Not enough funds")) {
            // Not enough funds, deleting all pending my offers
            int deletedOffers = 0;
            for (int i=myOffers.size()-1; i>=0; i--) {
                if (myOffers[i].status == OFFER_STATUS::PENDING) {
                    context->appContext->unlockOutputsById( myOffers[i].offer.id );
                    myOffers.remove(i);
                    deletedOffers++;
                }
            }
            if (deletedOffers>0) {
                if (deletedOffers==1) {
                    core::getWndManager()->messageTextDlg("Warning",
                                                          "Your account doesn't have enough funds to pay integrity fees for your offer. This offers will be deleted");
                }
                else {
                    core::getWndManager()->messageTextDlg("Warning",
                                                          "Your account doesn't have enough funds to pay integrity fees for " +
                                                          QString::number(deletedOffers) +
                                                          " offers. Those offers will be deleted");
                }
                emit onMyOffersChanged();
            }
        }

        return;
    }

    // Check if has any pending.
    if (std::find_if( myOffers.begin(), myOffers.end(), []( const MySwapOffer & offer) { return offer.status == OFFER_STATUS::PENDING;} ) == myOffers.end()) {
        // Not found any pending, nothing need to be done.
        return;
    }

    {
        double feeLevel = getFeeLevel();
        std::sort(myOffers.begin(), myOffers.end(), [feeLevel](const MySwapOffer &o1, const MySwapOffer &o2) {
            return o1.calcFee(feeLevel) < o2.calcFee(feeLevel);
        });
    }

    std::sort( fees.begin(), fees.end(), [](const wallet::IntegrityFees & f1, const wallet::IntegrityFees & f2) {
        return f1.fee < f2.fee;
    } );


    // Trying to satisfy the offers
    double feeLevel = getFeeLevel();
    for ( MySwapOffer & offer : myOffers ) {
        double offerFee = offer.calcFee(feeLevel);
        bool foundFee = false;
        for ( int i=0; i<fees.size(); i++ ) {
            wallet::IntegrityFees f = fees[i];
            if ( offerFee <= f.toDblFee() ) {
                // applying this fee
                if (offer.status == OFFER_STATUS::RUNNING) {
                    if (offer.integrityFee.uuid != f.uuid) {
                        // Updating and restarting
                        // Stopping first
                        context->wallet->messageWithdraw( offer.msgUuid );
                        offer.status = OFFER_STATUS::STARTING;
                        offer.integrityFee = f;
                        offer.msgUuid = "";
                    }
                }
                else if (offer.status == OFFER_STATUS::PENDING) {
                    // Assign fee and starting
                    offer.integrityFee = f;
                    offer.status = OFFER_STATUS::STARTING;
                }

                fees.remove(i);
                foundFee =  true;
                break;
            }

            if (!foundFee) {
                if (offer.status == OFFER_STATUS::RUNNING) {
                    // Stop it. It will wait for a new fee...
                    logger::logInfo("SwapMarketplace", "Stopping My offer with ID " + offer.offer.id + ", Integration fee is not active any more" );
                    context->wallet->messageWithdraw( offer.msgUuid );
                    offer.status = OFFER_STATUS::PENDING;
                    offer.integrityFee = wallet::IntegrityFees();
                    offer.msgUuid = "";
                }
            }
        }
    }

    emit onMyOffersChanged();
}

void SwapMarketplace::respRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees) {
    //
    emit onRequestIntegrityFees(error, balance, fees);

    // Let's update offers that are ruuning without active fees..

    for ( MySwapOffer & offer : myOffers ) {
        if (offer.status!=OFFER_STATUS::RUNNING)
            continue;

        QString feeUuid = offer.integrityFee.uuid;
        if ( std::find_if(fees.begin(),fees.end(), [feeUuid](const wallet::IntegrityFees & f) { return f.uuid==feeUuid; } ) == fees.end() ) {
            // Not found the fee any more.
            logger::logInfo("SwapMarketplace", "Stopping My offer with ID " + offer.offer.id + ", Integration fee is not active any more" );
            context->wallet->messageWithdraw( offer.msgUuid );
            offer.status = OFFER_STATUS::PENDING;
            offer.integrityFee = wallet::IntegrityFees();
            offer.msgUuid = "";
        }
    }

    // Let's check if we need to broadcast something...
    for (MySwapOffer & ms : myOffers) {
        if (ms.status == OFFER_STATUS::STARTING ) {
            bool isConfirmed = false;
            for ( auto & f : fees ) {
                if (f.uuid == ms.integrityFee.uuid) {
                    isConfirmed = f.confirmed;
                    break;
                }
            }

            if (isConfirmed) {
                // let's start the broadcasting...
                context->wallet->messagingPublish( ms.offer.toJsonStr(), ms.integrityFee.uuid, ms.offer.id, OFFER_PUBLISHING_INTERVAL_SEC, SWAP_TOPIC );
            }
        }
    }
}

void SwapMarketplace::respWithdrawIntegrityFees(QString error, double mwc, QString account) {
    emit onWithdrawIntegrityFees(error, mwc, account);

    // triggering active fees request to refresh the state
    requestIntegrityFees();
}

void SwapMarketplace::respRequestMessagingStatus(QString error, wallet::MessagingStatus status) {
    if (error.isEmpty()) {
        lastMessagingStatusRequest = QDateTime::currentSecsSinceEpoch();
        messagingStatus = status;

        if (!messagingStatus.connected || messagingStatus.gossippub_peers.size()<MIN_PEERS_NUMBER)
            startMktListening = QDateTime::currentSecsSinceEpoch();
    }
    else {
        qDebug() << "respRequestMessagingStatus return error: " << error;
        lastMessagingStatusRequest = 0;
        messagingStatus = wallet::MessagingStatus();
    }
    emit onMessagingStatusChanged();
}

void SwapMarketplace::respMessagingPublish(QString id, QString uuid, QString error) {
    if (error.isEmpty()) {
        // success, can change the message status
        for (auto & offer : myOffers) {
            if (offer.offer.id == id) {
                offer.msgUuid = uuid;
                offer.status = OFFER_STATUS::RUNNING;
                emit onMyOffersChanged();
                logger::logInfo("SwapMarketplace", "Offer " + id + " is published. Assigned message ID " + uuid );
                break;
            }
        }
    }
}

void SwapMarketplace::respCheckIntegrity(QString error, QVector<QString> expiredMsgUuid) {
    if (!error.isEmpty()) {
        qDebug() << "respCheckIntegrity get error: " << error;
        return;
    }

    if (expiredMsgUuid.isEmpty())
        return;

    logger::logInfo("SwapMarketplace", "Swap Offers has expired fees. We need to handle that: " + QStringList(expiredMsgUuid.toList()).join(", ") );

    for (auto & offer : myOffers ) {
        if (expiredMsgUuid.contains(offer.msgUuid)) {
            // don't need response. In any case if ot fail, we can't do much
            context->wallet->messageWithdraw(offer.msgUuid);
            logger::logInfo("SwapMarketplace", "The fee is expired for "+offer.msgUuid+" or " + offer.offer.id + " changing status to Pending" );
            offer.msgUuid = "";
            offer.status = OFFER_STATUS::PENDING;
            emit onMyOffersChanged();
        }
    }
}


/*void SwapMarketplace::startBroadcastMessages( QVector<wallet::IntegrityFees> fees ) {

}*/

// Request a new Marketplace offers from the wallet.
// Respond: onMarketPlaceOffersChanged
void SwapMarketplace::requestMktSwapOffers() {
    context->wallet->requestReceiveMessages(true);
}

void SwapMarketplace::respReceiveMessages(QString error, QVector<wallet::ReceivedMessages> msgs) {
    if (!error.isEmpty())
        return; // Should have the message at notifications. But there is nothing what we can do at handler

    QVector<QString> startMsgIds;
    for (auto & offer : marketOffers) {
        startMsgIds.push_back(offer.walletAddress + "_" + offer.id);
    }
    std::sort(startMsgIds.begin(), startMsgIds.end());


    // Updating  marketOffers
    for ( const auto & m : msgs) {
        if (m.topic!=SWAP_TOPIC)
            continue;

        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(m.message.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
            qDebug() << "Unable to process libp2p message " << m.message << "  wallet " << m.wallet;
            continue;
        }

        QJsonObject swapMsgJson = jsonDoc.object();

        MktSwapOffer offer(swapMsgJson);
        if (!offer.isValid()) {
            qDebug() << "Get invalid offer message " << m.message << "  wallet " << m.wallet;
            continue;
        }

        offer.mktFee = double(m.fee) / 1000000000.0;
        offer.walletAddress = m.wallet;
        offer.timestamp = QDateTime::currentSecsSinceEpoch();

        // Updating this offer
        QString key = offer.getKey();
        marketOffers.insert(key, offer);
    }
    cleanMarketOffers();

    QVector<QString> endMsgIds;
    for (auto & offer : marketOffers) {
        endMsgIds.push_back(offer.walletAddress + "_" + offer.id);
    }
    std::sort(endMsgIds.begin(), endMsgIds.end());

    // Emit change only if datta was changed because of UI.
    if (startMsgIds != endMsgIds) {
        emit onMarketPlaceOffersChanged();
    }

}


QString SwapMarketplace::getOffersListeningStatus() const {
    if (startMktListening==0 || !messagingStatus.connected || messagingStatus.topics.isEmpty())
        return "not listening";

    if (messagingStatus.gossippub_peers.isEmpty() ) {
        return "connecting...";
    }

    if (messagingStatus.gossippub_peers.size() < MIN_PEERS_NUMBER) {
        return "found " + QString::number(messagingStatus.gossippub_peers.size()) + " peers";
    }

    int64_t collectingTime = QDateTime::currentSecsSinceEpoch() - startMktListening;
    Q_ASSERT(collectingTime>=0);

    if ( collectingTime < OFFER_PUBLISHING_INTERVAL_SEC ) {
        return "Collecting marketplace offers... " + QString::number( collectingTime * 100 / OFFER_PUBLISHING_INTERVAL_SEC ) + "%";
    }

    return "Listening";
}
// start/stop offers publishing
void SwapMarketplace::setListeningForOffers(bool start) {
    if (start) {
        context->wallet->startListenOnTopic(SWAP_TOPIC);
        // resp at onStartListenOnTopic
    }
    else {
        startMktListening = 0;
        context->wallet->stopListenOnTopic(SWAP_TOPIC);
        // resp at onStopListenOnTopic
    }
    context->wallet->requestMessagingStatus();
}

void SwapMarketplace::onStartListenOnTopic(QString error) {
    if (!error.isEmpty()) {
        startMktListening = 0;
        core::getWndManager()->messageTextDlg("Error", "Unable to start listening for atomic swap marketplace messages.");
        return;
    }
}

void SwapMarketplace::onStopListenOnTopic(QString error) {
    Q_UNUSED(error);
    startMktListening = 0;
}


// Switch to create a new offer page. For new offer myMsgId must be empty string.
// Otherwise - exist offer id
void SwapMarketplace::pageCreateUpdateOffer(QString myMsgId) {
    selectedPage = SwapMarketplaceWnd::NewOffer;
    core::getWndManager()->pageNewUpdateOffer(myMsgId);
}

// Switch to swap marketplace first page
void SwapMarketplace::pageMktList(bool selectMyOffers) {
    selectedPage = SwapMarketplaceWnd::Marketplace;
    core::getWndManager()->pageMarketplace(selectMyOffers);

}

void SwapMarketplace::cleanMarketOffers() {
    int64_t curTime = QDateTime::currentSecsSinceEpoch();
    if (curTime - OFFER_PUBLISHING_INTERVAL_SEC < lastMarketOffersCleaning)
        return;

    lastMarketOffersCleaning = curTime;
    // Removing all records with expired time
    int64_t expiredTime = curTime - OFFER_PUBLISHING_INTERVAL_SEC * 2;

    QMutableHashIterator<QString, MktSwapOffer> i(marketOffers);
    while (i.hasNext()) {
        i.next();
        if (i.value().timestamp < expiredTime)
            i.remove();
    }
}

void SwapMarketplace::onNewMktMessage(int messageId, QString wallet_tor_address, QString offer_id) {
    if (messageId == wallet::WALLET_EVENTS::S_MKT_ACCEPT_OFFER) {
        // Our offer is accepted, let's check if it is running...
        for (const auto & offer : myOffers) {
            if (offer.offer.id == offer_id) {
                // we get an offer, let's start trading and notify the user...
                createNewSwapTrade(offer, wallet_tor_address);
                core::getWndManager()->messageTextDlg("Offer is accepted", "Congratulations, you offer " + offer.getOfferDescription() +
                    " is accepted, the swap trade is started. Please note, we will keep this offer active until some of your partners will lock the coins" );
            }
        }
    }
    else if (messageId == wallet::WALLET_EVENTS::S_MKT_FAIL_BIDDING) {
        // The related trades should be cancelled automatically. We just need to show the message. So delegate that to the swaps
        getSwap()->failBidding(wallet_tor_address, offer_id);
    }
    else {
        Q_ASSERT(false);
    }
}

void SwapMarketplace::createNewSwapTrade( MySwapOffer offer, QString wallet_tor_address) {
    getSwap()->startTrading(offer, wallet_tor_address);
}

// Accept the offer from marketplace
bool SwapMarketplace::acceptMarketplaceOffer(QString offerId, QString walletAddress) {
    QString key = walletAddress + "_" + offerId;

    MktSwapOffer mktOffer = marketOffers.value(key);
    if (mktOffer.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Unfortunately you can't accept offer from "+walletAddress+". It is not on the market any more.");
        return false;
    }

    // Let's send accept offer message to another wallet.
    context->wallet->sendMarketplaceMessage("accept_offer", walletAddress, offerId);
    return true;
}

// Response from sendMarketplaceMessage
void SwapMarketplace::onSendMarketplaceMessage(QString error, QString response, QString offerId, QString walletAddress) {

    if (!error.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Unable to accept offer from "+walletAddress+" because of the error:\n" + error);
        return;
    }

    QString key = walletAddress + "_" + offerId;

    MktSwapOffer mktOffer = marketOffers.value(key);
    if (mktOffer.isEmpty()) {
        core::getWndManager()->messageTextDlg("Error", "Unfortunately you can't accept offer from "+walletAddress+". It is not on the market any more.");
        return;
    }

    // Check the response. It is a Json, but we can parse it manually. We need to find how many offers are in process.
    // "{"running":2}"
    int idx1 = response.indexOf(':');
    int idx2 = response.indexOf('}', idx1+1);
    int running_num = 100;
    bool ok = false;
    if (idx1<idx2 && idx1>0) {
        running_num = response.mid(idx1+1, idx2-idx1-1).trimmed().toInt(&ok);
    }

    if (!ok || running_num<0) {
        core::getWndManager()->messageTextDlg("Error", "Unable to accept offer from "+walletAddress+" because of unexpected response:\n" + response);
        return;
    }

    if (running_num>0) {
        // There are something already going, let's report it.
        if ( core::WndManager::RETURN_CODE::BTN1 != core::getWndManager()->questionTextDlg("Warning", "Wallet "+walletAddress+" already has " + QString::number(running_num) + " accepted trades. Only one trade that lock "
                           "coins first will continue, the rest will be cancelled. As a result your trade might be cancelled even you lock the coins.\n\n"
                           "You can wait for some time, try to accept this offer later. Or you can continue, you trade might win.\n\n "
                           "Do you want to continue and start trading?",
        "Yes", "No",
        "I understand the risk and I want to continue", "No, I will better wait",
        false, true) )
        {
            getSwap()->rejectOffer(mktOffer, walletAddress);
            return;
        }
    }

    // Finally, we are good to accept the offer.
    getSwap()->acceptOffer(mktOffer,  walletAddress);
}




}

