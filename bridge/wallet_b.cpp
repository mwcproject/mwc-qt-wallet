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
#include "../state/state.h"
#include "../wallet/wallet.h"


namespace bridge {

static wallet::Wallet * getWallet() {
    return state::getStateContext()->wallet;
}

Wallet::Wallet(QObject *parent) : QObject(parent) {

    QObject::connect(notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                     this, &Wallet::onNewNotificationMessage, Qt::QueuedConnection);

    wallet::Wallet *wallet = state::getStateContext()->wallet;

    QObject::connect(wallet, &wallet::Wallet::onConfigUpdate,
                     this, &Wallet::onConfigUpdate, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onListenersStatus,
                     this, &Wallet::onUpdateListenerStatus, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onListeningStartResults,
                     this, &Wallet::onListeningStartResults, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onListeningStopResult,
                     this, &Wallet::onListeningStopResult, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onHttpListeningStatus,
                     this, &Wallet::onHttpListeningStatus, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onNodeStatus,
                     this, &Wallet::onUpdateNodeStatus, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onUpdateSyncProgress,
                     this, &Wallet::onUpdateSyncProgress, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onWalletBalanceUpdated,
                     this, &Wallet::onWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onLoginResult,
                     this, &Wallet::onLoginResult, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onLogout,
                     this, &Wallet::onLogout, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onMwcAddressWithIndex,
                     this, &Wallet::onMwcAddressWithIndex, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onTorAddress,
                     this, &Wallet::onTorAddress, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onOutputs,
                     this, &Wallet::onOutputs, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onTransactions,
                     this, &Wallet::onTransactions, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onCancelTransacton,
                     this, &Wallet::onCancelTransacton, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onTransactionById,
                     this, &Wallet::onTransactionById, Qt::QueuedConnection);


    QObject::connect(wallet, &wallet::Wallet::onExportProof,
                     this, &Wallet::onExportProof, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onVerifyProof,
                     this, &Wallet::onVerifyProof, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onNodeStatus,
                     this, &Wallet::onNodeStatus, Qt::QueuedConnection);

    QObject::connect(wallet, &wallet::Wallet::onAccountCreated,
                     this, &Wallet::onAccountCreated, Qt::QueuedConnection);
    QObject::connect(wallet, &wallet::Wallet::onAccountRenamed,
                     this, &Wallet::onAccountRenamed, Qt::QueuedConnection);
}

Wallet::~Wallet() {}

void Wallet::onNewNotificationMessage(notify::MESSAGE_LEVEL level, QString message) {
    emit sgnNewNotificationMessage(int(level), message);
}

void Wallet::onConfigUpdate() {
    emit sgnConfigUpdate();
}

void Wallet::onListeningStartResults( bool mqTry, bool kbTry, bool tor,
                              QStringList errorMessages, bool initialStart ) {
    Q_UNUSED(mqTry)
    Q_UNUSED(kbTry)
    Q_UNUSED(tor)
    Q_UNUSED(errorMessages)
    Q_UNUSED(initialStart)
    emit sgnListenerStartStop(mqTry, kbTry, tor);
}

void Wallet::onListeningStopResult(bool mqTry, bool kbTry, bool tor,
                           QStringList errorMessages ) {
    Q_UNUSED(mqTry)
    Q_UNUSED(kbTry)
    Q_UNUSED(tor)
    Q_UNUSED(errorMessages)
    emit sgnListenerStartStop(mqTry, kbTry, tor);
}


void Wallet::onUpdateListenerStatus(bool mqsOnline, bool keybaseOnline, bool torOnline) {
    emit sgnUpdateListenerStatus( mqsOnline, keybaseOnline, torOnline );
}

void Wallet::onHttpListeningStatus(bool listening, QString additionalInfo) {
    emit sgnHttpListeningStatus(listening, additionalInfo);
}

void Wallet::onUpdateNodeStatus(bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty,
                                  int connections) {
    emit sgnUpdateNodeStatus(online, errMsg, nodeHeight, peerHeight, double(totalDifficulty), connections);
}

void Wallet::onUpdateSyncProgress(double progressPercent) {
    emit sgnUpdateSyncProgress(progressPercent);
}

void Wallet::onWalletBalanceUpdated() {
    emit sgnWalletBalanceUpdated();
}

void Wallet::onLoginResult(bool ok) {
    emit sgnLoginResult(ok);
}

void Wallet::onLogout() {
    emit sgnLogout();
}

void Wallet::onMwcAddressWithIndex(QString mwcAddress, int idx) {
    emit sgnMwcAddressWithIndex(mwcAddress, idx);
}

void Wallet::onTorAddress(QString tor) {
    emit sgnTorAddress(tor);
}

void Wallet::onOutputs( QString account, bool showSpent, int64_t height, QVector<wallet::WalletOutput> outputs) {
    QVector<QString> outs;
    for (const auto & o : outputs) {
        outs.push_back( o.toJson() );
    }
    emit sgnOutputs( account, showSpent, QString::number(height), outs);
}

void Wallet::onTransactions( QString account, int64_t height, QVector<wallet::WalletTransaction> transactions) {
    QVector<QString> trans;
    for (auto & t : transactions)
        trans.push_back(t.toJson());

    emit sgnTransactions( account, QString::number(height), trans);
}
void Wallet::onCancelTransacton( bool success, QString account, int64_t trIdx, QString errMessage ) {
    emit sgnCancelTransacton(success, account, QString::number(trIdx), errMessage);
}
void Wallet::onTransactionById( bool success, QString account, int64_t height, wallet::WalletTransaction transaction,
                        QVector<wallet::WalletOutput> outputs, QVector<QString> messages ) {
    QVector<QString> outs;
    for (const auto & o : outputs)
        outs.push_back(o.toJson());

    emit sgnTransactionById(success, account, QString::number(height), transaction.toJson(), outs, messages);
}

void Wallet::onExportProof( bool success, QString fn, QString msg ) {
    emit sgnExportProofResult(success, fn, msg);
}
void Wallet::onVerifyProof( bool success, QString fn, QString msg ) {
    emit sgnVerifyProofResult(success, fn, msg);
}

void Wallet::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    emit sgnNodeStatus( online, errMsg, nodeHeight, peerHeight, QString::number(totalDifficulty), connections);
}

