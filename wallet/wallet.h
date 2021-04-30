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

#include <QVector>
#include <QString>
#include "../util/ioutils.h"
#include "../util/stringutils.h"
#include <QDateTime>
#include <QObject>

namespace core {
class AppContext;
}

namespace node {
class MwcNode;
}


namespace wallet {

struct AccountInfo {
    QString accountName = "default";
    int64_t height = 0;
    // in nano coins
    int64_t total = 0;
    int64_t awaitingConfirmation = 0;
    int64_t lockedByPrevTransaction = 0;
    int64_t currentlySpendable = 0;

    bool mwcServerBroken = true;

    void setData(QString account,
        int64_t total,
        int64_t awaitingConfirmation,
        int64_t lockedByPrevTransaction,
        int64_t currentlySpendable,
        int64_t height,
        bool mwcServerBroken);

    QString getLongAccountName() const;
    QString getSpendableAccountName() const;

    bool canDelete() const {return total == 0 && awaitingConfirmation==0 && lockedByPrevTransaction==0 && currentlySpendable==0;}

    // return true is this account can be concidered as deleted
    bool isDeleted() const;

    bool isAwaitingSomething() const {return awaitingConfirmation>0 || lockedByPrevTransaction>0; }

    // Debug/Log printing
    QString toString() const;

    bool balancesAreEquals(const AccountInfo & accInfo) const;
};

struct MwcNodeConnection {
    enum class NODE_CONNECTION_TYPE { CLOUD = 0, LOCAL = 1, CUSTOM = 2 }; //

    NODE_CONNECTION_TYPE connectionType = NODE_CONNECTION_TYPE::CLOUD;

    // Cloud - no data

    // Local:
    QString localNodeDataPath = "mwc-node"; // location for the node data

    // Custom
    QString mwcNodeURI; // URL
    QString mwcNodeSecret; // Secret for the node

    void setAsCloud() { connectionType = NODE_CONNECTION_TYPE::CLOUD; }
    void setAsLocal( const QString & _localNodeDataPath ) { connectionType = NODE_CONNECTION_TYPE::LOCAL; localNodeDataPath = _localNodeDataPath; }
    void setAsCustom( const QString & _mwcNodeURI, const QString & _mwcNodeSecret ) { connectionType = NODE_CONNECTION_TYPE::CUSTOM; mwcNodeURI = _mwcNodeURI; mwcNodeSecret = _mwcNodeSecret; }

    bool operator == (const MwcNodeConnection & itm) const {return connectionType==itm.connectionType && mwcNodeURI==itm.mwcNodeURI &&
                mwcNodeSecret==itm.mwcNodeSecret && localNodeDataPath==itm.localNodeDataPath; }

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);

    bool notCustom() const { return connectionType != NODE_CONNECTION_TYPE::CUSTOM; }
    bool isLocalNode() const { return connectionType == NODE_CONNECTION_TYPE::LOCAL; }
    bool isCloudNode() const { return connectionType == NODE_CONNECTION_TYPE::CLOUD; }

    QString toJson();
    static MwcNodeConnection fromJson(const QString & str);
private:
    void setData(NODE_CONNECTION_TYPE connectionType, const QString & localNodeDataPath,
                    const QString & mwcNodeURI,  const QString & mwcNodeSecret);
};

// Wallet config
struct WalletConfig {
private:
    QString network; // Name of the network as wallet understand: "Floonet" or "Mainnet"
    QString dataPath;

public:
    QString mwcmqsDomainEx;// empty - default value

    // Http listening params...
    bool foreignApi = false; // Is foreign API is enabled
    QString foreignApiAddress; // Example: 0.0.0.0:3416
    // For https configuration.
    QString tlsCertificateFile;
    QString tlsCertificateKey;

    WalletConfig() : network("Mainnet"), dataPath("Undefined") {}
    WalletConfig(const WalletConfig & other) = default;
    WalletConfig &operator = (const WalletConfig & other) = default;

    bool operator == (const WalletConfig & other) const;

    bool isDefined() const { return  dataPath!="Undefined"; }

    WalletConfig & setData(QString network,
                QString dataPath,
                QString mwcmqsDomain,
                bool foreignApi,
                QString foreignApiAddress,
                QString tlsCertificateFile,
                QString tlsCertificateKey);

    WalletConfig & setForeignApi(bool foreignApi,
                       QString foreignApiAddress,
                       QString tlsCertificateFile, QString tlsCertificateKey);

    WalletConfig & setDataWalletCfg(QString mwcmqsDomain);

    void updateDataPath(const QString & path) {dataPath=path;}
    void updateNetwork(const QString & mw) { Q_ASSERT(!mw.isEmpty()); network=mw; }

