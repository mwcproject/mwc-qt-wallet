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

#include "api/MwcWalletApi.h"
#include "core/global.h"
#include "core/WndManager.h"
#include "util/Log.h"
#include "../node/node_client.h"
#include "../../mwc-wallet/mwc_wallet_lib/c_header/mwc_wallet_interface.h"
#include "tasks/StartStopListeners.h"
#include "wallet_macro.h"
#include "bridge/wnd/u_nodeInfo_b.h"
#include "tasks/Scan.h"
#include "tasks/ScanRewindHash.h"
#include "tasks/Send.h"
#include "../node/node_client.h"
#include "core/appcontext.h"
#include "core/WalletApp.h"
#include "tasks/requestFaucet.h"
#include "util/message_mapper.h"

// Problem that we return pointer to a string from the callback. It is mean that we must
// store somewhere an instance. Using buffer for that
static std::string node_client_callback_responses[50];
static QAtomicInt resp_idx(0);

extern "C"
const int8_t* node_client_callback(void* ctx, const int8_t* message)
{
    node::NodeClient * nodeClient = (node::NodeClient *) ctx;
    Q_ASSERT(nodeClient);

    int respIdx = std::abs(resp_idx.fetchAndAddRelaxed(1)) % 50;

    QString request((const char*)message);
    QString response = nodeClient->foreignApiRequest(request);

    node_client_callback_responses[respIdx] = response.toStdString();
    const char * resp = node_client_callback_responses[respIdx].c_str();

    return (const int8_t*) resp;
}

extern "C"
const int8_t* update_status_callback(void* ctx, const int8_t* message)
{
    wallet::Wallet * wallet = (wallet::Wallet *) ctx;
    Q_ASSERT(wallet);

    QString updateMsg((const char*)message);
    logger::logDebug(logger::MWC_WALLET, "Status update : " + updateMsg);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(updateMsg.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError) {
        logger::logError(logger::MWC_WALLET,
            "Unable to parse state update message: " + updateMsg + ".  Error: " + err.errorString() +  " at offset " + QString::number(err.offset));
        return nullptr;
    }
    QJsonObject updateJson = doc.object();

    QString response_id = updateJson["response_id"].toString();
    QJsonObject status = updateJson["status"].toObject();

    wallet->emitStatusUpdate( response_id, status );

    return nullptr;
}

extern "C"
int8_t const * new_tx_callback(void* ctx, const int8_t* msg) {
    wallet::Wallet * wallet = (wallet::Wallet *) ctx;
    Q_ASSERT(wallet);

    // message is a json string with ReceiveData
    QString newTransaction((const char*)msg);
    logger::logDebug(logger::MWC_WALLET, "Receive transaction message: " + newTransaction);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(newTransaction.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError) {
        logger::logError(logger::MWC_WALLET,
            "Unable to parse Receive transaction message: " + newTransaction + ".  Error: " + err.errorString() +  " at offset " + QString::number(err.offset));
        return nullptr;
    }
    QJsonObject newTx = doc.object();

    int context_id = newTx["context_id"].toInt();
    if (wallet->getContextId()!=context_id) {
        logger::logError(logger::MWC_WALLET, "Get new Tx message with invalid context id");
        return nullptr;
    }

    QString tx_uuid = newTx["tx_uuid"].toString();
    QString from = newTx["from"].toString();
    qint64 amount = newTx["amount"].toInteger();
    QString message = newTx["message"].toString();

    wallet->emitSlateReceivedFrom(tx_uuid, amount, from, message );
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
}

bool Wallet::isBusy() const {
    if (restart_listeners.isRunning() || scanOp.isRunning() || sendOp.isRunning() || scanRewindHashOp.isRunning())
        return true;

    if (check_wallet_busy(context_id).response)
        return true;

    return false;
}

bool Wallet::isUpdateInProgress() const {
    return scanOp.isRunning();
}


Wallet::STARTED_MODE Wallet::getStartStatus() {
    return started_state;
}