void Wallet::onAccountCreated( QString newAccountName) {
    emit sgnAccountCreated(newAccountName);
}
void Wallet::onAccountRenamed(bool success, QString errorMessage) {
    emit sgnAccountRenamed(success, errorMessage);
}


// return true is MQS is online
bool Wallet::getMqsListenerStatus() {
    return getWallet()->getListenerStatus().mqs;
}
// return true is Keybase is online
bool Wallet::getKeybaseListenerStatus()  {
    return getWallet()->getListenerStatus().keybase;
}
// return true if Tor is online
bool Wallet::getTorListenerStatus() {
    return getWallet()->getListenerStatus().tor;
}

// return true is MQS is started
bool Wallet::isMqsListenerStarted() {
    return getWallet()->getListenerStartState().mqs;
}
// return true is Keybase is started
bool Wallet::isKeybaseListenerStarted() {
    return getWallet()->getListenerStartState().keybase;
}
bool Wallet::isTorListenerStarted() {
    return getWallet()->getListenerStartState().tor;
}

// Request start/stop listeners. Feedback should come with sgnUpdateListenerStatus
void Wallet::requestStartMqsListener() {
    getWallet()->listeningStart(true, false, false, false);
}
void Wallet::requestStopMqsListener() {
    getWallet()->listeningStop(true, false, false);
}
void Wallet::requestStartKeybaseListener() {
    getWallet()->listeningStart(false, true, false, false);
}
void Wallet::requestStopKeybaseListener() {
    getWallet()->listeningStop(false, true, false);
}
void Wallet::requestStartTorListener() {
    getWallet()->listeningStart(false, false, true, false);
}
void Wallet::requestStopTorListener() {
    getWallet()->listeningStop(false, false, true);
}

void Wallet::repost(QString account, int id, bool fluff) {
    getWallet()->repost(account, id, fluff);
}

// return values:
// "true"  - listening
// ""  - not listening, no errors
// string  - not listening, error message
QString Wallet::getHttpListeningStatus() {
    QPair<bool, QString> res = getWallet()->getHttpListeningStatus();
    if (res.first)
        return "true";
    return res.second;
}

// Return a password hash for that wallet
QString Wallet::getPasswordHash() {
    return getWallet()->getPasswordHash();
}

// return Total MWC amount as String
QString Wallet::getTotalMwcAmount() {
    QVector<wallet::AccountInfo> balance = getWallet()->getWalletBalance();

    int64_t mwcSum = 0;
    for ( const auto & ai : balance ) {
        mwcSum += ai.total;
    }

    return util::trimStrAsDouble( util::nano2one(mwcSum), 5);
}