    bool hasForeignApi() const { return foreignApi && !foreignApiAddress.isEmpty(); }


    bool hasTls() const {return !tlsCertificateFile.isEmpty() && !tlsCertificateKey.isEmpty(); }

    const QString & getDataPath() const {return dataPath;}
    const QString & getNetwork() const {return network;}

    // Get MQ/MQS host normalized name. Depend on current config
    QString getMwcMqHostNorm() const;

    // Get MQ/MQS host full name. Depend on current config
    QString getMwcMqHostFull() const;

    // First [Network, Arch, InstanceName]
    static QVector<QString> readNetworkArchInstanceFromDataPath(QString configPath, core::AppContext * context); // local path as writen in config
    static void saveNetwork2DataPath(QString configPath, QString network, QString arch, QString instanceName); // Save the network into the data path

    static bool    doesSeedExist(QString configPath);

    QString toString() const;
};

struct WalletOutput {

    QString    outputCommitment;
    QString    MMRIndex;
    QString    blockHeight;
    QString    lockedUntil;
    QString    status;
    bool       coinbase = false;
    QString    numOfConfirms;
    int64_t    valueNano = 0L;
    int64_t    txIdx;
    double     weight = 0.0; // HODL weight, used for ouptus optimization

    void setData(QString outputCommitment,
            QString     MMRIndex,
            QString     blockHeight,
            QString     lockedUntil,
            QString     status,
            bool        coinbase,
            QString     numOfConfirms,
            int64_t     valueNano,
            int64_t     txIdx);

    static WalletOutput create(QString outputCommitment,
                               QString     MMRIndex,
                               QString     blockHeight,
                               QString     lockedUntil,
                               QString     status,
                               bool        coinbase,
                               QString     numOfConfirms,
                               int64_t     valueNano,
                               int64_t     txIdx) {
        WalletOutput item;
        item.setData(outputCommitment,
                MMRIndex, blockHeight, lockedUntil,
                status, coinbase, numOfConfirms,
                valueNano, txIdx);
        return item;
    }

    QString toString() const;

    bool isValid() const {
        return !(outputCommitment.isEmpty() || status.isEmpty());
    }

    double getWeightedValue() const {return weight*valueNano; }

    bool isUnspent() const {return status == "Unspent";}

    QString toJson() const;
    static WalletOutput fromJson(QString str);
};

struct WalletTransaction {
    enum TRANSACTION_TYPE { NONE=0, SEND=1, RECEIVE=2, COIN_BASE=4, CANCELLED=0x8000};

    int64_t    txIdx = -1;
    uint    transactionType = TRANSACTION_TYPE::NONE;
    QString txid; // Full tx UUID
    QString address;
    QString creationTime;
    int64_t ttlCutoffHeight = -1;
    bool    confirmed = false;
    int64_t height = 0;
    QString confirmationTime;
    int     numInputs = -1;
    int     numOutputs = -1;
    int64_t credited = -1;
    int64_t debited = -1;
    int64_t fee = -1;
    int64_t coinNano = 0; // Net diffrence, transaction weight
    bool    proof=false;
    QString kernel;

    static QString csvHeaders;

    void setData(int64_t txIdx,
                      uint    transactionType,
                      QString txid,
                      QString address,
                      QString creationTime,
                      bool    confirmed,
                      int64_t ttlCutoffHeight,
                      int64_t height,
                      QString confirmationTime,
                      int     numInputs,
                      int     numOutputs,
                      int64_t credited,
                      int64_t debited,
                      int64_t fee,
                      int64_t coinNano,
                      bool    proof,
                      QString kernel);


    bool isValid() const {return txIdx>=0 && transactionType!=TRANSACTION_TYPE::NONE;}

    bool canBeCancelled() const { return (transactionType & TRANSACTION_TYPE::CANCELLED)==0 && !confirmed; }

    bool isCoinbase() const { return transactionType==TRANSACTION_TYPE::COIN_BASE; }

    // return transaction age (time interval from creation moment) in Seconds.
    int64_t calculateTransactionAge( const QDateTime & current ) const;

    // mark transaction as cancelled
    void cancelled() {
        transactionType |= TRANSACTION_TYPE::CANCELLED;
    }

    QString getTypeAsStr() const {
        QString res;

        if ( transactionType & TRANSACTION_TYPE::SEND )
            res += "Send";
        if ( transactionType & TRANSACTION_TYPE::RECEIVE )
            res += "Receive";
        if ( transactionType & TRANSACTION_TYPE::COIN_BASE )
            res += "CoinBase";

        if ( transactionType & TRANSACTION_TYPE::CANCELLED ) {
            if (!res.isEmpty())
                res += ", ";
            res += "Cancelled";
        }

        return res;
    }

