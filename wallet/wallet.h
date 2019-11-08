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
#include "../core/mwcexception.h"
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

    int64_t mwcNodeHeight = 0;
    bool mwcServerBroken = true;

    void setData(QString account,
        int64_t total,
        int64_t awaitingConfirmation,
        int64_t lockedByPrevTransaction,
        int64_t currentlySpendable,
        int64_t mwcNodeHeight,
        bool mwcServerBroken);

    QString getLongAccountName() const;
    QString getSpendableAccountName() const;

    bool canDelete() const {return total == 0 && awaitingConfirmation==0 && lockedByPrevTransaction==0 && currentlySpendable==0;}

    // return true is this account can be concidered as deleted
    bool isDeleted() const;

    bool isAwaitingSomething() const {return awaitingConfirmation>0 || lockedByPrevTransaction>0; }
};

struct MwcNodeConnection {
    enum class NODE_CONNECTION_TYPE { CLOUD, LOCAL, CUSTOM }; //

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
};

// Wallet config
struct WalletConfig {
private:
    QString network; // Name of the network as wallet undertand: "Floonet" or "Mainnet"
    QString dataPath;

public:
    QString mwcmqDomainEx; // empty - default value
    QString mwcmqsDomainEx;// empty - default value
    QString keyBasePath;

    WalletConfig() : network("Floonet"), dataPath("Undefined"), keyBasePath("Undefined") {}
    WalletConfig(const WalletConfig & other) = default;
    WalletConfig &operator = (const WalletConfig & other) = default;

    bool operator == (const WalletConfig & other) const { return dataPath==other.dataPath &&
                mwcmqDomainEx==other.mwcmqDomainEx && mwcmqsDomainEx==other.mwcmqsDomainEx &&
                keyBasePath==other.keyBasePath; }

    bool isDefined() const { return  dataPath!="Undefined" && keyBasePath!="Undefined"; }

    WalletConfig & setData(QString network,
                QString dataPath,
                QString mwcmqDomain,
                QString mwcmqsDomain,
                QString keyBasePath );

    void updateDataPath(const QString & path) {dataPath=path;}

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
};

struct WalletOutput {

    QString     outputCommitment;
    QString    MMRIndex;
    QString    blockHeight;
    QString    lockedUntil;
    QString    status;
    bool       coinbase = false;
    QString    numOfConfirms;
    int64_t    valueNano = 0L;
    int64_t    txIdx;

    void setData(QString outputCommitment,
            QString     MMRIndex,
            QString     blockHeight,
            QString     lockedUntil,
            QString     status,
            bool        coinbase,
            QString     numOfConfirms,
            int64_t     valueNano,
            int64_t     txIdx);
};

struct WalletTransaction {
    enum TRANSACTION_TYPE { NONE=0, SEND=1, RECEIVE=2, CANCELLED=0x8000};

    int64_t    txIdx = -1;
    uint    transactionType = TRANSACTION_TYPE::NONE;
    QString txid;
    QString address;
    QString creationTime;
    bool    confirmed = false;
    QString confirmationTime;
    int64_t    coinNano=0; // Net diffrence
    bool    proof=false;

    void setData(int64_t txIdx,
        uint    transactionType,
        QString txid,
        QString address,
        QString creationTime,
        bool    confirmed,
        QString confirmationTime,
        int64_t    coinNano,
        bool    proof);

    bool isValid() const {return txIdx>=0 && transactionType!=TRANSACTION_TYPE::NONE && !txid.isEmpty();}

    bool canBeCancelled() const { return (transactionType & TRANSACTION_TYPE::CANCELLED)==0 && !confirmed; }

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
};

struct WalletUtxoSignature {
    int64_t coinNano; // Output amount
    QString messageHash;
    QString pubKeyCompressed;
    QString messageSignature;

    void setData(int64_t _coinNano, // Output amount
            QString _messageHash,
            QString _pubKeyCompressed,
            QString _messageSignature);
};


// Interface to wallet functionality
// can throw MwcException to signal errors
class Wallet : public QObject
{
    Q_OBJECT
public:
    // network: main | floo
    Wallet();
    virtual ~Wallet();

