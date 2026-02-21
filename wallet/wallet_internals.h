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


#ifndef MWC_QT_WALLET_WALLET_INTERNALS_H
#define MWC_QT_WALLET_WALLET_INTERNALS_H

#include <QObject>
#include <QFuture>
#include <memory>

#include "wallet_objs.h"

namespace node {
    class MwcNode;
    class NodeClient;
}

namespace wallet {
    class Wallet;
    struct ViewWallet;

    // Move internals separately because I want to be able do some operaitons like a releas ein the separate tasks, so I need to isolate intenals
    class WalletInternals : public QObject
    {
        Q_OBJECT
    public:
        WalletInternals(Wallet * parentWallet, QFuture<QString> * torStarter,
                    QString _network, QString _walletDataPath, std::shared_ptr<node::NodeClient> _nodeClient, QString & errMessage );
        ~WalletInternals();

        std::shared_ptr<node::NodeClient> getNodeClient() const {return nodeClient;}

        void detach() {parentWallet = nullptr;}

        bool isOpen() const {return open;}
        bool isBusy() const;
        bool isUpdateInProgress() const;

        bool isExiting() const {return parentWallet==nullptr;}

        int getWalletId() const;

        void scanDone(QString responseId, bool fullScan, int height, QString errorMessage );
        void scanRewindDone( const QString & responseId, const ViewWallet & result, const QString & error );
        void sendDone( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag );
        void startStopListenersDone(int operation);

        void requestFaucetMWC();
        void mwcFromFlooFaucetDone(bool success, QString errorMsg);

        void emitStatusUpdate( const QString & response_id, const QJsonObject & status );

        // Create new wallet and generate a seed for it
        QPair<QStringList, QString> start2init(const QString & password, int seedLength);

        // Recover the wallet with a mnemonic phrase
        // recover wallet with a passphrase. Scan call needs to be done.
        // Return: Error
        QString start2recover(const QStringList & seed, const QString & password);

        // Note, return error include invalid password case as well
        // Return: Error
        QString loginWithPassword(const QString & password);

        QVector<Account> listAccounts() const;

        void switchAccountById(const QString & accountPath);

        void setReceiveAccountById(const QString & accountPath);

        bool checkPassword(const QString & password) const;

        QPair<QStringList, QString> getSeed(const QString & walletPassword) const;

        // Checking if wallet is listening through services
        ListenerStatus getListenerStatus() const;

        // Start listening through services
        void listeningStart(bool startMq, bool startTor);

        // Stop listening through services
        // return:
        //    true if the task was schediled and will be executed.
        //    false if nothing needs to be done. Everything is stopped
        bool listeningStop(bool stopMq, bool stopTor);

        // Request MQS address
        QString getMqsAddress() const;

        // Request Tor address
        QString getTorSlatepackAddress() const;

        // requst current address index
        int getAddressIndex() const;

        // Note, set address index does update the MQS and Tor addresses
        // The Listeners, if running, will be restarted automatically
        void setAddressIndex(int index);

        // Get all accounts with balances.
        QVector<AccountInfo> getWalletBalance(int confirmations, bool filterDeleted, const QStringList & manuallyLockedOutputs) const;

        QString getCurrentAccountId() const;

        // Create another account, note no delete exist for accounts
        QString createAccount( const QString & accountName );

        // Rename account
        void renameAccountById( const QString & accountPath, const QString & newAccountName);

        // Check and repair the wallet. Will take a while
        // Check Signal: onScanProgress, onScanDone
        // Return responseId
        QString scan(bool delete_unconfirmed);

        // Update the wallet state, resync with a current node state
        // Check Signal: onScanProgress, onScanDone
        // Return responseId
        QString update_wallet_state();

        QString getReceiveAccountPath() const;

        // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
        // Return error or JsonObject
        QString generateTransactionProof( const QString & transactionUuid );

        // Verify the proof for transaction
        // Return error or JsonObject
        QString verifyTransactionProof(const QString & proof) const;

        // Send some coins to address. Return false if another send is already in progress. Need to wait more, one send at a time
        // Before send, wallet always do the switch to account to make it active
        // Check signal:  onSend
        bool sendTo( const QString &accountPathFrom,
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
                        qint64 min_fee);  // 0 or negative to skip. Currently no needs to define it

        // Receive slatepack. Will generate the resulting slatepack.
        // Return: <SlatePack, Error>
        // In case of error SlatePack is empty
        QPair<ResReceive,QString> receiveSlatepack( QString slatePack, QString description);

        // finalize SP transaction and broadcast it
        // Return error message
        QString finalizeSlatepack( const QString & slatepack, bool fluff, bool nopost );

        // submit finalized transaction. Make sense for cold storage => online node operation
        // Return Error message
        QString submitFile( QString fileTx, bool fluff );

        // Show outputs for the wallet
        QVector<WalletOutput> getOutputs(const QString & accountPath, bool show_spent);

        // Show all transactions for current account
        QVector<WalletTransaction> getTransactions( const QString & accountPath ) const;

        WalletTransaction getTransactionByUUID( const QString & tx_uuid ) const;

        // Cancelt TX by UUID (in case of multi accounts, we want to cancel both)
        // Return error string
        QString  cancelTransacton(const QString & txUUID);

        // True if transaction was finalized and can be reposted
        bool hasFinalizedData(const QString & txUUID) const;

        // Repost the transaction.
        // Retunr Error
        QString repostTransaction(const QString & txUUID, bool fluff);

        // Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
        DecodedSlatepack decodeSlatepack(const QString & slatepackContent) const;

        // Request rewind hash
        QString viewRewindHash() const;

        // Scan with revind hash. That will generate bunch or messages similar to scan
        // Check Signal: onScanProgress
        // Check Signal: onScanRewindHash( QString responseId, ViewWallet walletOutputs, QString errors );
        // Return responseId
        QString scanRewindHash( const QString & rewindHash );

        // Generate ownership proof
        QJsonObject generateOwnershipProof(const QString & message, bool includeRewindHash, bool includeTorAddress, bool includeMqsAddress );

        // Validate ownership proof
        OwnershipProofValidation validateOwnershipProof(const QJsonObject & proof);

        bool isNodeHealthy();
        bool isUsePublicNode() const;
        qint64 getLastNodeHeight() const;
        QString getLastInternalNodeState() const;

        NodeStatus requestNodeStatus() const;
    public:
        Wallet  * parentWallet = nullptr;

        // Node and node client can be taken out.
        // But this it an owner of those objects.
        std::shared_ptr<node::NodeClient> nodeClient = nullptr;

        int context_id = -1;
        bool open = false;
        QString node_client_callback_name;
        QString update_status_callback_name;
        QString network;
        QString walletDataPath;
        bool mqs_running = false;
        bool tor_running = false;

        // running tasks
        QFuture<void> restart_listeners;
        int nextListenersTask = 0;

        QAtomicInt response_id_counter = QAtomicInt(0);

        QFuture<void> scanOp;
        QAtomicInt scanInProgress = QAtomicInt(0);
        QString lastScanResponseId;

        QFuture<void> sendOp;
        QFuture<void> scanRewindHashOp;
        QFuture<void> fausetRequest;

        // Not owned, just for usage
        QFuture<QString> * torStarter;


    private:
        void startNextStartStopListeners();
        void restartRunningListeners();
    };
}

#endif //MWC_QT_WALLET_WALLET_INTERNALS_H