// Get MQS address and index
// Return: signal  sgnMwcAddressWithIndex
void Wallet::requestMqsAddress() {
    getWallet()->getMwcBoxAddress();
}

// Change MWC box address to another from the chain. idx - index in the chain.
// Return: signal  sgnMwcAddressWithIndex
void Wallet::requestChangeMqsAddress(int idx) {
    getWallet()->changeMwcBoxAddress(idx);
}

// Generate next box address for the next index
// Return: signal  sgnMwcAddressWithIndex
void Wallet::requestNextMqsAddress() {
    getWallet()->nextBoxAddress();
}

// Get last known MQS address. It is good enough for cases when you don't expect address to be changed
QString Wallet::getMqsAddress() {
    return getWallet()->getMqsAddress();
}

QString Wallet::getTorAddress() {
    return getWallet()->getTorAddress();
}

// Request accounts info
// includeAccountName - return Account names
// includeAccountFullInfo - return account full info
QVector<QString> Wallet::getWalletBalance(bool includeAccountName,  bool includeSpendableInfo, bool includeAccountFullInfo) {
    QVector<QString> result;
    QVector<wallet::AccountInfo> accs = getWallet()->getWalletBalance();
    for (auto & a : accs) {
        if (includeAccountName)
            result.push_back(a.accountName);
        if (includeSpendableInfo)
            result.push_back(a.getSpendableAccountName());
        if (includeAccountFullInfo)
            result.push_back(a.getLongAccountName());
    }

    return result;
}

// Get current account name for the wallet
QString Wallet::getCurrentAccountName() {
    return getWallet()->getCurrentAccountName();
}

// Change current account
void Wallet::switchAccount( QString accountName ) {
    getWallet()->switchAccount(accountName);
}

// Initiate wallet balance update. Please note, update happens in the backgorund and on events.
// When done onWalletBalanceUpdated will be called.
void Wallet::requestWalletBalanceUpdate() {
    getWallet()->updateWalletBalance(false,false,false);
}

// Request list of outputs for the account.
// Respond will be with sgnOutputs
void Wallet::requestOutputs(QString account, bool show_spent, bool enforceSync) {
    getWallet()->getOutputs(account,show_spent, enforceSync);
}

// Show all transactions for current account
// Respond: sgnTransactions( QString account, QString height, QVector<QString> Transactions);
void Wallet::requestTransactions(QString account, bool enforceSync) {
    getWallet()->getTransactions(account, enforceSync);
}

// get Extended info for specific transaction
// Respond:  sgnTransactionById( bool success, QString account, QString height, QString transaction,
//                            QVector<QString> outputs, QVector<QString> messages );
void Wallet::requestTransactionById(QString account, QString txIdx ) {
    getWallet()->getTransactionById(account, txIdx.toLongLong() );
}

// Cancel transaction by id
// Respond: sgnCancelTransacton( bool success, QString trIdx, QString errMessage )
void Wallet::requestCancelTransacton(QString account, QString txIdx) {
    getWallet()->cancelTransacton(account, txIdx.toLongLong());
}

// Set acount to receive the coins
void Wallet::setReceiveAccount(QString account) {
    return getWallet()->setReceiveAccount(account);
}
// Get current account that receive coins
QString Wallet::getReceiveAccount() {
    return getWallet()->getReceiveAccount();
}

// Generating transaction proof for transaction.
// Respond: sgnExportProofResult( bool success, QString fn, QString msg );
void Wallet::generateTransactionProof( QString transactionId, QString resultingFileName ) {
    getWallet()->generateMwcBoxTransactionProof( transactionId.toLongLong(), resultingFileName );
}

// Verify the proof for transaction
// Respond: sgnVerifyProofResult( bool success, QString msg );
void Wallet::verifyTransactionProof( QString proofFileName ) {
    getWallet()->verifyMwcBoxTransactionProof(proofFileName);
}

// Request Node status
// Respond: onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections )
bool Wallet::requestNodeStatus() {
    return getWallet()->getNodeStatus();
}

// Create another account, note no delete exist for accounts
// Check Signal:  sgnAccountCreated
void Wallet::createAccount( QString accountName ) {
    return getWallet()->createAccount(accountName);
}

// Rename account
// Check Signal: sgnAccountRenamed(bool success, QString errorMessage);
void Wallet::renameAccount(QString oldName, QString newName) {
    getWallet()->renameAccount(oldName, newName);
}

}