    // Return true if wallet is running
    virtual bool isRunning() = 0;

    // Check if wallet need to be initialized or not. Will run standalone app, wait for exit and return the result
    // Call might take few seconds
    virtual bool checkWalletInitialized() = 0;

    enum STARTED_MODE { OFFLINE, NORMAL, INIT, RECOVER, GET_NEXTKEY, RECEIVE_SLATE };
    virtual STARTED_MODE getStartedMode() = 0;

    // ---- Wallet Init Phase
    virtual void start(bool loginWithLastKnownPassword)   = 0;
    // Create new wallet and generate a seed for it
    // Check signal: onNewSeed( seed [] )
    virtual void start2init(QString password) = 0;
    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );
    virtual void start2recover(const QVector<QString> & seed, QString password)   = 0;

    // Need for claiming process only
    // Starting the wallet and get the next key
    // wallet713> getnextkey --amount 1000000
    // "Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)"
    // Check Signal: onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);
    virtual void start2getnextkey( int64_t amountNano, QString btcaddress, QString airDropAccPassword ) = 0;

    // Need for claiming process only
    // identifier  - output from start2getnextkey
    // Check Signal: onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    virtual void start2receiveSlate( QString receiveAccount, QString identifier, QString slateFN ) = 0;

    // Check signal: onLoginResult(bool ok)
    virtual void loginWithPassword(QString password)   = 0;

    // Exit from the wallet. Expected that state machine will switch to Init state
    // syncCall - stop NOW. Caller suppose to understand what he is doing
    virtual void logout(bool syncCall) = 0;

    // Confirm that user write the passphase
    // SYNC command
    virtual void confirmNewSeed()  = 0;


    // Current seed for runnign wallet
    // Check Signals: onGetSeed(QVector<QString> seed);
    virtual void getSeed()  = 0;

    //--------------- Listening

    // Checking if wallet is listening through services
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual QPair<bool,bool> getListenerStatus()  = 0;
    // return:  <mwcmq status>, <keybase status>.   true mean was started and need to be stopped, false - never started or was stopped
    virtual QPair<bool,bool> getListenerStartState()  = 0;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startKb, bool initialStart)  = 0;
    // Check Signal: onListeningStartResults

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopKb)  = 0;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getLastKnownMwcBoxAddress()  = 0;

    // Get MWC box <address, index in the chain>
    virtual void getMwcBoxAddress()  = 0;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Change MWC box address to another from the chain. idx - index in the chain.
    virtual void changeMwcBoxAddress(int idx)  = 0;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Generate next box address
    virtual void nextBoxAddress()  = 0;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);


    // -------------- Accounts

    // Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
    // This info needed in many cases and we don't want spend time every time for that.
    virtual QVector<AccountInfo> getWalletBalance(bool filterDeleted = true) const  = 0;

    virtual QString getCurrentAccountName()  = 0;

    // Request Wallet balance update. It is a multistep operation
    virtual void updateWalletBalance()  = 0;
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress
    //          onAccountSwitched - multiple calls, please ignore


    // Create another account, note no delete exist for accounts
    virtual void createAccount( const QString & accountName )  = 0;
    // Check Signal:  onAccountCreated

    // Switch to different account
    virtual void switchAccount(const QString & accountName)  = 0;
    // Check Signal: onAccountSwitched

    // Rename account
    // Check Signal: onAccountRenamed(bool success, QString errorMessage);
    virtual void renameAccount(const QString & oldName, const QString & newName)  = 0;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onCheckResult(bool ok, QString errors );
    virtual void check(bool wait4listeners)  = 0;

    // Get current configuration of the wallet.
    virtual WalletConfig getWalletConfig()  = 0;

    // Get configuration form the resource file.
    virtual WalletConfig getDefaultConfig()  = 0;

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

    // -------------- Transactions

    // Set account that will receive the funds
    // Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
    virtual void setReceiveAccount(QString account)  = 0;

    // Get wallet balance
    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(int64_t transactionID)  = 0;

    // Proof results

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Check Signal: onExportProof( bool success, QString fn, QString msg );
    virtual void generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName )  = 0;

    // Verify the proof for transaction
    // Check Signal: onVerifyProof( bool success, QString msg );
    virtual void verifyMwcBoxTransactionProof( QString proofFileName )  = 0;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( const wallet::AccountInfo &account, int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs )  = 0;
    // Receive transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx)  = 0;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse )  = 0;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    // coinNano == -1  - mean All
    virtual void sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs )  = 0;
    // Check signal:  onSend

    // Get total number of Outputs
    // Check Signal: onOutputCount(int number)
    virtual void getOutputCount(QString account)  = 0;

    // Show outputs for the wallet
    // Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> outputs)
    virtual void getOutputs(QString account, int offset, int number)  = 0;

    // Get total number of Transactions
    // Check Signal: onTransactionCount(int number)
    virtual void getTransactionCount(QString account)  = 0;

    // Show all transactions for current account
    // Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
    virtual void getTransactions(QString account, int offset, int number)  = 0;

    // Read all transactions for all accounts. Might tale time...
    // Check Signal: onAllTransactions( QVector<WalletTransaction> Transactions)
    virtual void getAllTransactions()  = 0;


    // ----------- HODL
    // https://github.com/mimblewimble/grin/pull/2374

    // sign output commitment utxo. Hash must be provivded by party that want to verify the signature.
    // Late this signature can be used for verification of ounewship
    //virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) = 0;

