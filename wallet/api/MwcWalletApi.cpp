// Copyright 2035 The MWC Developers
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

#include "MwcWalletApi.h"

#include <QJsonArray>

#include "../../../mwc-wallet/mwc_wallet_lib/c_header/mwc_wallet_interface.h"

namespace wallet {

    static mwc_api::ApiResponse<bool> call_simple_context_id_only_api(int context_id, const QString & methodName) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request(methodName, params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse(methodName, result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    static WalletTransaction toWalletTransaction(const QJsonObject & tx, qint64 height) {
        QString tx_type = tx["tx_type"].toString();

        bool confirmed = tx["confirmed"].toBool(false);

        int tansType = WalletTransaction::TRANSACTION_TYPE::NONE;
        if ( tx_type== "ConfirmedCoinbase" ) {
            tansType = WalletTransaction::TRANSACTION_TYPE::COIN_BASE;
        }
        else if ( tx_type== "TxReceived" ) {
            tansType = WalletTransaction::TRANSACTION_TYPE::RECEIVE;
        }
        else if ( tx_type== "TxSent" ) {
            tansType = WalletTransaction::TRANSACTION_TYPE::SEND;
        }
        else if ( tx_type== "TxReceivedCancelled" ) {
            tansType = WalletTransaction::TRANSACTION_TYPE::RECEIVE | WalletTransaction::TRANSACTION_TYPE::CANCELLED;
        }
        else if ( tx_type== "TxSentCancelled" ) {
            tansType = WalletTransaction::TRANSACTION_TYPE::SEND | WalletTransaction::TRANSACTION_TYPE::CANCELLED;
        }
        else if ( tx_type== "TxReverted" ) {
            tansType = WalletTransaction::TRANSACTION_TYPE::RECEIVE;
            confirmed = false;
        }

        QString ttl_cutoff_height = tx["ttl_cutoff_height"].toString();

        qint64 credited = tx["amount_credited"].toString().toLongLong();
        qint64 debited = tx["amount_debited"].toString().toLongLong();

        QVector<QString> inputs;
        auto inp = tx["input_commits"].toArray();
        for ( int i=0; i<inp.size(); i++ ) {
            inputs.push_back( inp[i].toString() );
        }
        QVector<QString> outputs;
        auto out = tx["output_commits"].toArray();
        for ( int i=0; i<out.size(); i++ ) {
            outputs.push_back( out[i].toString() );
        }
        QVector<WalletTransactionMessage> messages;
        auto msg = tx["messages"].toObject()["messages"].toArray();
        for ( int i=0; i<msg.size(); i++ ) {
            QString m = msg[i].toObject()["message"].toString();
            int id = msg[i].toObject()["id"].toString().toInt();
            if (id>=0 && !m.isEmpty()) {
                messages.push_back(WalletTransactionMessage(id, m));
            }
        }

        WalletTransaction res_tx;
        res_tx.setData( tx["id"].toInt(),
                  tansType,
                  tx["tx_slate_id"].toString(""),
                  tx["address"].toString(""),
                  tx["creation_ts"].toString(), // Example: "2019-01-15T16:01:26Z"
                  confirmed,
                  ttl_cutoff_height.isEmpty() ? -1 : qint64 (ttl_cutoff_height.toLongLong()),
                  tx["output_height"].toInteger(),
                  height,
                  tx["confirmation_ts"].toString(),
                  tx["num_inputs"].toInt(),
                  tx["num_outputs"].toInt(),
                  credited,
                  debited,
                  tx["fee"].toString("0").toLongLong(),
                  credited - debited,
                  tx["kernel_excess"].toString(),
                  inputs,
                  outputs,
                  messages);

        return  res_tx;
    }

    mwc_api::ApiResponse<bool> set_receive_tx_callback( const QString & callback_name, int8_t const * (*newTxCallback)(void *, int8_t const *), void * newTransactionContext ) {
        // Register the callback function first
        register_lib_callback(callback_name.toStdString().c_str(), newTxCallback, newTransactionContext);

        QJsonObject params;
        params["callback_name"] = callback_name;

        QString request = mwc_api::build_request("register_receive_slate_callback", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("register_receive_slate_callback", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    mwc_api::ApiResponse<bool> clean_receive_tx_callback(const QString & callback_name) {
        QJsonObject params;

        QString request = mwc_api::build_request("clean_receive_slate_callback", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("clean_receive_slate_callback", result);

        unregister_lib_callback(callback_name.toStdString().c_str());

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }


    // Prepare to work wallet, creating context and config
    mwc_api::ApiResponse<int> init_wallet(const QString & chainNetworkName, const QString & dataPath) {

        QJsonObject wallet;
        wallet["chain_type"] = chainNetworkName;
        wallet[ "data_file_dir"] = dataPath;
        wallet["wallet_data_dir"] = "";
        QJsonObject config;
        config["wallet"] = wallet;
        QJsonObject params;
        params["config"] = config;

        QString request = mwc_api::build_request("init_wallet", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("create_context", result);

        if (response.second.isEmpty()) {
            return mwc_api::ApiResponse<int>( response.first["context_id"].toInt(-1) );
        }
        else {
            return mwc_api::ApiResponse<int>( -1, response.second );
        }
    }

    mwc_api::ApiResponse<bool> release_wallet(int context_id) {
        return call_simple_context_id_only_api(context_id, "release_wallet");
    }

    // Create a new waalet. Return new seed
    mwc_api::ApiResponse<QStringList> create_new_wallet(int context_id, const QString & node_client_callback_name, int mnemonic_length, const QString & password ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["node_client_callback"] = node_client_callback_name;
        params["mnemonic_length"] = mnemonic_length;
        params[ "password" ] = password;

        QString request = mwc_api::build_request("create_new_wallet", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("create_new_wallet", result, false);

        if (response.second.isEmpty()) {
            QString mnemonic_prase = response.first["mnemonic"].toString();
            return mwc_api::ApiResponse<QStringList> (mnemonic_prase.split(" ", Qt::SkipEmptyParts ) );
        }
        else {
            return mwc_api::ApiResponse<QStringList> (QStringList(), response.second);
        }
    }

    // Create a new wallet from the seed. No scan will do done.
    mwc_api::ApiResponse<bool> restore_new_wallet(int context_id, const QString & node_client_callback_name, const QString & mnemonic, const QString & password ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["node_client_callback"] = node_client_callback_name;
        params["mnemonic"] = mnemonic;
        params[ "password" ] = password;

        QString request = mwc_api::build_request("restore_new_wallet", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("restore_new_wallet", result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    // Open the wallet with a password.
    mwc_api::ApiResponse<bool> open_wallet(int context_id, const QString & node_client_callback_name, const QString & password ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["node_client_callback"] = node_client_callback_name;
        params[ "password" ] = password;

        QString request = mwc_api::build_request("open_wallet", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("open_wallet", result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    // Verify if the password correct. Wallet expected to be open
    mwc_api::ApiResponse<bool> validate_password(int context_id, const QString & password ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params[ "password" ] = password;

        QString request = mwc_api::build_request("validate_password", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("validate_password", result, false);

        if (response.second.isEmpty()) {
            bool valid = response.first["valid"].toBool(false);
            return mwc_api::ApiResponse<bool> (valid);
        }
        else {
            return mwc_api::ApiResponse<bool> (false, response.second);
        }
    }

    mwc_api::ApiResponse<bool> close_wallet(int context_id) {
        return call_simple_context_id_only_api(context_id, "close_wallet");
    }

    // Get mnemonic phrase
    mwc_api::ApiResponse<QStringList> get_mnemonic(int context_id, const QString & password ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params[ "password" ] = password;

        QString request = mwc_api::build_request("get_mnemonic", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_mnemonic", result, false);

        if (response.second.isEmpty()) {
            QString mnemonic_prase = response.first["mnemonic"].toString();
            return mwc_api::ApiResponse<QStringList> (mnemonic_prase.split(" ", Qt::SkipEmptyParts ) );
        }
        else {
            return mwc_api::ApiResponse<QStringList> (QStringList(), response.second);
        }
    }

    // Start/stop Tor listener
    mwc_api::ApiResponse<bool> start_tor_listener(int context_id) {
        return call_simple_context_id_only_api(context_id, "start_tor_listener");
    }

    mwc_api::ApiResponse<bool> stop_tor_listener(int context_id) {
        return call_simple_context_id_only_api(context_id, "stop_tor_listener");
    }

    mwc_api::ApiResponse<ResListenerStatus> get_tor_listener_status(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("get_tor_listener_status", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_tor_listener_status", result, false);

        ResListenerStatus res;
        if (response.second.isEmpty()) {
            res.running = response.first["running"].toBool(false);
            res.healthy = response.first["healthy"].toBool(false);
        }
        return mwc_api::ApiResponse<ResListenerStatus> (res, response.second);
    }


    // Start/stop MQS lestener
    mwc_api::ApiResponse<bool> start_mqs_listener(int context_id) {
        return call_simple_context_id_only_api(context_id, "start_mqs_listener");
    }

    mwc_api::ApiResponse<bool> stop_mqs_listener(int context_id) {
        return call_simple_context_id_only_api(context_id, "stop_mqs_listener");
    }

    mwc_api::ApiResponse<ResListenerStatus> get_mqs_listener_status(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("get_mqs_listener_status", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_mqs_listener_status", result, false);

        ResListenerStatus res;
        if (response.second.isEmpty()) {
            res.running = response.first["running"].toBool(false);
            res.healthy = response.first["healthy"].toBool(false);
        }
        return mwc_api::ApiResponse<ResListenerStatus> (res, response.second);
    }

    mwc_api::ApiResponse<QString> mqs_address(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("mqs_address", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("mqs_address", result);

        QString address;
        if (response.second.isEmpty()) {
            address = response.first["mqs_addr"].toString();
        }

        return mwc_api::ApiResponse<QString>(address, response.second);
    }

    mwc_api::ApiResponse<QString> tor_address(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("tor_address", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("tor_address", result);

        QString address;
        if (response.second.isEmpty()) {
            address = response.first["tor_addr"].toString();
        }

        return mwc_api::ApiResponse<QString>(address, response.second);
    }

    mwc_api::ApiResponse<int> get_address_index(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("get_address_index", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_address_index", result);

        int address_index = -1;
        if (response.second.isEmpty()) {
            address_index = response.first["address_index"].toInt(-1);
        }
        return mwc_api::ApiResponse<int>(address_index, response.second);
    }

    mwc_api::ApiResponse<bool> set_address_index(int context_id, int address_index) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["address_index"] = address_index;

        QString request = mwc_api::build_request("set_address_index", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("set_address_index", result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    mwc_api::ApiResponse<ResWalletInfo> info(int context_id, int confirmations, const QString & account_path, const QStringList & manuallyLockedOutputs ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["confirmations"] = confirmations;
        params["account_path"] = account_path;
        QJsonArray lockedOutputs;
        for (const auto & out : manuallyLockedOutputs) {
            lockedOutputs.append(out);
        }
        params["manually_locked_outputs"] = lockedOutputs;

        QString request = mwc_api::build_request("info", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("info", result);

        ResWalletInfo walletInfo;
        if (response.second.isEmpty()) {
            walletInfo.last_confirmed_height = response.first["last_confirmed_height"].toString("");
            walletInfo.minimum_confirmations = response.first["minimum_confirmations"].toString("");
            walletInfo.total = response.first["total"].toString("");
            walletInfo.amount_awaiting_finalization = response.first["amount_awaiting_finalization"].toString("");
            walletInfo.amount_awaiting_confirmation = response.first["amount_awaiting_confirmation"].toString("");
            walletInfo.amount_immature = response.first["amount_immature"].toString("");
            walletInfo.amount_currently_spendable = response.first["amount_currently_spendable"].toString("");
            walletInfo.amount_locked = response.first["amount_locked"].toString("");
            walletInfo.amount_reverted = response.first["amount_reverted"].toString("");
        }
        return mwc_api::ApiResponse<ResWalletInfo>(walletInfo, response.second);
    }

    mwc_api::ApiResponse<QVector<Account>> list_accounts(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("list_accounts", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("list_accounts", result);

        QVector<Account> accounts;
        if (response.second.isEmpty()) {
            QJsonArray accs = response.first["accounts"].toArray();
            for (int i=0;i<accs.size(); i++) {
                QJsonObject ac = accs[i].toObject();
                Account a;
                a.label = ac["label"].toString();
                a.path = ac["path"].toString();
                accounts.push_back(a);
            }
        }
        return mwc_api::ApiResponse<QVector<Account>>(accounts, response.second);
    }

    // Create a new account. Retirn account path
    mwc_api::ApiResponse<QString> create_account(int context_id, const QString & accountName) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["account_name"] = accountName;

        QString request = mwc_api::build_request("create_account", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("create_account", result);

        QString account_path;
        if (response.second.isEmpty()) {
            account_path = response.first["account_path"].toString();
        }
        return mwc_api::ApiResponse<QString>(account_path, response.second);
    }

    // Rename account by path
    mwc_api::ApiResponse<bool> rename_account(int context_id, const QString & accountPath, const QString & newAccountName) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["account_name"] = newAccountName;
        params["account_path"] = accountPath;

        QString request = mwc_api::build_request("rename_account", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("rename_account", result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    // Get current wallet accout. Return account path
    mwc_api::ApiResponse<QString> current_account(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("current_account", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("current_account", result);

        QString account_path;
        if (response.second.isEmpty()) {
            account_path = response.first["account_path"].toString();
        }
        return mwc_api::ApiResponse<QString>(account_path, response.second);

    }

    // Change current account
    mwc_api::ApiResponse<bool> switch_account(int context_id, const QString & accountPath) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["account_path"] = accountPath;

        QString request = mwc_api::build_request("switch_account", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("switch_account", result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    // Get receive wallet accout. Return account path
    mwc_api::ApiResponse<QString> receive_account(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("receive_account", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("receive_account", result);

        QString account_path;
        if (response.second.isEmpty()) {
            account_path = response.first["account_path"].toString();
        }
        return mwc_api::ApiResponse<QString>(account_path, response.second);
    }

    // Change current account
    mwc_api::ApiResponse<bool> switch_receive_account(int context_id, const QString & accountPath) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["account_path"] = accountPath;

        QString request = mwc_api::build_request("switch_receive_account", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("switch_receive_account", result);

        return mwc_api::ApiResponse<bool> (response.second.isEmpty(), response.second);
    }

    mwc_api::ApiResponse<int> scan(int context_id, bool delete_unconfirmed,  const QString & response_callback, const QString & response_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["delete_unconfirmed"] = delete_unconfirmed;
        params["response_callback"] = response_callback;
        params["response_id"] = response_id;

        QString request = mwc_api::build_request("scan", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("scan", result);

        int height = -1;
        if (response.second.isEmpty()) {
            height = response.first["height"].toInt();
        }
        return mwc_api::ApiResponse<int> (height, response.second);
    }

    mwc_api::ApiResponse<int> update_wallet_state(int context_id, const QString & response_callback, const QString & response_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["response_callback"] = response_callback;
        params["response_id"] = response_id;

        QString request = mwc_api::build_request("update_wallet_state", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("update_wallet_state", result);

        int height = -1;
        if (response.second.isEmpty()) {
            height = response.first["height"].toInt();
        }
        return mwc_api::ApiResponse<int> (height, response.second);
    }

    mwc_api::ApiResponse<QJsonObject> get_proof(int context_id, const QString & txUuid) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["tx_id"] = txUuid;

        QString request = mwc_api::build_request("get_proof", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_proof", result);

        return mwc_api::ApiResponse<QJsonObject>( response.first, response.second );
    }

    mwc_api::ApiResponse<QJsonObject> verify_proof(int context_id, const QString & proof) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["proof"] = proof;

        QString request = mwc_api::build_request("verify_proof", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("verify_proof", result);

        return mwc_api::ApiResponse<QJsonObject>( response.first, response.second );
    }

    mwc_api::ApiResponse<QString> send(
                int context_id,
                qint64 amount, //  -1  - mean All
                bool amount_includes_fee,
                const QString & message, // can be empty, means None
                int minimum_confirmations,
                const QString & selection_strategy, //  Values: all, smallest. Default: Smallest
                const QString & method,  // Values:  http, file, slatepack, self, mwcmqs
                QString dest, // Values depends on 'method' Send the transaction to the provided server (start with http://), destination for encrypting/proofs. For method self, dest can point to account name to move
                bool generate_proof,
                QString http_proof_address,
                int change_outputs,
                bool fluff,
                int ttl_blocks, // pass -1 to skip
                bool exclude_change_outputs,
                int minimum_confirmations_change_outputs,
                QStringList outputs, // Outputs to use. If None, all outputs can be used
                bool late_lock,
                qint64 min_fee)
    {
        if (method=="mwcmqs") {
            if (http_proof_address.startsWith("mwcmqs://"))
                http_proof_address = http_proof_address.right(http_proof_address.length() - strlen("mwcmqs://") );

            if (dest.startsWith("mwcmqs://"))
                dest = dest.right(dest.length() - strlen("mwcmqs://") );
        }

        QJsonObject send_args;
        if (amount<0) {
            // Send all
            send_args["amount"] = 0;
            send_args["use_max_amount"] = true;
            send_args["amount_includes_fee"] = true;
        }
        else {
            send_args["amount"] = amount;
            send_args["use_max_amount"] = false;
            send_args["amount_includes_fee"] = amount_includes_fee;
        }
        if (!message.isEmpty())
            send_args["message"] = message;
        send_args["minimum_confirmations"] = minimum_confirmations;
        send_args["selection_strategy"] = selection_strategy.isEmpty() ? "smallest" : selection_strategy;
        send_args["estimate_selection_strategies"] = false;
        send_args["method"] = method;
        send_args["dest"] = dest;
        send_args["max_outputs"] = 500;

        if (http_proof_address.isEmpty())
            http_proof_address = dest;

        if (!http_proof_address.isEmpty()) {
            if (method!="mwcmqs" && method!="self") {
                send_args["slatepack_recipient"] = http_proof_address;
            }
            if (generate_proof) {
                // destination is expected to be an address in thos case
                send_args["payment_proof_address"] = http_proof_address;
            }
        }

        send_args["change_outputs"] = change_outputs;
        send_args["fluff"] = fluff;
        if (ttl_blocks>0)
            send_args["ttl_blocks"] = ttl_blocks;

        send_args["exclude_change_outputs"] = exclude_change_outputs;
        send_args["minimum_confirmations_change_outputs"] = minimum_confirmations_change_outputs;

        if (!outputs.isEmpty()) {
            QJsonArray outs;
            for (const auto & o : outputs ) {
                outs.append( o );
            }
            send_args["outputs"] = outs;
        }

        send_args["late_lock"] = late_lock;
        if (min_fee>0) {
            send_args["min_fee"] = min_fee;
        }

        send_args["slatepack_qr"] = false;

        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["send_args"] = send_args;

        QString request = mwc_api::build_request("send", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("send", result);

        QString resUuid;
        if (response.second.isEmpty()) {
            resUuid = response.first["tx_uuid"].toString();
        }

        return mwc_api::ApiResponse<QString>( resUuid, response.second );
    }

    mwc_api::ApiResponse<ResReceive> receive(int context_id, const QString & slatepack,
        const QString & message, // Option, Can be empty
        const QString & receiveAccountName ) // Option, Can be empty
    {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["slatepack"] = slatepack;
        if (!message.isEmpty())
            params["message"] = message;

        if (!receiveAccountName.isEmpty())
            params["account"] = receiveAccountName;

        QString request = mwc_api::build_request("receive", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("receive", result);

        ResReceive res;
        if (response.second.isEmpty()) {
            res.slatepack = response.first["reply"].toString();
            res.tx_UUID = response.first["tx_uuid"].toString();
        }

        return mwc_api::ApiResponse<ResReceive>( res, response.second );
    }

    mwc_api::ApiResponse<bool> finalize(int context_id, const QString & slatepack,
            bool fluff, bool nopost)
    {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["slatepack"] = slatepack;
        params["fluff"] = fluff;
        params["nopost"] = nopost;

        QString request = mwc_api::build_request("finalize", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("finalize", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Post any transaction (used for cold wallet setup)
    mwc_api::ApiResponse<bool> post(int context_id, const QString & input_path,
        bool fluff)
    {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["input_path"] = input_path;
        params["fluff"] = fluff;

        QString request = mwc_api::build_request("post", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("post", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    mwc_api::ApiResponse<QVector<WalletOutput>> outputs(int context_id, const QString & accountPath, bool include_spent ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["account_path"] = accountPath;
        params["include_spent"] = include_spent;

        QString request = mwc_api::build_request("outputs", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("outputs", result);

        QVector<WalletOutput> res;

        if (response.second.isEmpty()) {
            int height = response.first["height"].toInt();
            QJsonArray outputs = response.first["outputs"].toArray();
            for (int i=0; i<outputs.size(); i++) {
                QJsonObject out = outputs.at(i).toObject();
                QJsonObject data = out["output"].toObject();

                QString commit = out["commit"].toString();

                QString mmr_index = data["mmr_index"].toString("");
                QString commit_height = data["height"].toString("");
                QString lock_height = data["lock_height"].toString("");
                QString status = data["status"].toString();
                bool is_coinbase = data["is_coinbase"].toBool();
                QString value = data["value"].toString();
                int tx_log_entry = data["tx_log_entry"].toInt(-1);

                int commit_height_val = commit_height.toInt();

                WalletOutput res_output;
                res_output.setData(commit,
                    mmr_index,
                    commit_height,
                    lock_height,
                    status,
                    is_coinbase,
                    (height >= commit_height_val) ? QString::number(height-commit_height_val) : "",
                    value.toLongLong(),
                    tx_log_entry);

                res.push_back(res_output);
            }
        }

        return mwc_api::ApiResponse<QVector<WalletOutput>>(res, response.second);
    }

    mwc_api::ApiResponse<QVector<WalletTransaction>> transactions(int context_id, const QString & accountPath) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["account_path"] = accountPath;

        QString request = mwc_api::build_request("transactions", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("transactions", result);

        QVector<WalletTransaction> res;

        if (response.second.isEmpty()) {
            int height = response.first["height"].toInt();
            QJsonArray transactions = response.first["transactions"].toArray();
            for (int i=0; i<transactions.size(); i++) {
                QJsonObject tx = transactions.at(i).toObject();
                res.push_back(toWalletTransaction(tx, height));
            }
        }

        return mwc_api::ApiResponse<QVector<WalletTransaction>>(res, response.second);
    }

    mwc_api::ApiResponse<WalletTransaction> transaction_by_uuid(int context_id, const QString & tx_uuid) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["tx_uuid"] = tx_uuid;

        QString request = mwc_api::build_request("transaction_by_uuid", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("transaction_by_uuid", result);

        WalletTransaction res;
        if (response.second.isEmpty()) {
            res = toWalletTransaction(response.first, -1);
        }

        return mwc_api::ApiResponse<WalletTransaction>(res, response.second);
    }

    mwc_api::ApiResponse<bool> has_finalized_data(int context_id, const QString & txUUID) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["tx_id"] = txUUID;

        QString request = mwc_api::build_request("has_finalized_data", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("has_finalized_data", result);

        bool finalized = false;
        if (response.second.isEmpty()) {
            finalized = response.first["finalized"].toBool(false);
        }

        return mwc_api::ApiResponse<bool>( finalized, response.second );
    }

    mwc_api::ApiResponse<bool> cancel(int context_id, const QString & txUUID) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["tx_id"] = txUUID;

        QString request = mwc_api::build_request("cancel", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("cancel", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Reposet existing Send transaction
    mwc_api::ApiResponse<bool> repost(int context_id, const QString & txUUID, bool fluff) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["tx_id"] = txUUID;
        params["fluff"] = fluff;

        QString request = mwc_api::build_request("repost", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("repost", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    mwc_api::ApiResponse<QString> request_receiver_proof_address(int context_id, const QString & url, const QString & apisecret) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["url"] = url;
        params["apisecret"] = apisecret;

        QString request = mwc_api::build_request("request_receiver_proof_address", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("request_receiver_proof_address", result);

        QString address;
        if (response.second.isEmpty()) {
            address = response.first["proof_address"].toString();
        }
        return mwc_api::ApiResponse<QString>(address, response.second);
    }

    DecodedSlatepack decode_slatepack(int context_id, const QString & slatepack ) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["slatepack"] = slatepack;

        QString request = mwc_api::build_request("decode_slatepack", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("decode_slatepack", result);

        DecodedSlatepack res;
        if (response.second.isEmpty()) {
            res.slate = response.first["slate"].toObject();
            res.content = response.first["content"].toString();
            res.sender = response.first["sender"].toString();
            res.recipient = response.first["recipient"].toString();
        }
        else {
            res.error = response.second;
        }

        return res;
    }

    mwc_api::ApiResponse<QString> rewind_hash(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("rewind_hash", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("rewind_hash", result);

        QString rewind_hash;
        if (response.second.isEmpty()) {
            rewind_hash = response.first["rewind_hash"].toString();
        }
        return mwc_api::ApiResponse<QString>(rewind_hash, response.second);
    }

    mwc_api::ApiResponse<ViewWallet> scan_rewind_hash(int context_id, const QString & rewind_hash, const QString & response_callback, const QString & response_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["rewind_hash"] = rewind_hash;
        params["response_callback"] = response_callback;
        params["response_id"] = response_id;

        QString request = mwc_api::build_request("scan_rewind_hash", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("scan_rewind_hash", result);

        ViewWallet res;
        if (response.second.isEmpty()) {
            res.rewind_hash = response.first["rewind_hash"].toString();
            res.total_balance = response.first["total_balance"].toInteger();
            res.last_pmmr_index = response.first["last_pmmr_index"].toInteger();
            QJsonArray output_result = response.first["output_result"].toArray();
            for ( int i=0;i<output_result.size(); i++) {
                QJsonObject out = output_result[i].toObject();

                ViewWalletOutputResult output;
                output.commit = out["commit"].toString();
                output.value = out["value"].toInteger();
                output.height = out["height"].toInteger();
                output.mmr_index = out["mmr_index"].toInteger();
                output.is_coinbase = out["is_coinbase"].toBool();
                output.lock_height = out["lock_height"].toInteger();
                res.output_result.push_back(output);
            }
        }
        return mwc_api::ApiResponse<ViewWallet> (res, response.second);
    }

    mwc_api::ApiResponse<QJsonObject> generate_ownership_proof(int context_id, const QString & message,
        bool include_rewind_hash, bool include_tor_address, bool include_mqs_address) {

        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["message"] = message;
        params["include_rewind_hash"] = include_rewind_hash;
        params["include_tor_address"] = include_tor_address;
        params["include_mqs_address"] = include_mqs_address;

        QString request = mwc_api::build_request("generate_ownership_proof", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("generate_ownership_proof", result);

        return mwc_api::ApiResponse<QJsonObject>( response.first, response.second );
    }

    OwnershipProofValidation validate_ownership_proof(int context_id, const QJsonObject & ownership_proof) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["ownership_proof"] = ownership_proof;

        QString request = mwc_api::build_request("validate_ownership_proof", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("validate_ownership_proof", result);

        OwnershipProofValidation res;
        if (response.second.isEmpty()) {
            res.network = response.first["network"].toString();
            res.message = response.first["message"].toString();
            res.viewing_key = response.first["viewing_key"].toString("");
            res.tor_address = response.first["tor_address"].toString("");
            res.mqs_address = response.first["mqs_address"].toString("");
        }
        else {
            res.error = response.second;
        }
        return res;
    }

    mwc_api::ApiResponse<bool> zip_file( const QString & src_file, const QString & dst_file, const QString & dst_file_name ) {
        QJsonObject params;
        params["src_file"] = src_file;
        params["dst_file"] = dst_file;
        params["dst_file_name"] = dst_file_name;

        QString request = mwc_api::build_request("zip_file", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("zip_file", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    mwc_api::ApiResponse<bool> check_wallet_busy(int context_id) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("check_wallet_busy", params, false);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("check_wallet_busy", result, false);

        bool busy = false;
        if (response.second.isEmpty()) {
            busy = response.first["busy"].toBool(false);
        }
        return mwc_api::ApiResponse<bool>( busy, response.second );
    }

    mwc_api::ApiResponse<bool> faucet_request(int context_id, quint64 amount) {
        Q_ASSERT(context_id>=0);
        QJsonObject params;
        params["context_id"] = context_id;
        params["amount"] = (qlonglong) amount;

        QString request = mwc_api::build_request("faucet_request", params);
        char * result = process_mwc_wallet_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("faucet_request", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }



}