QString Wallet::init(QString _network, QString _walletDataPath, node::NodeClient * _nodeClient) {
    network = _network;
    walletDataPath = _walletDataPath;

    mwc_api::ApiResponse<int> init_wallet_res = init_wallet(network, walletDataPath);
    if (init_wallet_res.hasError()) {
        return "Unable to create a context. " + init_wallet_res.error;
    }
    Q_ASSERT(context_id<0);
    context_id = init_wallet_res.response;
    logger::logInfo(logger::MWC_WALLET, "new Wallet is initialized for " + network + " at " + walletDataPath + ". Resulting context_id: " + QString::number(context_id));
    Q_ASSERT(context_id>=0);

    nodeClient = _nodeClient;
    Q_ASSERT(nodeClient);

    // registering a callback
    Q_ASSERT(node_client_callback_name.isEmpty());
    node_client_callback_name = "node_client_" + QString::number(context_id);
    register_lib_callback(node_client_callback_name.toStdString().c_str(), node_client_callback, nodeClient);

    Q_ASSERT(update_status_callback_name.isEmpty());
    update_status_callback_name = "scan_status_" + QString::number(context_id);
    register_lib_callback(update_status_callback_name.toStdString().c_str(), update_status_callback, this);

    started_state = STARTED_MODE::OFFLINE;

    return "";
}

#define CANCEL_FUTURE(future) if (future.isValid() && !future.isFinished()) { future.cancel();}

void Wallet::release() {
    started_state = STARTED_MODE::OFFLINE;

    if (context_id>=0) {
        mwc_api::ApiResponse<bool> res = stop_running_scan(context_id);
        LOG_CALL_RESULT("stop_running_scan", "OK" );
    }

    CANCEL_FUTURE(restart_listeners);
    CANCEL_FUTURE(scanOp);
    CANCEL_FUTURE(sendOp);
    CANCEL_FUTURE(scanRewindHashOp);

    restart_listeners.waitForFinished();
    scanOp.waitForFinished();
    sendOp.waitForFinished();
    scanRewindHashOp.waitForFinished();

    if (context_id>=0) {
        if (started_state != STARTED_MODE::OFFLINE) {
            mwc_api::ApiResponse<bool> res = close_wallet(context_id);
            LOG_CALL_RESULT("logout close_wallet", "OK" );
        }

        mwc_api::ApiResponse<bool> res = release_wallet(context_id);
        LOG_CALL_RESULT("logout release_wallet", "OK" );

        context_id = -1;
    }

    mqs_running = false;
    tor_running = false;

    if (!node_client_callback_name.isEmpty()) {
        unregister_lib_callback(node_client_callback_name.toStdString().c_str());
        node_client_callback_name = "";
    }
    if (!update_status_callback_name.isEmpty()) {
        unregister_lib_callback(update_status_callback_name.toStdString().c_str());
        update_status_callback_name = "";
    }
}

// Create new wallet and generate a seed for it
QPair<QStringList, QString> Wallet::start2init(const QString & password, int seedLength) {
    mwc_api::ApiResponse<QStringList> res = create_new_wallet(context_id, node_client_callback_name, seedLength, password );
    LOG_CALL_RESULT("start2init", "get a new seed");

    if (!res.hasError()) {
        started_state = STARTED_MODE::INIT;
    }

    mqs_running = false;
    tor_running = false;
    return QPair<QStringList, QString>(res.response, util::mapMessage(res.error));
}

// Recover the wallet with a mnemonic phrase
// recover wallet with a passphrase. Scan call needs to be done.
// Return: Error
QString Wallet::start2recover(const QStringList & seed, const QString & password) {
    mwc_api::ApiResponse<bool> res = restore_new_wallet(context_id, node_client_callback_name, seed.join(" "), password );
    LOG_CALL_RESULT("start2recover", "OK");

    if (!res.hasError()) {
        started_state = STARTED_MODE::RECOVER;
    }

    mqs_running = false;
    tor_running = false;
    return util::mapMessage(res.error);
}

static bool validateAccPath(const QVector<wallet::Account> & accounts, const QString & accPath) {
    if (accPath.isEmpty())
        return false;

    for (const auto & acc : accounts) {
        if (acc.label.startsWith(mwc::DEL_ACCONT_PREFIX))
            continue;

        if (acc.path == accPath)
            return true;
    }
    return false;
}

