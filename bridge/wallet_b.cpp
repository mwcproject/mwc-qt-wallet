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

#include "wallet_b.h"
#include "../core/Notification.h"
#include "../core/WndManager.h"
#include "../state/state.h"
#include "../wallet/wallet.h"
#include "../util/Files.h"
#include "../util/Log.h"
#include <QDebug>
#include <QFileInfo>

#include "core/appcontext.h"
#include "util/message_mapper.h"

namespace bridge {

static wallet::Wallet * getWallet() {
    return state::getStateContext()->wallet;
}

static core::AppContext * getAppContext() {
    return state::getStateContext()->appContext;
}


Wallet::Wallet(QObject *parent) : QObject(parent) {
    wallet::Wallet * wallet = getWallet();

    QObject::connect(wallet, &wallet::Wallet::onConfigUpdate,
                         this, &Wallet::onConfigUpdate, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onLogin,
                     this, &Wallet::onLogin, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onLogout,
                         this, &Wallet::onLogout, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onScanProgress,
                         this, &Wallet::onScanProgress, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onScanDone,
                         this, &Wallet::onScanDone, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onSend,
                         this, &Wallet::onSend, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onSlateReceivedFrom,
                         this, &Wallet::onSlateReceivedFrom, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onScanRewindHash,
                         this, &Wallet::onScanRewindHash, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onWalletBalanceUpdated,
                         this, &Wallet::onWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onFaucetMWCDone,
                         this, &Wallet::onFaucetMWCDone, Qt::QueuedConnection);
}

Wallet::~Wallet() {}

// Return true if wallet has this password. Wallet might not have password (has empty password) if it was created manually.
// Expected that the wallet is already open,
bool Wallet::checkPassword(QString password) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::checkPassword with <password>");
    return getWallet()->checkPassword(password);
}

// Return 4 values:
// [0]  mqs_started
// [1]  mqs_healthy
// [2]  tor_started
// [3]  tor_healthy
QVector<bool> Wallet::getListenerStatus() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getListenerStatus");
    wallet::ListenerStatus status = getWallet()->getListenerStatus();
    QVector<bool> res = { status.mqs_started, status.mqs_healthy, status.tor_started, status.tor_healthy };
    return res;
}

// Request MQS address
QString Wallet::getMqsAddress() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getMqsAddress");
    return getWallet()->getMqsAddress();
}

// Request Tor address
QString Wallet::getTorSlatepackAddress() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getTorSlatepackAddress");
    return getWallet()->getTorSlatepackAddress();
}

// request current address index
int Wallet::getAddressIndex() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getAddressIndex");
    return getWallet()->getAddressIndex();
}

// Note, set address index does update the MQS and Tor addresses
// The Listeners, if running, will be restarted automatically
void Wallet::setAddressIndex(int index) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::setAddressIndex with index=" + QString::number(index));
    return getWallet()->setAddressIndex(index);
}

// Request accounts info
// includeAccountName - return Account names
// includeAccountFullInfo - return account full info
QVector<QString> Wallet::getWalletBalance(bool includeAccountName, bool includeAccountPath, bool includeSpendableInfo, bool includeAccountFullInfo) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getWalletBalance with includeAccountName=" + QString(includeAccountName ? "true" : "false") + 
                    " includeAccountPath=" + QString(includeAccountPath ? "true" : "false") + 
                    " includeSpendableInfo=" + QString(includeSpendableInfo ? "true" : "false") + 
                    " includeAccountFullInfo=" + QString(includeAccountFullInfo ? "true" : "false"));

    int confNum = getAppContext()->getSendCoinsParams().inputConfirmationNumber;

    QVector<QString> result;
    QVector<wallet::AccountInfo> accs = getWallet()->getWalletBalance(confNum, true, getAppContext()->getLockedOutputs());
    for (auto & a : accs) {
        if (includeAccountName)
            result.push_back(a.accountName);
        if (includeAccountPath)
            result.push_back(a.accountPath);
        if (includeSpendableInfo)
            result.push_back(a.getSpendableAccountName());
        if (includeAccountFullInfo)
            result.push_back(a.getLongAccountName());
    }

    return result;
}

// return current account path
QString Wallet::getCurrentAccountId() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getCurrentAccountId");
    return getWallet()->getCurrentAccountId();
}

