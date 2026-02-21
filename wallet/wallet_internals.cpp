// Copyright 2026 The MWC Developers
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

#include "wallet_internals.h"

#include <QCoreApplication>
#include <qjsondocument.h>

#include "wallet.h"
#include "wallet_macro.h"
#include "../../mwc-wallet/mwc_wallet_lib/c_header/mwc_wallet_interface.h"

#include "api/MwcWalletApi.h"
#include "tasks/StartStopListeners.h"
#include "../state/state.h"
#include "../core/appcontext.h"
#include "core/global.h"
#include "node/MwcNode.h"
#include "node/node_client.h"
#include "tasks/requestFaucet.h"
#include "tasks/Scan.h"
#include "tasks/ScanRewindHash.h"
#include "tasks/Send.h"
#include "util/Log.h"
#include "util/message_mapper.h"


const int CALLBACK_SLOTS = 1000;

// Problem that we return pointer to a string from the callback. It is mean that we must
// store somewhere an instance. Using buffer for that
static std::string node_client_callback_responses[CALLBACK_SLOTS];
static QAtomicInt resp_idx(0);

extern "C"
const int8_t* node_client_callback(void* ctx, const int8_t* message)
{
    node::NodeClient * nodeClient = (node::NodeClient *) ctx;
    Q_ASSERT(nodeClient);

    int respIdx = std::abs(resp_idx.fetchAndAddRelaxed(1)) % CALLBACK_SLOTS;

    QString request((const char*)message);
    QString response = nodeClient->foreignApiRequest(request);

    node_client_callback_responses[respIdx] = response.toStdString();
    const char * resp = node_client_callback_responses[respIdx].c_str();

    return (const int8_t*) resp;
}

extern "C"
const int8_t* update_status_callback(void* ctx, const int8_t* message)
{
    wallet::WalletInternals * internals = (wallet::WalletInternals *) ctx;
    Q_ASSERT(internals);

    QString updateMsg((const char*)message);
    QMetaObject::invokeMethod(internals,
        [internals, updateMsg]() {
            logger::logDebug(logger::MWC_WALLET, "Status update : " + updateMsg);

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(updateMsg.toUtf8(), &err);

            if (err.error != QJsonParseError::NoError) {
                logger::logError(logger::MWC_WALLET,
                    "Unable to parse state update message: " + updateMsg + ".  Error: " + err.errorString() +  " at offset " + QString::number(err.offset));
                return;
            }
            QJsonObject updateJson = doc.object();

            QString response_id = updateJson["response_id"].toString();
            QJsonObject status = updateJson["status"].toObject();

            internals->emitStatusUpdate( response_id, status );
        },
        Qt::QueuedConnection);

    return nullptr;
}



namespace wallet {
    WalletInternals::WalletInternals(Wallet * _parentWallet, QFuture<QString> * _torStarter, QString _network, QString _walletDataPath,
                 std::shared_ptr<node::NodeClient> _nodeClient,
                QString & errMessage ) : QObject(_parentWallet)
    {
        parentWallet = _parentWallet;
        Q_ASSERT(parentWallet!=nullptr);

        torStarter = _torStarter;

        network = _network;
        walletDataPath = _walletDataPath;

        mwc_api::ApiResponse<int> init_wallet_res = init_wallet(network, walletDataPath);
        if (init_wallet_res.hasError()) {
            errMessage = "Unable to create a context. " + init_wallet_res.error;
            return;
        }
        Q_ASSERT(context_id<0);
        context_id = init_wallet_res.response;
        logger::logInfo(logger::MWC_WALLET, "new Wallet is initialized for " + network + " at " + walletDataPath + ". Resulting context_id: " + QString::number(context_id));
        Q_ASSERT(context_id>=0);

        nodeClient = _nodeClient;
        Q_ASSERT(nodeClient.get() != nullptr);

        // registering a callback
        Q_ASSERT(node_client_callback_name.isEmpty());
        node_client_callback_name = "node_client_" + QString::number(context_id);
        register_lib_callback(node_client_callback_name.toStdString().c_str(), node_client_callback, nodeClient.get());

        Q_ASSERT(update_status_callback_name.isEmpty());
        update_status_callback_name = "scan_status_" + QString::number(context_id);
        register_lib_callback(update_status_callback_name.toStdString().c_str(), update_status_callback, this);
    }

