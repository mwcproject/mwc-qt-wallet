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
    QString keyBasePath;

    // Http listening params...
    bool foreignApi = false; // Is foreign API is enabled
    QString foreignApiAddress; // Example: 0.0.0.0:3416
    QString foreignApiSecret;  // Secret value
    // For https configuration.
    QString tlsCertificateFile;
    QString tlsCertificateKey;

    WalletConfig() : network("Mainnet"), dataPath("Undefined"), keyBasePath("Undefined") {}
    WalletConfig(const WalletConfig & other) = default;
    WalletConfig &operator = (const WalletConfig & other) = default;

    bool operator == (const WalletConfig & other) const;

    bool isDefined() const { return  dataPath!="Undefined" && keyBasePath!="Undefined"; }

    WalletConfig & setData(QString network,
                QString dataPath,
                QString mwcmqsDomain,
                QString keyBasePath,
                bool foreignApi,
                QString foreignApiAddress,
                QString foreignApiSecret,
                QString tlsCertificateFile,
                QString tlsCertificateKey);

    WalletConfig & setForeignApi(bool foreignApi,
                       QString foreignApiAddress, QString foreignApiSecret,
                       QString tlsCertificateFile, QString tlsCertificateKey);

    WalletConfig & setDataWalletCfg(QString network,
                           QString dataPath,
                           QString mwcmqsDomain,
                           QString keyBasePath);

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

    // caller is responsible to call saveNetwork2DataPath if needed
    void setDataPathWithNetwork( QString _dataPath, QString _network ) { dataPath=_dataPath; network = _network;  }

    // First Nerwork, Second is Arch
    static QPair<QString,QString> readNetworkArchFromDataPath(QString configPath); // local path as writen in config
    static void    saveNetwork2DataPath(QString configPath, QString network, QString arch); // Save the network into the data path

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

    static QString getCSVHeaders() {
        return csvHeaders;
    }

    // return transactions values formatted into a CSV string
    QString toStringCSV();

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

// Some startus booleans for 3 listeners
struct ListenerStatus {
    bool mqs = false;
    bool keybase = false;
    bool tor = false;

    ListenerStatus() = default;
    ListenerStatus(bool _mqs, bool _keybase, bool _tor) : mqs(_mqs), keybase(_keybase),  tor(_tor) {}

    ListenerStatus(const ListenerStatus & item) = default;
    ListenerStatus & operator = (const ListenerStatus & item) = default;
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
    virtual bool checkWalletInitialized() = 0;

    enum STARTED_MODE { OFFLINE, NORMAL, INIT, RECOVER, GET_NEXTKEY, RECEIVE_SLATE };
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
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual ListenerStatus getListenerStatus()  = 0;
    // return:  <mwcmq status>, <keybase status>.   true mean was started and need to be stopped, false - never started or was stopped
    virtual ListenerStatus getListenerStartState()  = 0;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startKb, bool startTor, bool initialStart)  = 0;
    // Check Signal: onListeningStartResults

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopKb, bool stopTor)  = 0;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getMqsAddress()  = 0;
    // Get latest Tor address that we see
    virtual QString getTorAddress()  = 0;

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

    // Request sync (update_wallet_state) for the
    virtual void sync(bool showSyncProgress, bool enforce) = 0;


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
    virtual bool setWalletConfig(const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode  )  = 0;

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
            int inputConfirmationNumber, int changeOutputs, const QStringList & outputs )  = 0;

    // Receive transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx, QString identifier = "")  = 0;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse, bool fluff )  = 0;

    // submit finalized transaction. Make sense for cold storage => online node operation
    // Check Signal: onSubmitFile(bool ok, String message)
    virtual void submitFile( QString fileTx )  = 0;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    // coinNano == -1  - mean All
    // Check signal:  onSend
    virtual void sendTo( const QString &account, int64_t coinNano, const QString & address, const QString & apiSecret,
                         QString message, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, bool fluff )  = 0;

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
    virtual void getTransactionById(QString account, int64_t txIdx ) = 0;

    // Get wallet balance
    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(QString account, int64_t txIdx)  = 0;

    // Read all transactions for all accounts. Might take time...
    // Check Signal: onAllTransactions( QVector<WalletTransaction> Transactions)
    virtual void getAllTransactions() = 0;


    // ----------- HODL
    // https://github.com/mimblewimble/grin/pull/2374

    // sign output commitment utxo. Hash must be provivded by party that want to verify the signature.
    // Late this signature can be used for verification of ounewship
    //virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) = 0;

    // Get root public key with signed message. Message is optional, can be empty
    // Check Signal: onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature );
    virtual void getRootPublicKey( QString message2sign ) = 0;

private:
signals:

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

   // Get tor updated address.
   void onTorAddress(QString torAddress);

    // New seed generated by the wallet
    void onNewSeed(QVector<QString> seed);

    // Seed from current account
    // Get empty or size 1 in case of error
    void onGetSeed(QVector<QString> seed);

    void onRecoverProgress( int progress, int maxVal );
    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool kbTry, bool tor, // what we try to start
                                   QStringList errorMessages, bool initialStart ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool kbTry, bool tor, // what we try to stop
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
    void onListenersStatus(bool mqsOnline, bool keybaseOnline, bool torOnline);
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
};

}

Q_DECLARE_METATYPE(wallet::WalletTransaction);
Q_DECLARE_METATYPE(wallet::WalletOutput);

#endif // MWCWALLET_H