// Switch to different account
void Wallet::switchAccountById(QString accountPath) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::switchAccountById with accountPath=" + accountPath);

    int walletId = getWallet()->getWalletId();
    getAppContext()->setWalletParam(walletId, core::WALLET_PARAM_SELECTED_ACCOUNT_PATH, accountPath, false);
    return getWallet()->switchAccountById(accountPath);
}

// Show outputs for the wallet
QJsonArray Wallet::getOutputs(QString accountPath, bool show_spent) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getOutputs with accountPath=" + accountPath + " show_spent=" + QString(show_spent ? "true" : "false"));
    QVector<wallet::WalletOutput> outputs = getWallet()->getOutputs(accountPath, show_spent);
    QJsonArray result;

    for ( const wallet::WalletOutput & out : outputs ) {
        result.push_back(out.toJson());
    }

    return result;
}

// Show outputs for the wallet
QJsonArray Wallet::getOutputsByCommits(QString accountPath, QVector<QString> commits) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getOutputsByCommits with accountPath=" + accountPath + " commits.count=" + QString::number(commits.size()));
    // Requesting all outputs is the best option. We can't access by commit
    QVector<wallet::WalletOutput> outputs = getWallet()->getOutputs(accountPath, true);
    QMap<QString, int> outputMapping;

    for ( int i=0; i<outputs.size(); i++ ) {
        outputMapping[outputs[i].outputCommitment] = i;
    }

    QJsonArray result;
    for (const QString & c : commits) {
        int oidx = outputMapping.value(c, -1);
        if (oidx>=0)
            result.push_back( outputs[oidx].toJson() );
    }
    return result;
}


// Set account that will receive the funds
void Wallet::setReceiveAccountById(QString accountPath) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::setReceiveAccountById with accountPath=" + accountPath);

    int walletId = getWallet()->getWalletId();
    getAppContext()->setWalletParam(walletId, core::WALLET_PARAM_RECEIVE_ACCOUNT_PATH, accountPath, true);
    getWallet()->setReceiveAccountById(accountPath);
}

QString Wallet::getReceiveAccountPath() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getReceiveAccountPath");
    return getWallet()->getReceiveAccountPath();
}

// Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
QJsonObject Wallet::decodeSlatepack( QString slatepackContent) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::decodeSlatepack with slatepackContent=<hidden>");
    wallet::DecodedSlatepack res = getWallet()->decodeSlatepack(slatepackContent);
    if (!res.error.isEmpty())
        res.error = util::mapMessage(res.error);

    return res.toJson();
}

// Show all transactions for current account
QJsonArray Wallet::getTransactions( QString accountPath ) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getTransactions with accountPath=" + accountPath);
    QVector<wallet::WalletTransaction> txs = getWallet()->getTransactions(accountPath);
    QJsonArray res;
    for (const auto & t : txs) {
        res.push_back(t.toJson());
    }
    return res;
}

// Request single transaction by UUID, any account
QJsonObject Wallet::getTransactionByUUID(QString tx_uuid) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getTransactionByUUID with tx_uuid=" + tx_uuid);
    wallet::WalletTransaction tx = getWallet()->getTransactionByUUID(tx_uuid);
    return tx.toJson();
}


// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
// Return error or JsonObject
QString Wallet::generateTransactionProof( QString transactionUuid ) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::generateTransactionProof with transactionUuid=" + transactionUuid);
    return getWallet()->generateTransactionProof(transactionUuid);
}

// Verify the proof for transaction
// Return error or JsonObject
QString Wallet::verifyTransactionProof( QString proof ) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::verifyTransactionProof with <proof>");
    return getWallet()->verifyTransactionProof(proof);
}

// Check if slatepack data does exist
bool Wallet::hasSendSlatepack(QString txUUID) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::hasSendSlatepack with txUUID=" + txUUID);
    return state::getStateContext()->appContext->hasSendSlatepack(txUUID);
}

bool Wallet::hasReceiveSlatepack(QString txUUID) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::hasReceiveSlatepack with txUUID=" + txUUID);
    return state::getStateContext()->appContext->hasReceiveSlatepack(txUUID);
}

bool Wallet::hasFinalizedData(QString txUUID) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::hasFinalizedData with txUUID=" + txUUID);
    return getWallet()->hasFinalizedData(txUUID);
}


// Request to show the slatepack data
void Wallet::viewSendSlatepack(QString txUUID) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::viewSendSlatepack with txUUID=" + txUUID);
    QString slatepack = state::getStateContext()->appContext->getSendSlatepack(txUUID);
    if (slatepack.isEmpty())
        return;

    core::getWndManager()->pageShowSlatepack(slatepack, txUUID, state::STATE::TRANSACTIONS, ".send_init.slatepack", true);
}