    QString toStringShort() {
        using namespace util;

        return expandStrR( QString::number(txIdx), 3) +
                expandStrR(nano2one(coinNano), 8) +
                expandStrR( string2shortStrR(txid, 12), 12) +
                " " + creationTime;
    }

    static QString getCSVHeaders(const QStringList & extraHeaders) {
        return csvHeaders + (extraHeaders.isEmpty() ? "" : "," + extraHeaders.join(","));
    }

    // return transactions values formatted into a CSV string
    // Caller is responsible to make extraData to be CSV compatible
    QString toStringCSV(const QStringList & extraData) const;

    QString toJson() const;
    static WalletTransaction fromJson(QString str);
};

struct WalletUtxoSignature {
    int64_t coinNano = 0; // Output amount
    QString messageHash;
    QString pubKeyCompressed;
    QString messageSignature;

    void setData(int64_t _coinNano, // Output amount
            QString _messageHash,
            QString _pubKeyCompressed,
            QString _messageSignature);
};

struct SwapInfo {
    QString mwcAmount;
    QString secondaryAmount;
    QString secondaryCurrency;
    QString swapId;
    QString tag;
    int64_t startTime = 0;
    QString stateCmd; // state as command
    QString state; // State as string description
    QString action;
    int64_t expiration = 0;
    bool    isSeller;
    QString secondaryAddress;
    QString lastProcessError;

    void setData( QString mwcAmount, QString secondaryAmount, QString secondaryCurrency,
                  QString swapId, QString tag, int64_t startTime, QString stateCmd, QString state, QString action,
                  int64_t expiration, bool isSeller, QString secondaryAddress, QString lastProcessError );
};

struct SwapTradeInfo {
    QString swapId;
    QString tag;
    bool isSeller;
    double mwcAmount;
    double secondaryAmount;
    QString secondaryCurrency;
    QString secondaryAddress; // redeem/refund address

    double secondaryFee;
    QString secondaryFeeUnits;

    int mwcConfirmations;
    int secondaryConfirmations;
    int messageExchangeTimeLimit;
    int redeemTimeLimit;
    bool sellerLockingFirst;
    int mwcLockHeight;
    int64_t mwcLockTime;
    int64_t secondaryLockTime;

    QString communicationMethod;
    QString communicationAddress;

    QString electrumNodeUri; // Private electrumX URI


    void setData( QString swapId, QString tag, bool isSeller,  double mwcAmount, double secondaryAmount,
                QString secondaryCurrency,  QString secondaryAddress, double secondaryFee,
                QString secondaryFeeUnits, int mwcConfirmations, int secondaryConfirmations,
                int messageExchangeTimeLimit, int redeemTimeLimit, bool sellerLockingFirst,
                int mwcLockHeight, int64_t mwcLockTime, int64_t secondaryLockTime,
                QString communicationMethod, QString communicationAddress, QString electrumNodeUri );
};

struct SwapExecutionPlanRecord {
    bool active = false;
    int64_t end_time = 0;
    QString name;

    void setData( bool active, int64_t end_time, QString name );
};

struct SwapJournalMessage {
    QString message;
    int64_t time = 0;

    void setData( QString message, int64_t time );
};

// Some startus booleans for 3 listeners
struct ListenerStatus {
    bool mqs = false;
    bool tor = false;

    ListenerStatus() = default;
    ListenerStatus(bool _mqs, bool _tor) : mqs(_mqs), tor(_tor) {}

    ListenerStatus(const ListenerStatus & item) = default;
    ListenerStatus & operator = (const ListenerStatus & item) = default;
};

struct IntegrityFees {
    bool confirmed = false;
    int64_t expiration_height = -1;
    int64_t ask_fee = -1;
    int64_t fee = -1;
    QString uuid;

    IntegrityFees() = default;
    IntegrityFees(const IntegrityFees & item) = default;
    IntegrityFees & operator = (const IntegrityFees & item) = default;

    IntegrityFees(bool _confirmed,
        int64_t _expiration_height,
        int64_t _ask_fee,
        int64_t _fee,
        QString _uuid) :
            confirmed(_confirmed),
            expiration_height(_expiration_height),
            ask_fee(_ask_fee),
            fee(_fee),
            uuid(_uuid)
        {}

    IntegrityFees(QString jsonString);
    IntegrityFees(QJsonObject json);

    double toDblFee() const {return fee / 1000000000.0;}

    QJsonObject toJSon() const;
    QString toJSonStr() const;
};

struct BroadcastingMessage {
    QString uuid;
    int broadcasting_interval = -1;
    int64_t fee = 0;
    QString message;
    int published_time;