    #define CANCEL_FUTURE(future) if (future.isValid() && !future.isFinished()) { future.cancel();}

    WalletInternals::~WalletInternals() {
        // expeted that somebody will call release
        scanInProgress.storeRelease(0);

        if (context_id>=0) {
            mwc_api::ApiResponse<bool> res = stop_running_scan(context_id);
            LOG_CALL_RESULT("stop_running_scan", "OK" );
        }

        CANCEL_FUTURE(restart_listeners);
        CANCEL_FUTURE(scanOp);
        CANCEL_FUTURE(sendOp);
        CANCEL_FUTURE(scanRewindHashOp);
        CANCEL_FUTURE(fausetRequest);

        restart_listeners.waitForFinished();
        scanOp.waitForFinished();
        sendOp.waitForFinished();
        scanRewindHashOp.waitForFinished();
        fausetRequest.waitForFinished();

        if (context_id>=0) {
            mwc_api::ApiResponse<bool> res = close_wallet(context_id);
            LOG_CALL_RESULT("logout close_wallet", "OK" );
            res = release_wallet(context_id);
            LOG_CALL_RESULT("logout release_wallet", "OK" );
            context_id = -1;
        }

        mqs_running = false;
        tor_running = false;

        // Callbacks are garanteed be not in use. release_wallet did finish all threads and destroy all wallet's internals
        if (!node_client_callback_name.isEmpty()) {
            unregister_lib_callback(node_client_callback_name.toStdString().c_str());
            node_client_callback_name = "";
        }
        if (!update_status_callback_name.isEmpty()) {
            unregister_lib_callback(update_status_callback_name.toStdString().c_str());
            update_status_callback_name = "";
        }

        // Taking care about the client and a mwcNode. Callbacks are done, it is safe to delete them

        // deleting client first, the node must be after
        nodeClient.reset();

    }


    bool WalletInternals::isBusy() const {
        if (restart_listeners.isRunning() || scanOp.isRunning() || sendOp.isRunning() || scanRewindHashOp.isRunning() || fausetRequest.isRunning())
            return true;

        if (check_wallet_busy(context_id).response)
            return true;

        return false;
    }

    bool WalletInternals::isUpdateInProgress() const {
        return scanInProgress.loadAcquire() != 0;
    }

    void WalletInternals::scanDone(QString responseId, bool fullScan, int height, QString errorMessage ) {
        scanInProgress.storeRelease(0);

        if (parentWallet!=nullptr) {
            parentWallet->scanDone(responseId, fullScan, height, errorMessage );
        }
    }

    void WalletInternals::scanRewindDone( const QString & responseId, const ViewWallet & result, const QString & error ) {
        if (parentWallet!=nullptr) {
            parentWallet->scanRewindDone(responseId, result, error );
        }
    }

    void WalletInternals::sendDone( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag ) {
        if (parentWallet!=nullptr) {
            parentWallet->sendDone( success, error, tx_uuid, amount, method, dest, tag );
        }
    }

    void WalletInternals::startStopListenersDone(int operation) {
        restart_listeners = QFuture<void>();

        if (isExiting())
            return;

        if (parentWallet!=nullptr) {
            parentWallet->startStopListenersDone(operation);
        }

        startNextStartStopListeners();
    }

    void WalletInternals::startNextStartStopListeners() {
        if (restart_listeners.isRunning())
            return;

        if (isExiting())
            return;

        if (context_id>0 && nextListenersTask!=0) {
            restart_listeners = startStopListeners(this, nextListenersTask, torStarter);
            nextListenersTask = 0;
        }
    }

