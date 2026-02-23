// Copyright 2019 The MWC Developers
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

#include "wallet/wallet.h"

#include <qjsondocument.h>
#include <QMetaObject>
#include <QtConcurrent>

#include "api/MwcWalletApi.h"
#include "core/global.h"
#include "core/WndManager.h"
#include "util/Log.h"
#include "../node/node_client.h"
#include "../../mwc-wallet/mwc_wallet_lib/c_header/mwc_wallet_interface.h"
#include "tasks/StartStopListeners.h"
#include "wallet_macro.h"
#include "tasks/Scan.h"
#include "tasks/ScanRewindHash.h"
#include "tasks/Send.h"
#include "core/appcontext.h"
#include "core/WalletApp.h"
#include "util/message_mapper.h"

extern "C"
int8_t const * new_tx_callback(void* ctx, const int8_t* msg) {
    wallet::Wallet * wallet = (wallet::Wallet *) ctx;
    Q_ASSERT(wallet);

    // message is a json string with ReceiveData
    QString newTransaction((const char*)msg);
    QMetaObject::invokeMethod(wallet,
        [wallet, newTransaction]() {
            logger::logDebug(logger::MWC_WALLET, "Receive transaction message: " + newTransaction);

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(newTransaction.toUtf8(), &err);

            if (err.error != QJsonParseError::NoError) {
                logger::logError(logger::MWC_WALLET,
                    "Unable to parse Receive transaction message: " + newTransaction + ".  Error: " + err.errorString() +  " at offset " + QString::number(err.offset));
                return;
            }
            QJsonObject newTx = doc.object();

            int context_id = newTx["context_id"].toInt();
            if (wallet->getContextId()!=context_id) {
                logger::logError(logger::MWC_WALLET, "Get new Tx message with invalid context id");
                return;
            }

            QString tx_uuid = newTx["tx_uuid"].toString();
            QString from = newTx["from"].toString();
            qint64 amount = newTx["amount"].toInteger();
            QString message = newTx["message"].toString();

            wallet->emitSlateReceivedFrom(tx_uuid, amount, from, message );
        },
        Qt::QueuedConnection);
    return nullptr;
}