// Note, return error include invalid password case as well
// Return: Error
QString Wallet::loginWithPassword(const QString & password, core::AppContext * appContext) {
    mwc_api::ApiResponse<bool> res = open_wallet(context_id, node_client_callback_name, password );
    LOG_CALL_RESULT("loginWithPassword", "OK");

    if (!res.hasError()) {
        started_state = STARTED_MODE::NORMAL;
        emit onLogin();
    }

    // restoring current selected accounts
    int wallet_id = getWalletId();
    QVector<wallet::Account> accounts = listAccounts();

    QString selectedAccPath = appContext->getWalletParam(wallet_id, core::WALLET_PARAM_SELECTED_ACCOUNT_PATH, "");
    if (validateAccPath(accounts, selectedAccPath)) {
        switchAccountById(selectedAccPath);
    }

    QString receiveAccPath = appContext->getWalletParam(wallet_id, core::WALLET_PARAM_RECEIVE_ACCOUNT_PATH, "");
    if (validateAccPath(accounts, receiveAccPath)) {
        setReceiveAccountById(receiveAccPath);
    }

    mqs_running = false;
    tor_running = false;
    return util::mapMessage(res.error);
}

// Return true if wallet has this password. Wallet might not have password (has empty password) if it was created manually.
// Expected that the wallet is already open,
bool Wallet::checkPassword(const QString & password) const {
    if (started_state == STARTED_MODE::OFFLINE)
        return false;

    QPair <bool, QString> passCheck = util::validateMwc713Str(password, true);
    if (!passCheck.first) {
        QThread::msleep(100); // Brute force attack mitigation
        return false;
    }

    mwc_api::ApiResponse<bool> res = validate_password(context_id, password );
    if (!password.isEmpty()) {
        LOG_CALL_RESULT("checkPassword", (res.response ? "True" : "False") );
    }

    if (res.hasError()) {
        QThread::msleep(100); // Brute force attack mitigation
        return false;
    }
    else {
        if (!res.response) {
            QThread::msleep(100); // Brute force attack mitigation
        }
        return res.response;
    }
}

// Exit from the wallet. Expected that state machine will switch to Init state
// Signal:  onLogout
void Wallet::logout() {
    if (getStartStatus() != wallet::Wallet::STARTED_MODE::OFFLINE) {
        emit onLogout();
    }
    release();
}

// Current seed for runnign wallet
QPair<QStringList, QString> Wallet::getSeed(const QString & walletPassword) {
    if (started_state == STARTED_MODE::OFFLINE)
        return QPair<QStringList, QString>({}, "Wallet is offline");

    mwc_api::ApiResponse<QStringList> res = get_mnemonic(context_id, walletPassword );
    LOG_CALL_RESULT("getSeed", "OK" );

    return QPair<QStringList, QString>(res.response, util::mapMessage(res.error));
}

void Wallet::restartRunningListeners() {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    if (! (mqs_running || tor_running)) {
        logger::logError(logger::MWC_WALLET, "restartRunningListeners skipped because no Tor or MQS are running" );
        return; // nothing to restart
    }

    // Restart inb a separate thread
    logger::logError(logger::MWC_WALLET, "restartRunningListeners initiate Tor and MQS restart in the background");
    if (mqs_running)
        nextListenersTask = apply_operation(nextListenersTask, LISTENER_MQS_RESTART );
    if (tor_running)
        nextListenersTask = apply_operation(nextListenersTask, LISTENER_TOR_RESTART );

    startNextStartStopListeners();
}

// Checking if wallet is listening through services
ListenerStatus Wallet::getListenerStatus() {
    ListenerStatus result;

    if (started_state == STARTED_MODE::OFFLINE || context_id<0)
        return result;

    if (tor_running) {
        mwc_api::ApiResponse<ResListenerStatus> res = get_tor_listener_status(context_id);
        //LOG_CALL_RESULT("get_tor_listener_status", "OK" );
        result.tor_healthy = res.response.healthy;
        result.tor_started = res.response.running;
    }

    if (mqs_running) {
        mwc_api::ApiResponse<ResListenerStatus> res = get_mqs_listener_status(context_id);
        //LOG_CALL_RESULT("get_mws_listener_status", "OK" );
        result.mqs_healthy = res.response.healthy;
        result.mqs_started = res.response.running;
    }

    /*logger::logDebug( logger::MWC_WALLET, QString("getListenerStatus response: tor_healthy=") + (result.tor_healthy?"true":"false") +
        " tor_started=" +  (result.tor_started?"true":"false") +
        " mqs_healthy=" +  (result.mqs_healthy?"true":"false") +
        " mqs_started=" +  (result.mqs_started?"true":"false"));*/

    return result;
}