    void WalletInternals::mwcFromFlooFaucetDone(bool success, QString errorMsg) {
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

        // Mark fauset request as done.
        if (success) {
            state::getStateContext()->appContext->faucetRequested();
        }

        if (parentWallet!=nullptr) {
            parentWallet->mwcFromFlooFaucetDone(success, errorMsg);
        }
    }

    void WalletInternals::requestFaucetMWC() {
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

        if (isExiting()) {
            mwcFromFlooFaucetDone(false, "");
            return;
        }

        if (fausetRequest.isRunning()) {
            mwcFromFlooFaucetDone(false, "");
            return;
        }

        // requesting 3 MWC
        QPair<bool, qint64> amount = util::one2nano("3.0");
        Q_ASSERT(amount.first);

        fausetRequest = wallet::requestMwcFromFlooFaucet(this, amount.second);
    }

    void WalletInternals::emitStatusUpdate( const QString & response_id, const QJsonObject & status ) {
        if (parentWallet!=nullptr) {
            parentWallet->emitStatusUpdate(response_id, status);
        }
    }

    // Create new wallet and generate a seed for it
    QPair<QStringList, QString> WalletInternals::start2init(const QString & password, int seedLength) {
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

        mwc_api::ApiResponse<QStringList> res = create_new_wallet(context_id, node_client_callback_name, seedLength, password );
        LOG_CALL_RESULT("start2init", "get a new seed");

        if (!res.hasError()) {
            open = true;
        }

        mqs_running = false;
        tor_running = false;
        return QPair<QStringList, QString>(res.response, util::mapMessage(res.error));
    }

    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase. Scan call needs to be done.
    // Return: Error
    QString WalletInternals::start2recover(const QStringList & seed, const QString & password) {
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

        mwc_api::ApiResponse<bool> res = restore_new_wallet(context_id, node_client_callback_name, seed.join(" "), password );
        LOG_CALL_RESULT("start2recover", "OK");

        if (!res.hasError()) {
            open = true;
        }

        mqs_running = false;
        tor_running = false;
        return util::mapMessage(res.error);
    }