private:
signals:

    // Config was updated
    void onConfigUpdate();

    // Get next key result
    void onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPassword);

    // Result of the login
    void onLoginResult(bool ok);

    // Get MWC updated address. Normally you don't need that
    void onMwcAddress(QString mwcAddress);

    // Get MWC MQ address with index
    void onMwcAddressWithIndex(QString mwcAddress, int idx);

    // New seed generated by the wallet
    void onNewSeed(QVector<QString> seed);

    // Seed from current account
    // Get empty or size 1 in case of error
    void onGetSeed(QVector<QString> seed);

    void onRecoverProgress( int progress, int maxVal );
    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                                   QStringList errorMessages, bool initialStart ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                                QStringList errorMessages );

    // Account info is updated
    void onWalletBalanceUpdated();
    // Progress for balance update
    void onWalletBalanceProgress( int progress, int maxVal );

    // Empty accoount name mean error
    void onAccountSwitched(QString currentAccountName);
    void onAccountCreated( QString newAccountName);
    void onAccountRenamed(bool success, QString errorMessage);

    // Send results
    void onSend( bool success, QStringList errors, QString address, int64_t txid, QString slate );

    void onSlateSendTo( QString slate, QString mwc, QString sendAddr );
    void onSlateFinalized( QString slate );
    // I send and I get a responce from the 2nd party
    void onSlateReceivedBack(QString slate, QString mwc, QString fromAddr);
    // I get money
    void onSlateReceivedFrom(QString slate, QString mwc, QString fromAddr, QString message );

    // Files operations
    void onSendFile( bool success, QStringList errors, QString fileName );
    void onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    void onFinalizeFile( bool success, QStringList errors, QString fileName );

    // set receive account name results
    void onSetReceiveAccount( bool ok, QString AccountOrMessage );

    // Transactions
    void onTransactionCount(QString account, int number);
    void onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions);
    void onCancelTransacton( bool success, int64_t trIdx, QString errMessage );

    void onAllTransactions( QVector<WalletTransaction> Transactions);

    void onOutputCount(QString account, int number);
    void onOutputs( QString account, int64_t height, QVector<WalletOutput> Transactions);

    void onCheckResult(bool ok, QString errors );

    // Proof results
    void onExportProof( bool success, QString fn, QString msg );
    void onVerifyProof( bool success, QString fn, QString msg );


    // Listener status listeners...
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);
    // mwc713 get an error  ERROR: new login detected. mwcmqs listener will stop!
    void onListenerMqCollision();

    // Node info
    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

};

}

Q_DECLARE_METATYPE(wallet::WalletTransaction);
Q_DECLARE_METATYPE(wallet::WalletOutput);

#endif // MWCWALLET_H
