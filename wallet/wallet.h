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

#ifndef MWCWALLET_H
#define MWCWALLET_H

#include "../wallet_features.h"
#include <QMutex>

#include "wallet_internals.h"
#include "wallet_objs.h"
#include "tasks/TorEmbeddedStarter.h"

namespace core {
class AppContext;
}

namespace node {
class MwcNode;
class NodeClient;
}

namespace wallet {

// Interface to wallet functionality
class Wallet : public QObject
{
    Q_OBJECT
public:
    // network: main | floo
    Wallet(QFuture<QString> * torStarter);
    virtual ~Wallet();

    bool isBusy() const;

    // Init the instance for a starting/created wallet
    // Return error
    QString init(QString network, QString walletDataPath, std::shared_ptr<node::NodeClient> nodeClient);

    int getWalletId();

    // Return true if wallet is init, so API calls can be used
    bool isInit() const;

    QString getWalletDataPath() const;

    // Create new wallet and generate a seed for it. Return empty seed in case of error
    // Return: Seed, Error
    QPair<QStringList, QString> start2init(const QString & password, int seedLength);

    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase. Scan call needs to be done.
    // Return: Error
    QString start2recover(const QStringList & seed, const QString & password);

    // Note, return error include invalid password case as well
    // Return: Error
    // Signal:  onLogin
    QString loginWithPassword(const QString & password);

    // Return true if wallet has this password. Wallet might not have password (has empty password) if it was created manually.
    // Expected that the wallet is already open,
    bool checkPassword(const QString & password) const;

    // Exit from the wallet. Expected that state machine will switch to Init state
    // Signal:  onLogout
    void logout();

    // Current seed for runnign wallet
    QPair<QStringList, QString> getSeed(const QString & walletPassword);

    //--------------- Listening

    // Request restart for online services if needed (don't restart if offline), must be done in a background
    void restartRunningListeners();

    // Checking if wallet is listening through services
    ListenerStatus getListenerStatus();

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

    // request current address index
    int getAddressIndex() const;

    // Note, set address index does update the MQS and Tor addresses
    // The Listeners, if running, will be restarted automatically
    void setAddressIndex(int index);

    // -------------- Accounts

    // NOTE!!!:  It is child implemenation responsibility to process Outputs Locking correctly, so it looks
    //    like wallet initiate balance updates because of that!!!
    // Currently mwc713 taking care about that

    // Get all accounts with balances.
    QVector<AccountInfo> getWalletBalance(int confirmations, bool filterDeleted, const QStringList & manuallyLockedOutputs) const;

    QVector<Account> listAccounts() const;

    // return current account path
    QString getCurrentAccountId() const;

    // Create another account, note no delete exist for accounts
    // Return acount path
    QString createAccount( const QString & accountName );

    // Switch to different account
    void switchAccountById(const QString & accountPath);

    // Rename account
    void renameAccountById( const QString & accountPath, const QString & newAccountName);

    // Set account that will receive the funds
    void setReceiveAccountById(const QString & accountPath);
    QString getReceiveAccountPath();

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    // Check Signal: onScanProgress, onScanDone
    // Return responseId
    QString scan(bool delete_unconfirmed);

    // Update the wallet state, resync with a current node state
    // Check Signal: onScanProgress, onScanDone
    // Return responseId
    QString update_wallet_state();

    // Get current configuration of the wallet.
    WalletConfig getWalletConfig() const;
    std::shared_ptr<node::NodeClient> getNodeClient() const;

    NodeStatus requestNodeStatus() const;

    bool isNodeHealthy() const;
    bool isUsePublicNode() const;
    qint64 getLastNodeHeight() const;
    QString getLastInternalNodeState() const;

    // Proof results

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Return error or JsonObject
    QString generateTransactionProof( const QString & transactionUuid );

    // Verify the proof for transaction
    // Return error or JsonObject
    QString verifyTransactionProof( const QString & proof );

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
                qint64 min_fee); // 0 or negative to skip. Currently no needs to define it

    // Receive slatepack. Will generate the resulting slatepack.
    // Return: <SlatePack, Error>
    QPair<ResReceive,QString> receiveSlatepack( QString slatePack, QString description);