// Start listening through services
void Wallet::listeningStart(bool startMq, bool startTor) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    if (startMq) {
        if (!mqs_running) {
            nextListenersTask = apply_operation(nextListenersTask, LISTENER_MQS_START );
            mqs_running = true;
        }
    }

    if (startTor) {
        if (!tor_running) {
            nextListenersTask = apply_operation(nextListenersTask, LISTENER_TOR_START );
            tor_running = true;
        }
    }

    startNextStartStopListeners();
}

// Stop listening through services
// return:
//    true if the task was schediled and will be executed.
//    false if nothing needs to be done. Everything is stopped
bool Wallet::listeningStop(bool stopMq, bool stopTor) {
    if (started_state == STARTED_MODE::OFFLINE)
        return false;

    if (stopMq) {
        if (mqs_running) {
            nextListenersTask = apply_operation(nextListenersTask, LISTENER_MQS_STOP );
            mqs_running = false;
        }
    }

    if (stopTor) {
        if (tor_running) {
            nextListenersTask = apply_operation(nextListenersTask, LISTENER_TOR_STOP );
            tor_running = false;
        }
    }

    bool needToSchedule = nextListenersTask!=0;
    startNextStartStopListeners();
    return needToSchedule;
}

// Request MQS address
QString Wallet::getMqsAddress() {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QString> res = mqs_address(context_id);
    LOG_CALL_RESULT("getMqsAddress", "OK" );
    return res.response;
}

// Request Tor address
QString Wallet::getTorSlatepackAddress() {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QString> res = tor_address(context_id);
    LOG_CALL_RESULT("tor_address", "OK" );
    return res.response;
}

// requst current address index
int Wallet::getAddressIndex() {
    if (started_state == STARTED_MODE::OFFLINE)
        return 0;

    mwc_api::ApiResponse<int> res = get_address_index(context_id);
    LOG_CALL_RESULT("get_address_index", QString::number(res.response) );
    return res.response;
}

// Note, set address index does update the MQS and Tor addresses
// The Listeners, if running, will be restarted automatically
void Wallet::setAddressIndex(int index) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    mwc_api::ApiResponse<bool> res = set_address_index(context_id, index);
    LOG_CALL_RESULT("get_address_index", "OK" );

    if (mqs_running || tor_running) {
        logger::logInfo(logger::MWC_WALLET, "Address index was changed, restarting listeners...");
        restartRunningListeners();
    }
}

// Get all accounts with balances.
QVector<AccountInfo> Wallet::getWalletBalance(int confirmations, bool filterDeleted, const QStringList & manuallyLockedOutputs) const {
    if (started_state == STARTED_MODE::OFFLINE || context_id<0)
        return {};

    mwc_api::ApiResponse<QVector<Account>> res = list_accounts(context_id);
    LOG_CALL_RESULT("getWalletBalance list_accounts", QString::number(res.response.size()) + " accounts" );

    QVector<AccountInfo> result;
    for (const Account & acc : res.response ) {
        mwc_api::ApiResponse<ResWalletInfo> balance = info(context_id, confirmations, acc.path, manuallyLockedOutputs );
        LOG_CALL_RESULT("getWalletBalance info", QString::number(res.response.size()) + " accounts" );
        if (!balance.hasError()) {
            AccountInfo wi;
            wi.setData( acc.label,
                acc.path,
                balance.response.total.toLongLong() + balance.response.amount_locked.toLongLong(), // total plus locked (by some reasons mwc-wallet doesn't count locked, I think it is wrong)
                balance.response.amount_awaiting_confirmation.toLongLong() + balance.response.amount_awaiting_finalization.toLongLong() +  balance.response.amount_immature.toLongLong(), // awaitingConfirmation
                balance.response.amount_locked.toLongLong(),  // lockedByPrevTransaction
                balance.response.amount_currently_spendable.toLongLong(), // currentlySpendable
                balance.response.last_confirmed_height.toLongLong()); // hegiht
            if (filterDeleted && wi.isDeleted())
                continue;
            result.push_back(wi);
        }
    }

    return result;
}

QVector<Account> Wallet::listAccounts() const {
    if (started_state == STARTED_MODE::OFFLINE)
        return {};

    mwc_api::ApiResponse<QVector<Account>> res = list_accounts(context_id);
    LOG_CALL_RESULT("getWalletBalance list_accounts", QString::number(res.response.size()) + " accounts" );
    return res.response;
}