    // {"broadcasting_interval":60,"fee":"10000000","message":"{}","published_time":49,"uuid":"7f0a6a89-5ad5-40cb-b204-0805ffcd1903"}
    BroadcastingMessage(const QJsonObject & json);
    BroadcastingMessage() = default;
    BroadcastingMessage(const BroadcastingMessage & item) = default;
    BroadcastingMessage & operator = (const BroadcastingMessage & item) = default;
};

struct MessagingStatus {
    QVector<BroadcastingMessage> broadcasting;
    bool connected = false;
    QStringList gossippub_peers;
    int received_messages = 0;
    QStringList topics;

    // {"broadcasting":[{"broadcasting_interval":60,"fee":"10000000","message":"{}","published_time":49,"uuid":"7f0a6a89-5ad5-40cb-b204-0805ffcd1903"}],"gossippub_peers":null,"received_messages":0,"topics":["swapmarketplace","testing"]}
    MessagingStatus(const QJsonObject & json);
    MessagingStatus() = default;
    MessagingStatus(const MessagingStatus & item) = default;
    MessagingStatus & operator = (const MessagingStatus & item) = default;

    // Status for logs
    QString toString() const;

    bool isConnected() const {return connected && gossippub_peers.size()>=3; }
};

struct ReceivedMessages {
    QString topic;   // Topic where we received the message
    int64_t fee;     // fee that was paid
    QString message; // message that received
    QString wallet;  // wallet onion address
    int64_t timestamp;

    ReceivedMessages(QString _topic,
            int64_t _fee,
            QString _message,
            QString _wallet,
            int64_t _timestamp) :
            topic(_topic),
            fee(_fee),
            message(_message),
            wallet(_wallet),
            timestamp(_timestamp) {}

    ReceivedMessages() = default;
    ReceivedMessages(const ReceivedMessages & item) = default;
    ReceivedMessages & operator = (const ReceivedMessages & item) = default;
};

// Interface to wallet functionality
class Wallet : public QObject
{
    Q_OBJECT
public:
    // network: main | floo
    Wallet();
    virtual ~Wallet();

    // Return true if wallet is running
    virtual bool isRunning() = 0;

    // Just a helper method
    virtual bool isWalletRunningAndLoggedIn() const = 0;

    // Check if wallet need to be initialized or not. Will run standalone app, wait for exit and return the result
    // Call might take few seconds
    virtual bool checkWalletInitialized(bool hasSeed) = 0;

    enum STARTED_MODE { OFFLINE, NORMAL, INIT, RECOVER };
    virtual STARTED_MODE getStartedMode() = 0;