    // finalize SP transaction and broadcast it
    // Return erro message
    QString finalizeSlatepack( const QString & slatepack, bool fluff, bool nopost );

    // submit finalized transaction. Make sense for cold storage => online node operation
    // Return Error message
    QString submitFile( QString fileTx, bool fluff );

    // Show outputs for the wallet
    QVector<WalletOutput> getOutputs(const QString & accountPath, bool show_spent) const;

    // Show all transactions for current account
    QVector<WalletTransaction> getTransactions( const QString & accountPath ) const;

    // Request single transaction by UUID, any account
    WalletTransaction getTransactionByUUID( const QString & tx_uuid ) const;

    // True if transaction was finalized and can be reposted
    bool hasFinalizedData(const QString & txUUID) const;

    // Cancel TX by UUID (in case of multi accouns, we want to cancel both)
    // Return error string
    QString cancelTransacton(const QString & txUUID);

    // Repost the transaction.
    // Return Error
    QString repostTransaction(const QString & txUUID, bool fluff);

    // Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
    DecodedSlatepack decodeSlatepack(const QString & slatepackContent) const;


    // ---------------- Swaps -------------

#ifdef FEATURE_SWAP
    // Request all running swap trades.
    QVector<SwapInfo> requestSwapTrades() { Q_ASSERT(false); return QVector<SwapInfo>(); }

    // Delete the swap trade
    QString deleteSwapTrade(const QString & swapId)  { Q_UNUSED(swapId); Q_ASSERT(false); return ""; }

    // Create a new Swap trade deal.
    // Return SwapID
    QString createNewSwapTrade(const QString & accountPath,
                                    QVector<QString> outputs, // If defined, those outputs will be used to trade. They might belong to another trade, that if be fine.
                                    int min_confirmations, // minimum number of confimations
                                    const QString & mwcAmount, const QString & secAmount, const QString & secondary,
                                    const QString & redeemAddress,
                                    double secTxFee,
                                    bool sellerLockFirst,
                                    int messageExchangeTimeMinutes,
                                    int redeemTimeMinutes,
                                    int mwcConfirmationNumber,
                                    int secondaryConfirmationNumber,
                                    const QString & communicationMethod,
                                    const QString & communicationAddress,
                                    const QString & electrum_uri1,
                                    const QString & electrum_uri2,
                                    bool dryRun,
                                    const QString & tag,
                                    const QString & mkt_trade_tag,
                                    const QVector<QString> & params )
    { Q_ASSERT(false); return "";}

    // Cancel the trade
    // Return Error
    QString cancelSwapTrade(const QString & swapId) { Q_ASSERT(false); return "";}


    // Request details about this trade.
    SwapTradeDetails requestTradeDetails(const QString & swapId, bool waitForBackup1) { Q_ASSERT(false); SwapTradeDetails res; return res;}

    // Request Eth Info.
    EthInfo requestEthInfo() { Q_ASSERT(false); EthInfo res; return res;}

    // Request Eth Info.
    // Return Error message
    QString requestEthSend(QString dest,
                                QString currency,
                                QString amount) { Q_ASSERT(false);  return "";}

    // Adjust swap state values. params are optional
    // Return error message
    QString adjustSwapData( const QString & swapId,
                                 const QString &destinationMethod, const QString & destinationDest,
                                 const QString &secondaryAddress,
                                 const QString &secondaryFee,
                                 const QString &electrumUri1,
                                 const QString &tag ) {Q_ASSERT(false); return "";}

    // Perform a auto swap step for this trade.
    AutoSwapStepResult performAutoSwapStep( QString swapId, bool waitForBackup1 ) {Q_ASSERT(false); AutoSwapStepResult res; return res;}

    // Backup/export swap trade data file
    // Response with error message
    QString backupSwapTradeData(QString swapId, QString backupFileName) { Q_ASSERT(false); return ""; }

    // Restore/import swap trade from the file
    // Return <swapId, ErrorMessage>
    QPair<QString, QString> restoreSwapTradeData(QString filename) { Q_ASSERT(false); return QPair<QString, QString>(); }