QString Wallet::getCurrentAccountId() {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QString> res = current_account(context_id);
    LOG_CALL_RESULT("current_account", res.response );

    return res.response;
}

// Create another account, note no delete exist for accounts
QString Wallet::createAccount( const QString & accountName ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QString> res = create_account(context_id, accountName);
    LOG_CALL_RESULT("create_account", res.response );

    return res.response;
}

// Switch to different account
void Wallet::switchAccountById(const QString & accountPath) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    mwc_api::ApiResponse<bool> res = switch_account(context_id, accountPath);
    LOG_CALL_RESULT("switch_account", "OK" );
}

// Rename account
void Wallet::renameAccountById( const QString & accountPath, const QString & newAccountName) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    mwc_api::ApiResponse<bool> res = rename_account(context_id, accountPath, newAccountName);
    LOG_CALL_RESULT("rename_account", "OK" );
}

// Check and repair the wallet. Will take a while
// Check Signal: onScanProgress, onScanDone
// Return responseId
QString Wallet::scan(bool delete_unconfirmed) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    if (scanOp.isRunning())
        return lastScanResponseId;

    int id = response_id_counter.fetchAndAddRelaxed(1);
    QString responseId = "scan_" + QString::number(id);

    // Scan started in any case
    lastScanResponseId = responseId;

    scanOp = startScan(this, update_status_callback_name, responseId, true, delete_unconfirmed);
    return responseId;
}

// Update the wallet state, resync with a current node state
// Check Signal: onScanProgress, onScanDone
// Return responseId
QString Wallet::update_wallet_state() {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    if (scanOp.isRunning())
        return lastScanResponseId;

    int id = response_id_counter.fetchAndAddRelaxed(1);
    QString responseId = "upd_" + QString::number(id);

    lastScanResponseId = responseId;

    // Scan started in any case
    scanOp = startScan(this, update_status_callback_name, responseId, false, false);
    return responseId;
}

// Get current configuration of the wallet.
WalletConfig  Wallet::getWalletConfig() {
    WalletConfig config;
    config.setData(network, walletDataPath);
    return config;
}

void Wallet::setReceiveAccountById(const QString & accountPath) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    mwc_api::ApiResponse<bool> res = switch_receive_account(context_id, accountPath);
    LOG_CALL_RESULT("setReceiveAccount", "OK" );
}

QString Wallet::getReceiveAccountPath() {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QString> res = receive_account(context_id);
    LOG_CALL_RESULT("setReceiveAccount", res.response );
    return res.response;
}

// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
// Return error or JsonObject
QString Wallet::generateTransactionProof( const QString & transactionUuid ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QJsonObject> res = get_proof(context_id, transactionUuid);
    LOG_CALL_RESULT("get_proof", "OK" );

    if (res.hasError())
        return util::mapMessage(res.error);
    else
        return QJsonDocument(res.response).toJson(QJsonDocument::Compact);
}

// Verify the proof for transaction
// Return error or JsonObject
QString Wallet::verifyTransactionProof(const QString & proof) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QJsonObject> res = verify_proof(context_id, proof);
    LOG_CALL_RESULT("verify_proof", "OK" );

    if (res.hasError())
        return util::mapMessage(res.error);
    else
        return QJsonDocument(res.response).toJson(QJsonDocument::Compact);
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
    if (started_state == STARTED_MODE::OFFLINE)
        return false;

    if (sendOp.isRunning()) {
        logger::logInfo(logger::QT_WALLET, "Send task is already running, can't start another one");
        return false;
    }

    sendOp = send(this, accountPathFrom, responseTag, amount, amount_includes_fee,
                    message, minimum_confirmations, selection_strategy, method,
                    dest, generate_proof, change_outputs, fluff, ttl_blocks,
                    exclude_change_outputs, outputs,
                    late_lock, min_fee);
    return true;
}

// Receive slatepack. Will generate the resulting slatepack.
// Return: <SlatePack, Error>
// In case of error SlatePack is empty
QPair<ResReceive,QString> Wallet::receiveSlatepack( QString slatePack, QString description) {
    if (started_state == STARTED_MODE::OFFLINE)
        return QPair<ResReceive,QString>(ResReceive(), "Wallet is offline");

    mwc_api::ApiResponse<ResReceive> res = receive(context_id, slatePack, description, "");
    LOG_CALL_RESULT("receive", "OK" );
    return QPair<ResReceive,QString>(res.response, util::mapMessage(res.error));
}