namespace wallet {

//////////////////////////////////////////////////////////////////
//  Wallet
Wallet::Wallet(QFuture<QString> * _torStarter)
{
    torStarter = _torStarter;

    // Note, expected single instance of Wallet at the same time. If not, newTxUpdate need to be generated
    set_receive_tx_callback( "newTxUpdate", new_tx_callback, this );
}

Wallet::~Wallet()
{
    // There is a
    clean_receive_tx_callback("newTxUpdate");
    release();

    // Waiting for all started before tasks to finish
    for (auto & tasks : releaseTasks.values()) {
        tasks.waitForFinished();
    }
}

bool Wallet::isBusy() const {
    if (internals==nullptr)
        return false;

    return internals->isBusy();
}

QString Wallet::init(QString network, QString walletDataPath,
    std::shared_ptr<node::NodeClient> nodeClient) {

    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    Q_ASSERT(internals==nullptr);

    if (internals!=nullptr)
        return "Internal error, wallet is not released!";

    // The same wallet might be still in the process ot release
    QFuture<void> pendingTask = releaseTasks.take(walletDataPath);
    pendingTask.waitForFinished();

    QString errMsg;
    internals = new WalletInternals(this, torStarter, network, walletDataPath, nodeClient, errMsg);

    if (!errMsg.isEmpty()) {
        delete internals;
        internals = nullptr;
    }

    return errMsg;
}

void Wallet::release() {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    if (internals==nullptr)
        return;

    internals->detach();
    QString path = getWalletDataPath();
    WalletInternals * internals2 = internals;
    QFuture<void> releaseWalletTask = QtConcurrent::run( [internals2]() -> void {
        delete internals2;
    });
    releaseTasks.insert( path, releaseWalletTask);
    internals = nullptr;
}

// Create new wallet and generate a seed for it
QPair<QStringList, QString> Wallet::start2init(const QString & password, int seedLength) {
    Q_ASSERT(internals);
    if (internals==nullptr) {
        return QPair<QStringList, QString>(QStringList(), "Internal Error. Wallet is not initialized!");
    }
    return internals->start2init(password, seedLength);
}

// Recover the wallet with a mnemonic phrase
// recover wallet with a passphrase. Scan call needs to be done.
// Return: Error
QString Wallet::start2recover(const QStringList & seed, const QString & password) {
    Q_ASSERT(internals);
    if (internals==nullptr) {
        return "Internal Error. Wallet is not initialized!";
    }
    return internals->start2recover(seed, password);
}

// Note, return error include invalid password case as well
// Return: Error
QString Wallet::loginWithPassword(const QString & password) {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    Q_ASSERT(internals);
    if (internals==nullptr) {
        return "Internal Error. Wallet is not initialized!";
    }

    QString errMsg = internals->loginWithPassword(password);
    if (internals->isOpen()) {
        emit onLogin();
    }
    return errMsg;
}

// Return true if wallet has this password. Wallet might not have password (has empty password) if it was created manually.
// Expected that the wallet is already open,
bool Wallet::checkPassword(const QString & password) const {
    if (internals==nullptr || internals->context_id<0)
        return false;

    QPair <bool, QString> passCheck = util::validateMwc713Str(password, true);
    if (!passCheck.first) {
        QThread::msleep(100); // Brute force attack mitigation
        return false;
    }

    return internals->checkPassword(password);
}

// Exit from the wallet. Expected that state machine will switch to Init state
// Signal:  onLogout
void Wallet::logout() {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    if (isInit()) {
        emit onLogout();
    }
    release();
}

// Current seed for runnign wallet
QPair<QStringList, QString> Wallet::getSeed(const QString & walletPassword) {
    if (!isInit())
        return QPair<QStringList, QString>({}, "Wallet is offline");

    return internals->getSeed(walletPassword);
}

// Checking if wallet is listening through services
ListenerStatus Wallet::getListenerStatus() {
    if (!isInit())
        return ListenerStatus();

    return internals->getListenerStatus();
}

// Start listening through services
void Wallet::listeningStart(bool startMq, bool startTor) {
    if (!isInit())
        return;

    internals->listeningStart(startMq, startTor);
}

// Stop listening through services
// return:
//    true if the task was schediled and will be executed.
//    false if nothing needs to be done. Everything is stopped
bool Wallet::listeningStop(bool stopMq, bool stopTor) {
    if (!isInit())
        return false;

    return internals->listeningStop(stopMq, stopTor);
}

// Request MQS address
QString Wallet::getMqsAddress() const {
    if (!isInit())
        return "";

    return internals->getMqsAddress();
}

// Request Tor address
QString Wallet::getTorSlatepackAddress() const {
    if (!isInit())
        return "";

    return internals->getTorSlatepackAddress();
}

// requst current address index
int Wallet::getAddressIndex() const {
    if (!isInit())
        return 0;

    return internals->getAddressIndex();
}

// Note, set address index does update the MQS and Tor addresses
// The Listeners, if running, will be restarted automatically
void Wallet::setAddressIndex(int index) {
    if (!isInit())
        return;

    internals->setAddressIndex(index);
}

// Get all accounts with balances.
QVector<AccountInfo> Wallet::getWalletBalance(int confirmations, bool filterDeleted, const QStringList & manuallyLockedOutputs) const {
    if (!isInit())
        return {};

    return internals->getWalletBalance(confirmations, filterDeleted, manuallyLockedOutputs);
}

QVector<Account> Wallet::listAccounts() const {
    if (!isInit())
        return {};

    return internals->listAccounts();
}


QString Wallet::getCurrentAccountId() const {
    if (!isInit())
        return "";

    return internals->getCurrentAccountId();
}

// Create another account, note no delete exist for accounts
QString Wallet::createAccount( const QString & accountName ) {
    if (!isInit())
        return "";

    return internals->createAccount( accountName );
}

// Switch to different account
void Wallet::switchAccountById(const QString & accountPath) {
    if (!isInit())
        return;

    internals->switchAccountById(accountPath);
}

// Rename account
void Wallet::renameAccountById( const QString & accountPath, const QString & newAccountName) {
    if (!isInit())
        return;

    internals->renameAccountById( accountPath, newAccountName);
}

// Check and repair the wallet. Will take a while
// Check Signal: onScanStart, onScanProgress, onScanDone
// Return responseId
QString Wallet::scan(bool delete_unconfirmed, bool fullScan) {
    if (!isInit())
        return "";

    QString responseId = internals->scan(delete_unconfirmed, fullScan);
    emit onScanStart(responseId, fullScan);
    return responseId;
}

// Get current configuration of the wallet.
WalletConfig  Wallet::getWalletConfig() const {
    WalletConfig config;
    if (internals) {
        config.setData(internals->network, internals->walletDataPath);
    }
    return config;
}

void Wallet::setReceiveAccountById(const QString & accountPath) {
    if (!isInit())
        return;

    internals->setReceiveAccountById(accountPath);
}

QString Wallet::getReceiveAccountPath() {
    if (!isInit())
        return "";

    return internals->getReceiveAccountPath();
}

// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
// Return error or JsonObject
QString Wallet::generateTransactionProof( const QString & transactionUuid ) {
    if (!isInit())
        return "";

    return internals->generateTransactionProof( transactionUuid );
}

// Verify the proof for transaction
// Return error or JsonObject
QString Wallet::verifyTransactionProof(const QString & proof) {
    if (!isInit())
        return "";

    return internals->verifyTransactionProof(proof);
}

// Send some coins to address. Return false if another send is already in progress. Need to wait more, one send at a time
// Before send, wallet always do the switch to account to make it active
// Check signal:  onSend
bool Wallet::sendTo( const QString &accountPathFrom,
                const QString & responseTag,
                qint64 amount, //  -1  - mean All
                bool amount_includes_fee,
                const QString & message, // can be empty, means None
                int minimum_confirmations,
                const QString & selection_strategy, //  Values: all, smallest. Default: Smallest
                const QString & method,  // Values:  http, file, slatepack, self, mwcmqs
                const QString & dest, // Values depends on 'method' Send the transaction to the provided server (start with http://), destination for encrypting/proofs. For method self, dest can point to account name to move
                bool generate_proof,
                int change_outputs,
                bool fluff,
                int ttl_blocks, // pass -1 to skip
                bool exclude_change_outputs,
                const QStringList & outputs, // Outputs to use. If None, all outputs can be used
                bool late_lock,
                qint64 min_fee)  // 0 or negative to skip. Currently no needs to define it
{
    if (!isInit())
        return false;

    return internals->sendTo(accountPathFrom,responseTag,amount,amount_includes_fee,
                message, minimum_confirmations, selection_strategy, method, dest, generate_proof,
                change_outputs, fluff, ttl_blocks, exclude_change_outputs, outputs, late_lock, min_fee);
}

// Receive slatepack. Will generate the resulting slatepack.
// Return: <SlatePack, Error>
// In case of error SlatePack is empty
QPair<ResReceive,QString> Wallet::receiveSlatepack( QString slatePack, QString description) {
    if (!isInit())
        return QPair<ResReceive,QString>(ResReceive(), "Wallet is offline");

    return internals->receiveSlatepack( slatePack, description);
}

// finalize SP transaction and broadcast it
// Return error message
QString Wallet::finalizeSlatepack( const QString & slatepack, bool fluff, bool nopost ) {
    if (!isInit())
        return "Wallet is offline";

    return internals->finalizeSlatepack( slatepack, fluff, nopost );
}

// submit finalized transaction. Make sense for cold storage => online node operation
// Return Error message
QString Wallet::submitFile( QString fileTx, bool fluff ) {
    if (!isInit())
        return "Wallet is offline";

    return internals->submitFile( fileTx, fluff );
}

// Show outputs for the wallet
QVector<WalletOutput> Wallet::getOutputs(const QString & accountPath, bool show_spent) const {
    if (!isInit())
        return {};

    return internals->getOutputs(accountPath, show_spent);
}

// Show all transactions for current account
QVector<WalletTransaction> Wallet::getTransactions( const QString & accountPath ) const {
    if (!isInit())
        return {};

    return internals->getTransactions( accountPath );
}

WalletTransaction Wallet::getTransactionByUUID( const QString & tx_uuid ) const {
    if (!isInit())
        return WalletTransaction();

    return internals->getTransactionByUUID(tx_uuid);
}

// Cancelt TX by UUID (in case of multi accounts, we want to cancel both)
// Return error string
QString  Wallet::cancelTransacton(const QString & txUUID) {
    if (!isInit())
        return "Wallet is offline";

    QString errMsg = internals->cancelTransacton(txUUID);
    emitWalletBalanceUpdated();
    return errMsg;
}

// True if transaction was finalized and can be reposted
bool Wallet::hasFinalizedData(const QString & txUUID) const {
    if (!isInit())
        return false;

    return internals->hasFinalizedData(txUUID);
}


// Repost the transaction.
// Retunr Error
QString Wallet::repostTransaction(const QString & txUUID, bool fluff) {
    if (!isInit())
        return "Wallet is offline";

    return internals->repostTransaction(txUUID, fluff);
}

// Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
DecodedSlatepack Wallet::decodeSlatepack(const QString & slatepackContent) const {
    if (!isInit()) {
        DecodedSlatepack res;
        res.error = "Wallet is offline";
        return res;
    }

    return internals->decodeSlatepack(slatepackContent);
}

// Request rewind hash
QString Wallet::viewRewindHash() const {
    if (!isInit())
        return "";

    return internals->viewRewindHash();
}

// Scan with revind hash. That will generate bunch or messages similar to scan
// Check Signal: onScanProgress
// Check Signal: onScanRewindHash( QString responseId, ViewWallet walletOutputs, QString errors );
// Return responseId
QString Wallet::scanRewindHash( const QString & rewindHash ) {
    if (!isInit())
        return "";

    return internals->scanRewindHash(rewindHash);
}

// Generate ownership proof
QJsonObject Wallet::generateOwnershipProof(const QString & message, bool includeRewindHash, bool includeTorAddress, bool includeMqsAddress ) {
    if (!isInit())
        return QJsonObject();

    return internals->generateOwnershipProof(message, includeRewindHash, includeTorAddress, includeMqsAddress );
}

// Validate ownership proof
OwnershipProofValidation Wallet::validateOwnershipProof(const QJsonObject & proof) {
    if (!isInit())
    {
        OwnershipProofValidation res;
        res.error = "Wallet is offline";
        return res;
    }

    return internals->validateOwnershipProof(proof);
}

// Sync long call, wallet will process QT events and show Success/Error Dlg
void Wallet::requestFaucetMWC() {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    if (!isInit()) {
        mwcFromFlooFaucetDone(false, "");
        return;
    }

    if (internals!=nullptr) {
        internals->requestFaucetMWC();
    }
}

void Wallet::mwcFromFlooFaucetDone(bool success, QString errorMsg) {
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    emit onFaucetMWCDone(success);

    if (success) {
        core::getWndManager()->messageTextDlg("Faucet request", "Floonet faucet request for 3 MWC was submitted successfully.\nThe funds will be spendable after the transaction is confirmed on the Floonet blockchain.\n\nExplorer: https://explorer.floonet.mwc.mw/");
    }
    else {
        if (!errorMsg.isEmpty()) {
            core::getWndManager()->messageTextDlg("Request failed", "Floonet faucet request was failed with error:\n" + errorMsg + "\n\nIf faucet is not running, please notify devs about that.");
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Wallet::reportWalletFatalError( QString message ) const {
    if (!isInit())
        return;

    core::getWndManager()->messageTextDlg("MWC Wallet Error", message);
    mwc::closeApplication();
}

void Wallet::startStopListenersDone(int operation) {
    if (core::WalletApp::isExiting())
        return;

    emit onStartStopListenersDone(operation);
}

void Wallet::scanDone(QString responseId, bool fullScan, int height, QString errorMessage ) {
    if (!isInit())
        return;
    emit onScanDone( responseId, fullScan, height, errorMessage );

}

void Wallet::emitStatusUpdate( const QString & response_id, const QJsonObject & status ) {
    emit  onScanProgress( response_id, status );
}

void Wallet::sendDone( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag ) {
    emit onSend(success, util::mapMessage(error), tx_uuid, amount, method, dest, tag);
}

void Wallet::scanRewindDone( const QString & responseId, const ViewWallet & result, const QString & error ) {
    emit onScanRewindHash( responseId, result, util::mapMessage(error) );
}

void Wallet::emitSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message ) {
    emit onSlateReceivedFrom(slate, mwc, fromAddr, message );
    emit onWalletBalanceUpdated();
}

// Account info is updated
void Wallet::emitWalletBalanceUpdated() {
    emit onWalletBalanceUpdated();
}

int Wallet::getWalletId() {
    if (internals==nullptr)
        return 0;
    return internals->getWalletId();
}

bool Wallet::isInit() const {
    if (internals==nullptr)
        return false;
    return internals->context_id>=0 && internals->isOpen();
}

QString Wallet::getWalletDataPath() const {
    if (internals==nullptr)
        return "";
    return internals->walletDataPath;
}

std::shared_ptr<node::NodeClient> Wallet::getNodeClient() const {
    if (internals==nullptr)
        return nullptr;
    return internals->nodeClient;
}

NodeStatus Wallet::requestNodeStatus() const {
    if (internals==nullptr)
        return NodeStatus();

    return internals->requestNodeStatus();
}


bool Wallet::isNodeHealthy() const {
    if (internals==nullptr)
        return false;

    return internals->isNodeHealthy();
}

// True if the node is loaded and can process any requests. Return folse for cold wallet starting process
bool Wallet::isNodeAlive() const {
    if (internals==nullptr)
        return false;

    return internals->isNodeAlive();
}

bool Wallet::isUsePublicNode() const {
    if (internals==nullptr)
        return true;

    return internals->isUsePublicNode();
}

qint64 Wallet::getLastNodeHeight() const {
    if (internals==nullptr)
        return false;

    return internals->getLastNodeHeight();
}

QString Wallet::getLastInternalNodeState() const {
    if (internals==nullptr)
        return "Not initialized";

    return internals->getLastInternalNodeState();
}


int Wallet::getContextId() const {
    if (internals==nullptr)
        return -1;
    return internals->context_id;
}

bool Wallet::isMqsRunning() const {
    if (internals==nullptr)
        return false;
    return internals->mqs_running;
}

bool Wallet::isTorRunning() const {
    if (internals==nullptr)
        return false;

    return internals->tor_running;
}


}