    // Adjust trade state. It is dev support functionality, so no feedback will be provided.
    // In case you need it, add the signal as usual
    void adjustTradeState(QString swapId, QString newState) { Q_ASSERT(false);}
#endif
#ifdef FEATURE_MKTPLACE
    // Pay fees, validate fees.
    QVector<IntegrityFees> createIntegrityFee( const QString & account, double mwcReserve, const QVector<double> & fees ) { Q_ASSERT(false); return QVector<IntegrityFees>();}

    // Request info about paid integrity fees
    // Check Signal: onRequestIntegrityFees(QString error, qint64 balance, QVector<wallet::IntegrityFees> fees)
    QVector<IntegrityFees> requestIntegrityFees()  { Q_ASSERT(false); return QVector<IntegrityFees>();}

    // Request withdraw for available deposit at integrity account. Return error
    QString withdrawIntegrityFees(const QString & account) { Q_ASSERT(false); return "";}

    // Status of the messaging
    MessagingStatus requestMessagingStatus() {Q_ASSERT(false); MessagingStatus res; return res; }

    // Publish new json message
    // Return: QString id, QString uuid, QString error)
    void messagingPublish(QString messageJsonStr, QString feeTxUuid, QString id, int publishInterval, QString topic) {Q_ASSERT(false);}

    // Check integrity of published messages.
    // return expiredMsgUuid
    QVector<QString>  checkIntegrity() {Q_ASSERT(false); return QVector<QString>();}

    // Stop publishing the message
    // return error
    QString messageWithdraw(QString uuid) {Q_ASSERT(false); return "";}

    // Request messages from the receive buffer
    QVector<ReceivedMessages> requestReceiveMessages(bool cleanBuffer) {Q_ASSERT(false); return QVector<ReceivedMessages>();}

    // Start listening on the libp2p topic
    // Return Error
    QString startListenOnTopic(const QString & topic) {Q_ASSERT(false);  return "";}

    // Stop listening on the libp2p topic
    QString stopListenOnTopic(const QString & topic) {Q_ASSERT(false);  return "";}

    // Send marketplace message and get a response back
    // command: "accept_offer" or "fail_bidding"
    void sendMarketplaceMessage(QString command, QString wallet_tor_address, QString offer_id, QString cookie) {Q_ASSERT(false);}
#endif

    // Get rewind hash
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

    int getContextId() const;

    bool isMqsRunning() const;
    bool isTorRunning() const;

    // Sync long call, wallet will process QT events and show Success/Error Dlg
    void requestFaucetMWC();
    void mwcFromFlooFaucetDone(bool success, QString errorMsg);

    // Note, Done methods called from another thread.
    void startStopListenersDone(int operation);
    void scanDone(QString responseId, bool fullScan, int height, QString errorMessage );

    void emitStatusUpdate( const QString & response_id, const QJsonObject & status );

    void sendDone( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag );

    void scanRewindDone( const QString & responseId, const ViewWallet & result, const QString & error );

    void emitSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message );

    // Account info is updated
    void emitWalletBalanceUpdated();
private:
signals:
    void onConfigUpdate();
    void onLogin();
    void onLogout();

    // Scanning progress
    void onScanProgress( QString responseId, QJsonObject statusMessage );
    void onScanDone( QString responseId, bool fullScan, int height, QString errorMessage );
    void onStartStopListenersDone(int operation);

    // Send results
    void onSend( bool success, QString error, QString tx_uuid, qint64 amount, QString method, QString dest, QString tag );

    // I get money
    void onSlateReceivedFrom(QString slate, qint64 mwc, QString fromAddr, QString message );

    // response from scanRewindHash
    void onScanRewindHash( QString responseId, ViewWallet walletOutputs, QString errors );

    // Account info might be updated (we just got a new block and finish the scan)
    void onWalletBalanceUpdated();

    void onFaucetMWCDone(bool success);
private:
    void reportWalletFatalError( QString message ) const;

    void startNextStartStopListeners();

    void release();
private:
    WalletInternals * internals = nullptr;

    // Outside task, we don't manage it, just reading
    QFuture<QString> * torStarter;

    QMap<QString, QFuture<void>> releaseTasks;
};

}


#endif // MWCWALLET_H