// finalize SP transaction and broadcast it
// Return error message
QString Wallet::finalizeSlatepack( const QString & slatepack, bool fluff, bool nopost ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "Wallet is offline";

    mwc_api::ApiResponse<bool> res = finalize(context_id, slatepack, fluff, nopost);
    LOG_CALL_RESULT("finalize", "OK" );
    return util::mapMessage(res.error);
}

// submit finalized transaction. Make sense for cold storage => online node operation
// Return Error message
QString Wallet::submitFile( QString fileTx, bool fluff ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "Wallet is offline";

    mwc_api::ApiResponse<bool> res = post(context_id, fileTx,
             fluff);
    LOG_CALL_RESULT("post", "OK" );
    return util::mapMessage(res.error);
}

// Show outputs for the wallet
QVector<WalletOutput> Wallet::getOutputs(const QString & accountPath, bool show_spent) {
    if (started_state == STARTED_MODE::OFFLINE)
        return {};

    mwc_api::ApiResponse<QVector<WalletOutput>> res = outputs(context_id, accountPath, show_spent );
    LOG_CALL_RESULT("outputs", QString::number(res.response.size()) +  " output" );
    return res.response;
}

// Show all transactions for current account
QVector<WalletTransaction> Wallet::getTransactions( const QString & accountPath ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return {};

    mwc_api::ApiResponse<QVector<WalletTransaction>> res = transactions(context_id, accountPath);
    LOG_CALL_RESULT("transactions", QString::number(res.response.size()) +  " transactions" );
    return res.response;
}

WalletTransaction Wallet::getTransactionByUUID( const QString & tx_uuid ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return WalletTransaction();

    mwc_api::ApiResponse<WalletTransaction> res = transaction_by_uuid(context_id, tx_uuid);
    LOG_CALL_RESULT("transactions", "OK" );
    return res.response;
}

// Cancelt TX by UUID (in case of multi accouns, we want to cancel both)
// Return erro string
QString  Wallet::cancelTransacton(const QString & txUUID) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "Wallet is offline";

    mwc_api::ApiResponse<bool> res = cancel(context_id, txUUID);
    LOG_CALL_RESULT("cancel", "OK" );
    emitWalletBalanceUpdated();
    return util::mapMessage(res.error);
}

// True if transaction was finalized and can be reposted
bool Wallet::hasFinalizedData(const QString & txUUID) {
    if (started_state == STARTED_MODE::OFFLINE)
        return false;

    mwc_api::ApiResponse<bool> res = has_finalized_data(context_id, txUUID);
    LOG_CALL_RESULT("has_finalized_data", "OK" );
    return res.response;
}


// Repost the transaction.
// Retunr Error
QString Wallet::repostTransaction(const QString & txUUID, bool fluff) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "Wallet is offline";

    mwc_api::ApiResponse<bool> res = repost(context_id, txUUID, fluff);
    LOG_CALL_RESULT("repost", "OK" );
    return util::mapMessage(res.error);
}

// Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
DecodedSlatepack Wallet::decodeSlatepack(const QString & slatepackContent) {
    if (started_state == STARTED_MODE::OFFLINE) {
        DecodedSlatepack res;
        res.error = "Wallet is offline";
        return res;
    }

    DecodedSlatepack res = decode_slatepack(context_id, slatepackContent );
    if (!res.error.isEmpty())
        logger::logError(logger::MWC_WALLET, QString("decode_slatepack for id ") + QString::number(context_id) + " is failed: " + res.error);
    else
        logger::logInfo(logger::MWC_WALLET, QString("decode_slatepack for id ") + QString::number(context_id) + ", is OK");
    return res;
}

// Request rewind hash
QString Wallet::viewRewindHash() {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    mwc_api::ApiResponse<QString> res = rewind_hash(context_id);
    LOG_CALL_RESULT("rewind_hash", "OK" );
    return res.response;
}

// Scan with revind hash. That will generate bunch or messages similar to scan
// Check Signal: onScanProgress
// Check Signal: onScanRewindHash( QString responseId, ViewWallet walletOutputs, QString errors );
// Return responseId
QString Wallet::scanRewindHash( const QString & rewindHash ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return "";

    if (scanRewindHashOp.isRunning())
        return "";

    int id = response_id_counter.fetchAndAddRelaxed(1);
    QString responseId = "rw_h_" + QString::number(id);

    // Scan started in any case
    scanRewindHashOp = wallet::scanRewindHash(this, rewindHash,
                update_status_callback_name, responseId );
    return responseId;
}