    // ---- Wallet Init Phase
    virtual void start()   = 0;
    // Create new wallet and generate a seed for it
    // Check signal: onNewSeed( seed [] )
    virtual void start2init(QString password) = 0;
    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );
    virtual void start2recover(const QVector<QString> & seed, QString password)   = 0;

    // Check signal: onLoginResult(bool ok)
    virtual void loginWithPassword(QString password)   = 0;

    // Return true if wallet has password. Wallet might not have password if it was created manually.
    virtual bool hasPassword() const  = 0;

    // Exit from the wallet. Expected that state machine will switch to Init state
    // syncCall - stop NOW. Caller suppose to understand what he is doing
    virtual void logout(bool syncCall) = 0;

    // Confirm that user write the passphase
    // SYNC command
    virtual void confirmNewSeed()  = 0;


    // Current seed for runnign wallet
    // Check Signals: onGetSeed(QVector<QString> seed);
    virtual void getSeed(const QString & walletPassword)  = 0;

    // Get last used password. Just don't export from DLL
    virtual QString getPasswordHash() = 0;

    //--------------- Listening

    // Checking if wallet is listening through services
    virtual ListenerStatus getListenerStatus()  = 0;
    virtual ListenerStatus getListenerStartState()  = 0;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startTor, bool initialStart)  = 0;
    // Check Signal: onListeningStartResults

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopTor)  = 0;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getMqsAddress()  = 0;
    // Get latest Tor address that we see
    virtual QString getTorAddress()  = 0;

    // Request proof address for files
    // Check signal onFileProofAddress(QString address);
    virtual void requestFileProofAddress() = 0;

    // Get MWC box <address, index in the chain>
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
    virtual void getMwcBoxAddress()  = 0;

    // Change MWC box address to another from the chain. idx - index in the chain.
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
    virtual void changeMwcBoxAddress(int idx)  = 0;

    // Generate next box address
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
    virtual void nextBoxAddress()  = 0;

    // Request http(s) listening status.
    // bool - true is listening. Then next will be the address
    // bool - false, not listening. Then next will be error or empty if listening is not active.
    // Check signal: onHttpListeningStatus(bool listening, QString additionalInfo)
    virtual QPair<bool, QString> getHttpListeningStatus() const = 0;

    // Return true if Tls is setted up for the wallet for http connections.
    virtual bool hasTls() const = 0;

    // -------------- Accounts

    // NOTE!!!:  It is child implemenation responsibility to process Outputs Locking correctly, so it looks
    //    like wallet initiate balance updates because of that!!!
    // Currently mwc713 taking care about that

    // Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
    // This info needed in many cases and we don't want spend time every time for that.
    virtual QVector<AccountInfo> getWalletBalance(bool filterDeleted = true) const  = 0;

    // Get outputs that was collected for this wallet. Outputs should be ready with balances
    virtual const QMap<QString, QVector<wallet::WalletOutput> > & getwalletOutputs() const = 0;

    virtual QString getCurrentAccountName()  = 0;

    // Request Wallet balance update. It is a multistep operation
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress
    virtual void updateWalletBalance(bool enforceSync, bool showSyncProgress, bool skipSync=false)  = 0;


    // Create another account, note no delete exist for accounts
    // Check Signal:  onAccountCreated
    virtual void createAccount( const QString & accountName )  = 0;

    // Switch to different account
    virtual void switchAccount(const QString & accountName)  = 0;

    // Rename account
    // Check Signal: onAccountRenamed(bool success, QString errorMessage);
    virtual void renameAccount(const QString & oldName, const QString & newName)  = 0;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onCheckResult(bool ok, QString errors );
    virtual void check(bool wait4listeners)  = 0;

    // Get current configuration of the wallet.
    virtual const WalletConfig & getWalletConfig()  = 0;

    // Get configuration form the resource file.
    virtual const WalletConfig & getDefaultConfig()  = 0;

    // Update wallet config. Will update config and restart the mwc713.
    // Note!!! Caller is fully responsible for input validation. Normally mwc713 will sart, but some problems might exist
    //          and caller suppose listen for them
    // If returns true, expected that wallet will need to have password input.
    // Check signal: onConfigUpdate()
    virtual bool setWalletConfig(const WalletConfig & config, bool canStartNode )  = 0;

    // Status of the node
    // return true if task was scheduled
    // Check Signal: onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections )
    virtual bool getNodeStatus() = 0;

    // Set account that will receive the funds
    // Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
    virtual void setReceiveAccount(QString account)  = 0;
    virtual QString getReceiveAccount() = 0;

    // Proof results

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Check Signal: onExportProof( bool success, QString fn, QString msg );
    virtual void generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName )  = 0;

    // Verify the proof for transaction
    // Check Signal: onVerifyProof( bool success, QString msg );
    virtual void verifyMwcBoxTransactionProof( QString proofFileName )  = 0;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( const QString &account, int64_t coinNano, QString message, QString fileTx,
            int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, int ttl_blocks, bool generateProof )  = 0;

    // Receive transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx, QString description, QString identifier = "")  = 0;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse, bool fluff )  = 0;

    // Init send transaction with file output
    // Check signal:  onSendSlatepack
    virtual void sendSlatepack( const QString &account, int64_t coinNano, QString message,
                           int inputConfirmationNumber, int changeOutputs, const QStringList & outputs,
                           int ttl_blocks, bool generateProof,
                           QString slatepackRecipientAddress, // optional. Encrypt SP if it is defined.
                           bool isLockLater,
                           QString tag )  = 0;

    // Receive transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveSlatepack
    virtual void receiveSlatepack( QString slatePack, QString description, QString tag)  = 0;

    // finalize transaction and broadcast it
    // Check signal:  onFinalizeSlatepack
    virtual void finalizeSlatepack( QString slatepack, bool fluff, QString tag ) = 0;

    // submit finalized transaction. Make sense for cold storage => online node operation
    // Check Signal: onSubmitFile(bool ok, String message)
    virtual void submitFile( QString fileTx )  = 0;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    // coinNano == -1  - mean All
    // Check signal:  onSend
    virtual void sendTo( const QString &account, int64_t coinNano, const QString & address, const QString & apiSecret,
                         QString message, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, bool fluff, int ttl_blocks, bool generateProof, QString expectedproofAddress )  = 0;

    // Airdrop special. Generating the next Public key for transaction
    // wallet713> getnextkey --amount 1000000
    // "Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)"
    // Check Signal: onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);
    virtual void getNextKey( int64_t amountNano, QString btcaddress, QString airDropAccPassword ) = 0;


    // Show outputs for the wallet
    // Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> outputs)
    virtual void getOutputs(QString account, bool show_spent, bool enforceSync)  = 0;

    // Show all transactions for current account
    // Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
    virtual void getTransactions(QString account, bool enforceSync)  = 0;

    // get Extended info for specific transaction
    // Check Signal: onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages )
    virtual void getTransactionById(QString account, QString txIdxOrUUID ) = 0;

    // Get wallet balance
    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(QString account, int64_t txIdx)  = 0;

    // ----------- HODL
    // https://github.com/mimblewimble/grin/pull/2374

    // sign output commitment utxo. Hash must be provivded by party that want to verify the signature.
    // Late this signature can be used for verification of ounewship
    //virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) = 0;

    // Get root public key with signed message. Message is optional, can be empty
    // Check Signal: onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature );
    virtual void getRootPublicKey( QString message2sign ) = 0;

    // Repost the transaction. Optionally fluff.
    // index is the tx_index in the tx_log.
    // Check Signal: onRepost(QString error)
    virtual void repost(QString account, int index, bool fluff) = 0;

    // ---------------- Swaps -------------

    // Request all running swap trades.
    // Check Signal: void onRequestSwapTrades(QString cookie, QVector<SwapInfo> swapTrades, QString error);
    virtual void requestSwapTrades(QString cookie) = 0;

    // Delete the swap trade
    // Check Signal: void onDeleteSwapTrade(QString swapId, QString errMsg)
    virtual void deleteSwapTrade(QString swapId) = 0;

    // Create a new Swap trade deal.
    // Check Signal: void onCreateNewSwapTrade(tag, dryRun, QVector<QString> params, QString swapId, QString err);
    virtual void createNewSwapTrade(QString account,
                                    QVector<QString> outputs, // If defined, those outputs will be used to trade. They might belong to another trade, that if be fine.
                                    int min_confirmations, // minimum number of confimations
                                    QString mwcAmount, QString secAmount, QString secondary,
                                    QString redeemAddress,
                                    double secTxFee,
                                    bool sellerLockFirst,
                                    int messageExchangeTimeMinutes,
                                    int redeemTimeMinutes,
                                    int mwcConfirmationNumber,
                                    int secondaryConfirmationNumber,
                                    QString communicationMethod,
                                    QString communicationAddress,
                                    QString electrum_uri1,
                                    QString electrum_uri2,
                                    bool dryRun,
                                    QString tag,
                                    QString mkt_trade_tag,
                                    QVector<QString> params ) = 0;

    // Cancel the trade
    // Check Signal: void onCancelSwapTrade(QString swapId, QString error);
    virtual void cancelSwapTrade(QString swapId) = 0;


    // Request details about this trade.
    // Check Signal: void onRequestTradeDetails( SwapTradeInfo swap,
    //                            QVector<SwapExecutionPlanRecord> executionPlan,
    //                            QString currentAction,
    //                            QVector<SwapJournalMessage> tradeJournal,
    //                            QString error,
    //                            QString cookie );
    virtual void requestTradeDetails(QString swapId, bool waitForBackup1, QString cookie ) = 0;

    // Adjust swap stade values. params are optional
    // Check Signal: onAdjustSwapData(QString swapId, QString call_tag, QString errMsg);
    virtual void adjustSwapData( const QString & swapId, QString call_tag,
                                 const QString &destinationMethod, const QString & destinationDest,
                                 const QString &secondaryAddress,
                                 const QString &secondaryFee,
                                 const QString &electrumUri1,
                                 const QString &tag ) = 0;

    // Perform a auto swap step for this trade.
    // Check Signal: void onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
    //                       QString lastProcessError,
    //                       QVector<SwapExecutionPlanRecord> executionPlan,
    //                       QVector<SwapJournalMessage> tradeJournal,
    //                       QString error );
    virtual void performAutoSwapStep( QString swapId, bool waitForBackup1 ) = 0;

    // Backup/export swap trade data file
    // Check Signal: onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage)
    virtual void backupSwapTradeData(QString swapId, QString backupFileName) = 0;

    // Restore/import swap trade from the file
    // Check Signal: onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);
    virtual void restoreSwapTradeData(QString filename) = 0;

    // Request proff address from http transaction
    // apiSecret - if foreign API secret, optional. Normally it is empty
    // Check signal: onRequestRecieverWalletAddress(QString url, QString address, QString error)
    virtual void requestRecieverWalletAddress(QString url, QString apiSecret) = 0;

    // Adjust trade state. It is dev support functionality, so no feedback will be provided.
    // In case you need it, add the signal as usual
    virtual void adjustTradeState(QString swapId, QString newState) = 0;

    // Decode the slatepack data (or validate slate json) are respond with Slate SJon that can be processed
    // Check Signal: onDecodeSlatepack( QString tag, QString error, QString slateJSon, QString content, QString sender, QString receiver )
    virtual void decodeSlatepack(QString slatepackContent, QString tag) = 0;

    // Pay fees, validate fees.
    // Check signal: onCreateIntegrityFee(QString err, QVector<IntegrityFees> result);
    virtual void createIntegrityFee( const QString & account, double mwcReserve, const QVector<double> & fees ) = 0;

    // Request info about paid integrity fees
    // Check Signal: onRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees)
    virtual void requestIntegrityFees() = 0;

    // Request withdraw for available deposit at integrity account.
    // Check Signal: onWithdrawIntegrityFees(QString error)
    virtual void withdrawIntegrityFees(const QString & account) = 0;

    // Status of the messaging
    // Check Signal: onRequestMessagingStatus(MessagingStatus status)
    virtual void requestMessagingStatus() = 0;

    // Publish new json message
    // Check Signal: onMessagingPublish(QString id, QString uuid, QString error)
    virtual void messagingPublish(QString messageJsonStr, QString feeTxUuid, QString id, int publishInterval, QString topic) = 0;

    // Check integrity of published messages.
    // Check Signal:  onCheckIntegrity(QVector<QString> expiredMsgUuid)
    virtual void checkIntegrity() = 0;

    // Stop publishing the message
    // Check Signal: onMessageWithdraw(QString uuid, QString error)
    virtual void messageWithdraw(QString uuid) = 0;

    // Request messages from the receive buffer
    // Check Signal: onReceiveMessages(QString error, QVector<ReceivedMessages>)
    virtual void requestReceiveMessages(bool cleanBuffer) = 0;

    // Start listening on the libp2p topic
    // Check Signal: onStartListenOnTopic(QString error);
    virtual void startListenOnTopic(const QString & topic) = 0;

    // Stop listening on the libp2p topic
    // Check Signal: onStopListenOnTopic(QString error);
    virtual void stopListenOnTopic(const QString & topic) = 0;

    // Send marketplace message and get a response back
    // command: "accept_offer" or "fail_bidding"
    // Check Signal: onSendMarketplaceMessage(QString error, QString response, QString offerId, QString walletAddress, QString cookie);
    virtual void sendMarketplaceMessage(QString command, QString wallet_tor_address, QString offer_id, QString cookie) = 0;

