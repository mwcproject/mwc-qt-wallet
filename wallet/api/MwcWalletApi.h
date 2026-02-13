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

#ifndef MWC_QT_WALLET_MWCWALLETAPI_H
#define MWC_QT_WALLET_MWCWALLETAPI_H

#include "node/MwcNodeApi.h"
#include "wallet/wallet_objs.h"

namespace wallet {
    struct ResListenerStatus {
        bool running = false;
        bool healthy = false;
    };

    struct ResWalletInfo {
        // Those values are uint64,  but stored as String because of Json limitaitons
        QString last_confirmed_height;
        QString minimum_confirmations;
        QString total;
        QString amount_awaiting_finalization;
        QString amount_awaiting_confirmation;
        QString amount_immature;
        QString amount_currently_spendable;
        QString amount_locked;
        QString amount_reverted;
    };

    mwc_api::ApiResponse<bool> set_receive_tx_callback( const QString & callback_name, int8_t const * (*newTxCallback)(void *, int8_t const *), void * newTransactionContext );
    mwc_api::ApiResponse<bool> clean_receive_tx_callback(const QString & callback_name);

    // Prepare to work wallet, creating context and config
    mwc_api::ApiResponse<int> init_wallet(const QString & chainNetworkName, const QString & dataPath);

    // Release the wallet and close everything
    mwc_api::ApiResponse<bool> release_wallet(int context_id);

    // Create a new waalet. Return new seed
    mwc_api::ApiResponse<QStringList> create_new_wallet(int context_id, const QString & node_client_callback_name, int mnemonic_length, const QString & password );

    // Create a new wallet from the seed. No scan will do done.
    mwc_api::ApiResponse<bool> restore_new_wallet(int context_id, const QString & node_client_callback_name, const QString & mnemonic, const QString & password );

    // Open the wallet with a password.
    mwc_api::ApiResponse<bool> open_wallet(int context_id, const QString & node_client_callback_name, const QString & password );

    // Verify if the password correct. Wallet expected to be open
    mwc_api::ApiResponse<bool> validate_password(int context_id, const QString & password );

    // Logout from the account
    mwc_api::ApiResponse<bool> close_wallet(int context_id);

    // Interrupt scanning
    mwc_api::ApiResponse<bool> stop_running_scan(int context_id);

    // Get mnemonic phrase
    mwc_api::ApiResponse<QStringList> get_mnemonic(int context_id, const QString & password );

    // Start/stop Tor listener
    mwc_api::ApiResponse<bool> start_tor_listener(int context_id);
    mwc_api::ApiResponse<bool> stop_tor_listener(int context_id);
    mwc_api::ApiResponse<ResListenerStatus> get_tor_listener_status(int context_id);

    // Start/stop MQS lestener
    mwc_api::ApiResponse<bool> start_mqs_listener(int context_id);
    mwc_api::ApiResponse<bool> stop_mqs_listener(int context_id);
    mwc_api::ApiResponse<ResListenerStatus> get_mqs_listener_status(int context_id);

    // Get Tor/Slatepack/MQS addresses
    mwc_api::ApiResponse<QString> mqs_address(int context_id);
    mwc_api::ApiResponse<QString> tor_address(int context_id);

    // Address index for addresses
    mwc_api::ApiResponse<int> get_address_index(int context_id);
    mwc_api::ApiResponse<bool> set_address_index(int context_id, int address_index);

    //////////////////////////////////////////
    // Accounts related methods

    // List all account, including 'deleted'
    mwc_api::ApiResponse<QVector<Account>> list_accounts(int context_id);
    // Create a new account. Return account path
    mwc_api::ApiResponse<QString> create_account(int context_id, const QString & accountName);
    // Rename account by path
    mwc_api::ApiResponse<bool> rename_account(int context_id, const QString & accountPath, const QString & newAccountName);
    // Get current wallet accout. Return account path
    mwc_api::ApiResponse<QString> current_account(int context_id);
    // Change current account
    mwc_api::ApiResponse<bool> switch_account(int context_id, const QString & accountPath);
    // Get receive wallet accout. Return account path
    mwc_api::ApiResponse<QString> receive_account(int context_id);
    // Change current account
    mwc_api::ApiResponse<bool> switch_receive_account(int context_id, const QString & accountPath);

    mwc_api::ApiResponse<ResWalletInfo> info(int context_id, int confirmations, const QString & account_path, const QStringList & manuallyLockedOutputs );

    mwc_api::ApiResponse<int> scan(int context_id, bool delete_unconfirmed,  const QString & response_callback, const QString & response_id);
    mwc_api::ApiResponse<int> update_wallet_state(int context_id, const QString & response_callback, const QString & response_id);

    mwc_api::ApiResponse<QJsonObject> get_proof(int context_id, const QString & txUuid);
    mwc_api::ApiResponse<QJsonObject> verify_proof(int context_id, const QString & proof);

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
                qint64 min_fee); // 0 or negative to skip. Currently no needs to define it

    mwc_api::ApiResponse<ResReceive> receive(int context_id, const QString & slatepack,
        const QString & message, // Option, Can be empty
        const QString & receiveAccountName ); // Option, Can be empty

    mwc_api::ApiResponse<bool> finalize(int context_id, const QString & slatepack,
        bool fluff, bool nopost);

    // Post any transaction (used for cold wallet setup)
    mwc_api::ApiResponse<bool> post(int context_id, const QString & input_path,
        bool fluff);

    mwc_api::ApiResponse<QVector<WalletOutput>> outputs(int context_id, const QString & accountPath, bool include_spent );

    mwc_api::ApiResponse<QVector<WalletTransaction>> transactions(int context_id, const QString & accountPath);

    mwc_api::ApiResponse<WalletTransaction> transaction_by_uuid(int context_id, const QString & tx_uuid);

    // return true if transaction has finalized data and can be reposted
    mwc_api::ApiResponse<bool> has_finalized_data(int context_id, const QString & txUUID);

    // Cancel TX by UUID
    mwc_api::ApiResponse<bool> cancel(int context_id, const QString & txUUID);

    // Reposet existing Send transaction
    mwc_api::ApiResponse<bool> repost(int context_id, const QString & txUUID, bool fluff);

    mwc_api::ApiResponse<QString> request_receiver_proof_address(int context_id, const QString & url, const QString & apisecret);

    DecodedSlatepack decode_slatepack(int context_id, const QString & slatepack );

    mwc_api::ApiResponse<QString> rewind_hash(int context_id);

    mwc_api::ApiResponse<ViewWallet> scan_rewind_hash(int context_id, const QString & rewind_hash, const QString & response_callback, const QString & response_id);

    mwc_api::ApiResponse<QJsonObject> generate_ownership_proof(int context_id, const QString & message,
            bool include_rewind_hash, bool include_tor_address, bool include_mqs_address);

    OwnershipProofValidation validate_ownership_proof(int context_id, const QJsonObject & ownership_proof);

    mwc_api::ApiResponse<bool> zip_file( const QString & src_file, const QString & dst_file, const QString & dst_file_name );

    mwc_api::ApiResponse<bool> check_wallet_busy(int context_id);

    mwc_api::ApiResponse<bool> faucet_request(int context_id, quint64 amount);
}

#endif //MWC_QT_WALLET_MWCWALLETAPI_H