// Generate ownership proof
QJsonObject Wallet::generateOwnershipProof(const QString & message, bool includeRewindHash, bool includeTorAddress, bool includeMqsAddress ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return QJsonObject();

    mwc_api::ApiResponse<QJsonObject> res = generate_ownership_proof(context_id, message,
        includeRewindHash, includeTorAddress, includeMqsAddress );
    LOG_CALL_RESULT("generate_ownership_proof", "OK" );
    return res.response;
}

// Validate ownership proof
OwnershipProofValidation Wallet::validateOwnershipProof(const QJsonObject & proof) {
    if (started_state == STARTED_MODE::OFFLINE) {
        OwnershipProofValidation res;
        res.error = "Wallet is offline";
        return res;
    }

    OwnershipProofValidation res = validate_ownership_proof(context_id, proof);
    if (!res.error.isEmpty())
        logger::logError(logger::MWC_WALLET, QString("validate_ownership_proof for id ") + QString::number(context_id) + " is failed: " + res.error);
    else
        logger::logInfo(logger::MWC_WALLET, QString("validate_ownership_proof for id ") + QString::number(context_id) + ", is OK");

    return res;
}

// Sync long call, wallet will process QT events and show Success/Error Dlg
bool Wallet::requestFaucetMWC() {
    if (started_state == STARTED_MODE::OFFLINE)
        return false;

    // requesting 3 MWC
    QPair<bool, qint64> amount = util::one2nano("3.0");
    Q_ASSERT(amount.first);

    QFuture<QPair<bool, QString>> rsp = wallet::requestMwcFromFlooFaucet(this, amount.second);

    while (!rsp.isFinished()) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 50);
        QThread::msleep(10);
    }

    QPair<bool, QString> res = rsp.result();
    if (res.first) {
        core::getWndManager()->messageTextDlg("Faucet request", "Your faucet request for 3 MWC was submitted successfully.\nThe funds will be spendable after the transaction is confirmed on the Floonet blockchain.\n\nExplorer: https://explorer.floonet.mwc.mw/");
        return true;
    }
    else {
        core::getWndManager()->messageTextDlg("Request failed", "Yout faucet request was failed with error:\n" + res.second + "\n\nIf faucet is not running, please notify devs about that.");
        return false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Wallet::reportWalletFatalError( QString message ) const {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    core::getWndManager()->messageTextDlg("MWC Wallet Error", message);
    mwc::closeApplication();
}

void Wallet::startNextStartStopListeners() {
    if (restart_listeners.isRunning())
        return;

    if (started_state == STARTED_MODE::OFFLINE)
        return;

    if (context_id>0 && nextListenersTask!=0) {
        restart_listeners = startStopListeners(this, nextListenersTask, torStarter);
        nextListenersTask = 0;
    }
}


void Wallet::startStopListenersDone(int operation) {
    if (core::WalletApp::isExiting())
        return;

    emit onStartStopListenersDone(operation);

    restart_listeners = QFuture<void>();

    if (started_state == STARTED_MODE::OFFLINE)
        return;

    startNextStartStopListeners();
}

void Wallet::scanDone(QString responseId, bool fullScan, int height, QString errorMessage ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    emit onScanDone( responseId, fullScan, height, errorMessage );

    if (height>0 && height != lastTopHeight) {
        lastTopHeight = height;
    }
}

void Wallet::emitStatusUpdate( const QString & response_id, const QJsonObject & status ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    emit  onScanProgress( response_id, status );
}

void Wallet::sendDone( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    emit onSend(success, util::mapMessage(error), tx_uuid, amount, method, dest, tag);
}

void Wallet::scanRewindDone( const QString & responseId, const ViewWallet & result, const QString & error ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    emit onScanRewindHash( responseId, result, util::mapMessage(error) );
}

void Wallet::emitSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message ) {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    emit onSlateReceivedFrom(slate, mwc, fromAddr, message );
    emit onWalletBalanceUpdated();
}

// Account info is updated
void Wallet::emitWalletBalanceUpdated() {
    if (started_state == STARTED_MODE::OFFLINE)
        return;

    emit onWalletBalanceUpdated();
}

}