private:
signals:
    // Wallet doing something. This message is needed for the progress.
    void onStartingCommand(QString actionName);

    // Config was updated
    void onConfigUpdate();

    // Get next key result
    void onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPassword);

    // Result of the login
    void onLoginResult(bool ok);

    // Logout event
    void onLogout();

    // Get MWC updated address. Normally you don't need that
    void onMwcAddress(QString mwcAddress);

    // Get MWC MQ address with index
    void onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Get Tor updated address.
    void onTorAddress(QString torAddress);

    // Get File proof address.
    void onFileProofAddress(QString address);

    // New seed generated by the wallet
    void onNewSeed(QVector<QString> seed);

    // Seed from current account
    // Get empty or size 1 in case of error
    void onGetSeed(QVector<QString> seed);

    void onRecoverProgress( int progress, int maxVal );
    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool tor, // what we try to start
                                   QStringList errorMessages, bool initialStart ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool tor, // what we try to stop
                                QStringList errorMessages );

    // Account info is updated
    void onWalletBalanceUpdated();
    // Progress for balance update
    void onWalletBalanceProgress( int progress, int maxVal );

    // Empty account name mean error
    void onAccountCreated( QString newAccountName);
    void onAccountRenamed(bool success, QString errorMessage);

    // Send results
    void onSend( bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc );

    // I get money
    void onSlateReceivedFrom(QString slate, QString mwc, QString fromAddr, QString message );

    // Files operations
    void onSendFile( bool success, QStringList errors, QString fileName );
    void onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    void onFinalizeFile( bool success, QStringList errors, QString fileName );
    void onSendSlatepack( QString tagId, QString error, QString slatepack );
    void onReceiveSlatepack( QString tagId, QString error, QString slatepack );
    void onFinalizeSlatepack( QString tagId, QString error, QString txUuid );
    void onSubmitFile(bool success, QString message, QString fileName);

    // set receive account name results
    void onSetReceiveAccount( bool ok, QString AccountOrMessage );

    // Transactions
    void onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions);
    void onCancelTransacton( bool success, QString account, int64_t trIdx, QString errMessage );

    void onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages );

    void onAllTransactions( QVector<WalletTransaction> Transactions);

    void onOutputs( QString account, bool showSpent, int64_t height, QVector<WalletOutput> outputs);

    void onCheckResult(bool ok, QString errors );

    // Proof results
    void onExportProof( bool success, QString fn, QString msg );
    void onVerifyProof( bool success, QString fn, QString msg );


    // Listener status listeners...
    void onListenersStatus(bool mqsOnline, bool torOnline);
    // mwc713 get an error  ERROR: new login detected. mwcmqs listener will stop!
    void onListenerMqCollision();

    // Http listener status
    void onHttpListeningStatus(bool listening, QString additionalInfo);

    // Node info
    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

    // getRootPublicKey
    void onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature );

    // Progress update regarding Sync progress. NOTE, THIS SUGNAL doesn't have caller, it can be emitted background.
    void onUpdateSyncProgress(double progressPercent);

    // Response from requestSwapTrades
    void onRequestSwapTrades(QString cookie, QVector<SwapInfo> swapTrades, QString error);

    // Response form deleteSwapTrade. OK - errMsg will be empty
    void onDeleteSwapTrade(QString swapId, QString errMsg);

    // Response from createNewSwapTrade, SwapId on OK,  errMsg on failure
    void onCreateNewSwapTrade(QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg);

    // Response from cancelSwapTrade
    void onCancelSwapTrade(QString swapId, QString error);

    // Response from requestTradeDetails
    void onRequestTradeDetails( SwapTradeInfo swap,
                                QVector<SwapExecutionPlanRecord> executionPlan,
                                QString currentAction,
                                QVector<SwapJournalMessage> tradeJournal,
                                QString error,
                                QString cookie );

    // Response from adjustSwapData
    void onAdjustSwapData(QString swapId, QString call_tag, QString errMsg);

    // Response from performAutoSwapStep
    void onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                           QString lastProcessError,
                           QVector<SwapExecutionPlanRecord> executionPlan,
                           QVector<SwapJournalMessage> tradeJournal,
                           QString error );

    // Notificaiton that nee Swap trade offer was recieved.
    void onNewSwapTrade(QString currency, QString swapId);

    // Notification about new received swap message
    void onNewSwapMessage(QString swapId);
    // Swap marketplace. We have a winner for the offer.
    void onMktGroupWinner(QString swapId, QString tag);

    // Response from backupSwapTradeData
    void onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage);

    // Response from restoreSwapTradeData
    void onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);

    // Response from requestRecieverWalletAddress(url)
    void onRequestRecieverWalletAddress(QString url, QString address, QString error);

    // Response to the post command. Empty message - mean we are good, no errors.
    void onRepost(int txIdx, QString error);

    // Response to decodeSlatepack
    void onDecodeSlatepack( QString tag, QString error, QString slatepack, QString slateJSon, QString content, QString sender, QString recipient );

    // Response from createIntegrityFee
    void onCreateIntegrityFee(QString err, QVector<IntegrityFees> result);

    // Response from requestIntegrityFees
    void onRequestIntegrityFees(QString error, int64_t balance, QVector<wallet::IntegrityFees> fees);

    // Response from withdrawIntegrityFees
    void onWithdrawIntegrityFees(QString error, double mwc, QString account);

    // Response from requestMessagingStatus
    void onRequestMessagingStatus(QString error, MessagingStatus status);

    // Response from messagingPublish
    void onMessagingPublish(QString id, QString uuid, QString error);

    // Response from checkIntegrity
    void onCheckIntegrity(QString error, QVector<QString> expiredMsgUuid);

    // Response from messageWithdraw
    void onMessageWithdraw(QString uuid, QString error);

    // Response from requestReceiveMessages
    void onReceiveMessages(QString error, QVector<ReceivedMessages>);

    // Response from startListenOnTopic
    void onStartListenOnTopic(QString error);

    // Response from stopListenOnTopic
    void onStopListenOnTopic(QString error);

    // Notificaiton that nee Swap trade offer was recieved.
    // messageId: S_MKT_ACCEPT_OFFER or S_MKT_FAIL_BIDDING
    // wallet_tor_address: address from what we get a message
    // offer_id: offer_i from the swap marketplace. It is expected to be known
    void onNewMktMessage(int messageId, QString wallet_tor_address, QString offer_id);

    // Response from sendMarketplaceMessage
    void onSendMarketplaceMessage(QString error, QString response, QString offerId, QString walletAddress, QString cookie);
};

}

Q_DECLARE_METATYPE(wallet::WalletTransaction);
Q_DECLARE_METATYPE(wallet::WalletOutput);
Q_DECLARE_METATYPE(wallet::SwapInfo);
Q_DECLARE_METATYPE(wallet::SwapTradeInfo);
Q_DECLARE_METATYPE(wallet::SwapExecutionPlanRecord);
Q_DECLARE_METATYPE(wallet::SwapJournalMessage);
Q_DECLARE_METATYPE(wallet::IntegrityFees);
Q_DECLARE_METATYPE(wallet::BroadcastingMessage);
Q_DECLARE_METATYPE(wallet::MessagingStatus);
Q_DECLARE_METATYPE(wallet::ReceivedMessages);

#endif // MWCWALLET_H