    int WalletInternals::getWalletId() const {
        return (int) qHash(walletDataPath);
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
    QString WalletInternals::loginWithPassword(const QString & password) {
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

        mwc_api::ApiResponse<bool> res = open_wallet(context_id, node_client_callback_name, password );
        LOG_CALL_RESULT("loginWithPassword", "OK");

        if (!res.hasError()) {
            open = true;
        }

        // restoring current selected accounts
        int wallet_id = getWalletId();
        QVector<wallet::Account> accounts = listAccounts();

        core::AppContext * appContext = state::getStateContext()->appContext;

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

    QVector<Account> WalletInternals::listAccounts() const {
        mwc_api::ApiResponse<QVector<Account>> res = list_accounts(context_id);
        LOG_CALL_RESULT("getWalletBalance list_accounts", QString::number(res.response.size()) + " accounts" );
        return res.response;
    }

    // Switch to different account
    void WalletInternals::switchAccountById(const QString & accountPath) {
        mwc_api::ApiResponse<bool> res = switch_account(context_id, accountPath);
        LOG_CALL_RESULT("switch_account", "OK" );
    }

    void WalletInternals::setReceiveAccountById(const QString & accountPath) {
        mwc_api::ApiResponse<bool> res = switch_receive_account(context_id, accountPath);
        LOG_CALL_RESULT("setReceiveAccount", "OK" );
    }

    // Return true if wallet has this password. Wallet might not have password (has empty password) if it was created manually.
    // Expected that the wallet is already open,
    bool WalletInternals::checkPassword(const QString & password) const {
        mwc_api::ApiResponse<bool> res = validate_password(context_id, password );
        if (!password.isEmpty()) {
            LOG_CALL_RESULT("checkPassword", (res.response ? "True" : "False") );
        }

        if (res.hasError() || !res.response) {
            QThread::msleep(100); // Brute force attack mitigation
            return false;
        }
        else {
            Q_ASSERT(res.response);
            return true;
        }
    }

    // Current seed for runnign wallet
    QPair<QStringList, QString> WalletInternals::getSeed(const QString & walletPassword) const {
        mwc_api::ApiResponse<QStringList> res = get_mnemonic(context_id, walletPassword );
        LOG_CALL_RESULT("getSeed", "OK" );
        return QPair<QStringList, QString>(res.response, util::mapMessage(res.error));

    }

    void WalletInternals::restartRunningListeners() {
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
    ListenerStatus WalletInternals::getListenerStatus() const {
        ListenerStatus result;
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
    void WalletInternals::listeningStart(bool startMq, bool startTor) {
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
    bool WalletInternals::listeningStop(bool stopMq, bool stopTor) {
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
    QString WalletInternals::getMqsAddress() const {
        mwc_api::ApiResponse<QString> res = mqs_address(context_id);
        LOG_CALL_RESULT("getMqsAddress", "OK" );
        return res.response;
    }

    // Request Tor address
    QString WalletInternals::getTorSlatepackAddress() const {
        mwc_api::ApiResponse<QString> res = tor_address(context_id);
        LOG_CALL_RESULT("tor_address", "OK" );
        return res.response;
    }

    // requst current address index
    int WalletInternals::getAddressIndex() const {
        mwc_api::ApiResponse<int> res = get_address_index(context_id);
        LOG_CALL_RESULT("get_address_index", QString::number(res.response) );
        return res.response;
    }

    // Note, set address index does update the MQS and Tor addresses
    // The Listeners, if running, will be restarted automatically
    void WalletInternals::setAddressIndex(int index) {
        mwc_api::ApiResponse<bool> res = set_address_index(context_id, index);
        LOG_CALL_RESULT("get_address_index", "OK" );

        if (mqs_running || tor_running) {
            logger::logInfo(logger::MWC_WALLET, "Address index was changed, restarting listeners...");
            restartRunningListeners();
        }
    }

    // Get all accounts with balances.
    QVector<AccountInfo> WalletInternals::getWalletBalance(int confirmations, bool filterDeleted, const QStringList & manuallyLockedOutputs) const {
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
                    balance.response.last_confirmed_height.toLongLong()); // height
                if (filterDeleted && wi.isDeleted())
                    continue;
                result.push_back(wi);
            }
        }

        return result;
    }

    QString WalletInternals::getCurrentAccountId() const {
        mwc_api::ApiResponse<QString> res = current_account(context_id);
        LOG_CALL_RESULT("current_account", res.response );

        return res.response;
    }

    // Create another account, note no delete exist for accounts
    QString WalletInternals::createAccount( const QString & accountName ) {
        mwc_api::ApiResponse<QString> res = create_account(context_id, accountName);
        LOG_CALL_RESULT("create_account", res.response );

        return res.response;
    }

    // Rename account
    void WalletInternals::renameAccountById( const QString & accountPath, const QString & newAccountName) {
        mwc_api::ApiResponse<bool> res = rename_account(context_id, accountPath, newAccountName);
        LOG_CALL_RESULT("rename_account", "OK" );
    }

    // Check and repair the wallet. Will take a while
    // Check Signal: onScanProgress, onScanDone
    // Return responseId
    QString WalletInternals::scan(bool delete_unconfirmed) {
        if (scanOp.isRunning())
            return lastScanResponseId;

        int id = response_id_counter.fetchAndAddRelaxed(1);
        QString responseId = "scan_" + QString::number(id);

        // Scan started in any case
        lastScanResponseId = responseId;
        scanInProgress.storeRelease(1);

        scanOp = startScan(this, update_status_callback_name, responseId, true, delete_unconfirmed);
        return responseId;
    }

    // Update the wallet state, resync with a current node state
    // Check Signal: onScanProgress, onScanDone
    // Return responseId
    QString WalletInternals::update_wallet_state() {
        if (scanOp.isRunning())
            return lastScanResponseId;

        int id = response_id_counter.fetchAndAddRelaxed(1);
        QString responseId = "upd_" + QString::number(id);

        lastScanResponseId = responseId;
        scanInProgress.storeRelease(1);

        // Scan started in any case
        scanOp = startScan(this, update_status_callback_name, responseId, false, false);
        return responseId;
    }

    QString WalletInternals::getReceiveAccountPath() const {
        mwc_api::ApiResponse<QString> res = receive_account(context_id);
        LOG_CALL_RESULT("setReceiveAccount", res.response );
        return res.response;
    }

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Return error or JsonObject
    QString WalletInternals::generateTransactionProof( const QString & transactionUuid ) {
        mwc_api::ApiResponse<QJsonObject> res = get_proof(context_id, transactionUuid);
        LOG_CALL_RESULT("get_proof", "OK" );

        if (res.hasError())
            return util::mapMessage(res.error);
        else
            return QJsonDocument(res.response).toJson(QJsonDocument::Compact);
    }

    // Verify the proof for transaction
    // Return error or JsonObject
    QString WalletInternals::verifyTransactionProof(const QString & proof) const {
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
    bool WalletInternals::sendTo( const QString &accountPathFrom,
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
        if (sendOp.isRunning()) {
            logger::logInfo(logger::QT_WALLET, "Send task is already running, can't start another one");
            return false;
        }

        sendOp = sendTask(this, accountPathFrom, responseTag, amount, amount_includes_fee,
                        message, minimum_confirmations, selection_strategy, method,
                        dest, generate_proof, change_outputs, fluff, ttl_blocks,
                        exclude_change_outputs, outputs,
                        late_lock, min_fee);
        return true;
    }

    // Receive slatepack. Will generate the resulting slatepack.
    // Return: <SlatePack, Error>
    // In case of error SlatePack is empty
    QPair<ResReceive,QString> WalletInternals::receiveSlatepack( QString slatePack, QString description) {
        mwc_api::ApiResponse<ResReceive> res = receive(context_id, slatePack, description, "");
        LOG_CALL_RESULT("receive", "OK" );
        return QPair<ResReceive,QString>(res.response, util::mapMessage(res.error));
    }

    // finalize SP transaction and broadcast it
    // Return error message
    QString WalletInternals::finalizeSlatepack( const QString & slatepack, bool fluff, bool nopost ) {
        mwc_api::ApiResponse<bool> res = finalize(context_id, slatepack, fluff, nopost);
        LOG_CALL_RESULT("finalize", "OK" );
        return util::mapMessage(res.error);
    }

    // submit finalized transaction. Make sense for cold storage => online node operation
    // Return Error message
    QString WalletInternals::submitFile( QString fileTx, bool fluff ) {
        mwc_api::ApiResponse<bool> res = post(context_id, fileTx,
                 fluff);
        LOG_CALL_RESULT("post", "OK" );
        return util::mapMessage(res.error);
    }

    // Show outputs for the wallet
    QVector<WalletOutput> WalletInternals::getOutputs(const QString & accountPath, bool show_spent) {
        mwc_api::ApiResponse<QVector<WalletOutput>> res = outputs(context_id, accountPath, show_spent );
        LOG_CALL_RESULT("outputs", QString::number(res.response.size()) +  " output" );
        return res.response;
    }

    // Show all transactions for current account
    QVector<WalletTransaction> WalletInternals::getTransactions( const QString & accountPath ) const {
        mwc_api::ApiResponse<QVector<WalletTransaction>> res = transactions(context_id, accountPath);
        LOG_CALL_RESULT("transactions", QString::number(res.response.size()) +  " transactions" );
        return res.response;
    }

    WalletTransaction WalletInternals::getTransactionByUUID( const QString & tx_uuid ) const {
        mwc_api::ApiResponse<WalletTransaction> res = transaction_by_uuid(context_id, tx_uuid);
        LOG_CALL_RESULT("transactions", "OK" );
        return res.response;
    }

    // Cancelt TX by UUID (in case of multi accounts, we want to cancel both)
    // Return error string
    QString  WalletInternals::cancelTransacton(const QString & txUUID) {
        mwc_api::ApiResponse<bool> res = cancel(context_id, txUUID);
        LOG_CALL_RESULT("cancel", "OK" );
        return util::mapMessage(res.error);
    }

    // True if transaction was finalized and can be reposted
    bool WalletInternals::hasFinalizedData(const QString & txUUID) const {
        mwc_api::ApiResponse<bool> res = has_finalized_data(context_id, txUUID);
        LOG_CALL_RESULT("has_finalized_data", "OK" );
        return res.response;
    }

    // Repost the transaction.
    // Retunr Error
    QString WalletInternals::repostTransaction(const QString & txUUID, bool fluff) {
        mwc_api::ApiResponse<bool> res = repost(context_id, txUUID, fluff);
        LOG_CALL_RESULT("repost", "OK" );
        return util::mapMessage(res.error);
    }

    // Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
    DecodedSlatepack WalletInternals::decodeSlatepack(const QString & slatepackContent) const {
        DecodedSlatepack res = decode_slatepack(context_id, slatepackContent );
        if (!res.error.isEmpty())
            logger::logError(logger::MWC_WALLET, QString("decode_slatepack for id ") + QString::number(context_id) + " is failed: " + res.error);
        else
            logger::logInfo(logger::MWC_WALLET, QString("decode_slatepack for id ") + QString::number(context_id) + ", is OK");
        return res;
    }

    // Request rewind hash
    QString WalletInternals::viewRewindHash() const {
        mwc_api::ApiResponse<QString> res = rewind_hash(context_id);
        LOG_CALL_RESULT("rewind_hash", "OK" );
        return res.response;
    }

    // Scan with revind hash. That will generate bunch or messages similar to scan
    // Check Signal: onScanProgress
    // Check Signal: onScanRewindHash( QString responseId, ViewWallet walletOutputs, QString errors );
    // Return responseId
    QString WalletInternals::scanRewindHash( const QString & rewindHash ) {
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
    QJsonObject WalletInternals::generateOwnershipProof(const QString & message, bool includeRewindHash, bool includeTorAddress, bool includeMqsAddress ) {
        mwc_api::ApiResponse<QJsonObject> res = generate_ownership_proof(context_id, message,
            includeRewindHash, includeTorAddress, includeMqsAddress );
        LOG_CALL_RESULT("generate_ownership_proof", "OK" );
        return res.response;
    }

    // Validate ownership proof
    OwnershipProofValidation WalletInternals::validateOwnershipProof(const QJsonObject & proof) {
        OwnershipProofValidation res = validate_ownership_proof(context_id, proof);
        if (!res.error.isEmpty())
            logger::logError(logger::MWC_WALLET, QString("validate_ownership_proof for id ") + QString::number(context_id) + " is failed: " + res.error);
        else
            logger::logInfo(logger::MWC_WALLET, QString("validate_ownership_proof for id ") + QString::number(context_id) + ", is OK");

        return res;
    }

    bool WalletInternals::isNodeHealthy() {
        return nodeClient->isNodeHealthy();
    }

    bool WalletInternals::isUsePublicNode() const {
        return nodeClient->isUsePublicNode();
    }

    qint64 WalletInternals::getLastNodeHeight() const {
        return nodeClient->getLastNodeHeight();
    }

    NodeStatus WalletInternals::requestNodeStatus() const {
        return nodeClient->requestNodeStatus();
    }

    QString WalletInternals::getLastInternalNodeState() const {
        return nodeClient->getLastInternalNodeState();
    }


}