void Wallet::viewReceiveSlatepack(QString txUUID) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::viewReceiveSlatepack with txUUID=" + txUUID);
    QString slatepack = state::getStateContext()->appContext->getReceiveSlatepack(txUUID);
    if (slatepack.isEmpty())
        return;
    core::getWndManager()->pageShowSlatepack(slatepack, txUUID, state::STATE::TRANSACTIONS, ".send_response.slatepack", false);
}

// Cancel TX by UUID (in case of multi accouns, we want to cancel both)
// Return error string
QString Wallet::cancelTransacton(QString txUUID) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::cancelTransacton with txUUID=" + txUUID);
    return getWallet()->cancelTransacton(txUUID);
}

// Repost the transaction.
// Return Error
QString Wallet::repostTransaction(QString txUUID, bool fluff) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::repostTransaction with txUUID=" + txUUID + " fluff=" + QString(fluff ? "true" : "false"));
    return getWallet()->repostTransaction(txUUID, fluff);
}

// Rename account
void Wallet::renameAccountById( QString accountPath, QString newAccountName) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::renameAccountById with accountPath=" + accountPath + " newAccountName=" + newAccountName);
    getWallet()->renameAccountById(accountPath, newAccountName);
}

// Create another account, note no delete exist for accounts
// Return account path
QString Wallet::createAccount( const QString & accountName ) {
    logger::logInfo(logger::BRIDGE, "Call Wallet::createAccount with accountName=" + accountName);
    return getWallet()->createAccount(accountName);
}

// Get rewind hash
QString Wallet::viewRewindHash() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::viewRewindHash");
    return getWallet()->viewRewindHash();
}

// Return Total MWC and Unconfirmed MWC
// [0] - total MWC whole
// [1] - total MWC full fractions (9 digits)
// [2] - unconfirmed
QVector<QString> Wallet::getTotalAmount() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::getTotalAmount");
    int confNum = getAppContext()->getSendCoinsParams().inputConfirmationNumber;

    QVector<wallet::AccountInfo> balance = getWallet()->getWalletBalance(confNum, true, getAppContext()->getLockedOutputs());

    qint64 mwcSum = 0;
    qint64 awaiting = 0;
    for ( const auto & ai : balance ) {
        mwcSum += ai.total;
        awaiting += ai.awaitingConfirmation;
    }

    QString mwcTotalFraction = QString::number( mwcSum % qint64(1000000000),10);
    while (mwcTotalFraction.length()<9)
        mwcTotalFraction = "0" + mwcTotalFraction;

    QString mwcTotalWhole = QString::number( mwcSum / qint64(1000000000),10);

    QVector<QString> res(3);
    res[0] = mwcTotalWhole;
    res[1] = mwcTotalFraction;
    if (awaiting>0)
        res[2] = util::trimStrAsDouble( util::nano2one(awaiting), 5);

    return res;
}

void Wallet::requestFaucetMWC() {
    logger::logInfo(logger::BRIDGE, "Call Wallet::requestFaucetMWC");
    getWallet()->requestFaucetMWC();
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
///
///
void Wallet::onConfigUpdate() {
    emit sgnConfigUpdate();
}

void Wallet::onLogin() {
    emit sgnLogin();
}
void Wallet::onLogout() {
    emit sgnLogout();
}

void Wallet::onScanProgress( QString responseId, QJsonObject statusMessage ) {
    emit sgnScanProgress(responseId, statusMessage);
}

void Wallet::onScanDone( QString responseId, bool fullScan, int height, QString errorMessage ) {
    emit sgnScanDone(responseId, fullScan, height, errorMessage);
}

void Wallet::onSend( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag ) {
    Q_UNUSED(amount);
    Q_UNUSED(method);
    Q_UNUSED(dest);

    emit sgnSend(success, error, tx_uuid, tag);
}

void Wallet::onSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message ) {
    emit sgnSlateReceivedFrom(slate, util::nano2one(mwc), fromAddr, message);
}

void Wallet::onScanRewindHash( QString responseId, wallet::ViewWallet walletOutputs, QString errors ) {
    emit sgnScanRewindHash(responseId, walletOutputs.toJson(), errors);
}

void Wallet::onWalletBalanceUpdated() {
    emit sgnWalletBalanceUpdated();
}

void Wallet::onFaucetMWCDone(bool success) {
    emit sgnFaucetMWCDone(success);
}


}
