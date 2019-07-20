#ifndef MWCWALLET_H
#define MWCWALLET_H

#include <QVector>
#include <QString>
#include "../core/mwcexception.h"
#include "../util/ioutils.h"
#include "../util/stringutils.h"
#include <QDateTime>
#include <QObject>

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

    bool canDelete() const {return total == 0 && awaitingConfirmation==0 && lockedByPrevTransaction==0 && currentlySpendable==0;}

    // return true is this account can be concidered as deleted
    bool isDeleted() const;
};

// Wallet config
struct WalletConfig {
    QString dataPath;
    QString mwcmqDomain;
    QString keyBasePath;

    WalletConfig() : dataPath("Undefined"), mwcmqDomain("Undefined"), keyBasePath("Undefined") {}
    WalletConfig(const WalletConfig & other) = default;
    WalletConfig &operator = (const WalletConfig & other) = default;

    bool operator == (const WalletConfig & other) const { return dataPath==other.dataPath && mwcmqDomain==other.mwcmqDomain && keyBasePath==other.keyBasePath; }

    bool isDefined() const { return  dataPath!="Undefined" && mwcmqDomain!="Undefined" && keyBasePath!="Undefined"; }

    WalletConfig & setData(QString dataPath,
                QString mwcmqDomain,
                QString keyBasePath);
};

struct NodeStatus {
    int     connection; // current number of connections
    QString tip; // Status og the current fork tip
    int64_t    height; // Height of the tip
    int64_t    total_difficulty; // Total difficulty accumulated on that fork since genesis block

    void setData(int   connection,
        QString tip,
        int64_t height,
        int64_t total_difficulty);
};

struct WalletContact {
    QString name;
    QString address;

    void setData(QString name,
        QString address);

    void saveData( QDataStream & out) const;
    bool loadData( QDataStream & in);
};

struct WalletOutput {

    QString     outputCommitment;
    QString    MMRIndex = 0;
    QString    blockHeight = 0;
    QString    lockedUntil = 0;
    QString    status;
    bool       coinbase = 0;
    QString    numOfConfirms = 0;
    int64_t    valueNano = 0;
    QString    txIdx = 0;

    void setData(QString outputCommitment,
            QString     MMRIndex,
            QString     blockHeight,
            QString     lockedUntil,
            QString     status,
            bool        coinbase,
            QString     numOfConfirms,
            int64_t     valueNano,
            QString     txIdx);
};

struct WalletTransaction {
    enum TRANSACTION_TYPE { NONE=0, SEND=1, RECIEVE=2, CANCELLED=0x8000};

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
        if ( transactionType & TRANSACTION_TYPE::RECIEVE )
            res += "Recieve";

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


struct WalletNotificationMessages {
    enum LEVEL {ERROR, WARNING, INFO, DEBUG};
    LEVEL level=DEBUG;
    QString message;
    QDateTime time;

    WalletNotificationMessages() {time=QDateTime::currentDateTime();}
    WalletNotificationMessages(LEVEL _level, QString _message) : level(_level), message(_message) {time=QDateTime::currentDateTime();}
    WalletNotificationMessages(const WalletNotificationMessages&) = default;
    WalletNotificationMessages &operator=(const WalletNotificationMessages&) = default;

    // To debug string
    QString toString() const;
};

enum InitWalletStatus {NONE, NEED_PASSWORD, NEED_INIT, WRONG_PASSWORD, READY};
QString toString(InitWalletStatus status);

// Interface to wallet functionality
// can throw MwcException to signal errors
class Wallet : public QObject
{
    Q_OBJECT
public:
    // network: main | floo
    Wallet();
    virtual ~Wallet();

    // Generic. Reporting fatal error that somebody will process and exit app
    virtual void reportFatalError( QString message )  = 0;


    // Get all notification messages
    virtual const QVector<WalletNotificationMessages> & getWalletNotificationMessages()  = 0;
    // Check signal: onNewNotificationMessage

    // ---- Wallet Init Phase
    virtual void start()   = 0;
    virtual void loginWithPassword(QString password)   = 0;

    // Exit from the wallet. Expected that state machine will switch to Init state
    virtual void logout()   = 0;

    // Check signal: onInitWalletStatus
    virtual InitWalletStatus getWalletStatus()  = 0;

    // Close the wallet and release the process
    virtual bool close()  = 0;

    // Create the wallet, generate the seed. Return the words to recover the wallet
    virtual void generateSeedForNewAccount(QString password)  = 0;
    // Check signal: onNewSeed( seed [] )


    // Confirm that user write the passphase
    virtual void confirmNewSeed()  = 0;


    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    virtual void recover(const QVector<QString> & seed, QString password)   = 0;
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );

    // Current seed for runnign wallet
    // Check Signals: onGetSeed(QVector<QString> seed);
    virtual void getSeed()  = 0;

    //--------------- Listening

    // Checking if wallet is listening through services
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual QPair<bool,bool> getListeningStatus()  = 0;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startKb)  = 0;
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

    // Update wallet config. Will update config and restart the wmc713.
    // Note!!! Caller is fully responsible for input validation. Normally mwc713 will sart, but some problems might exist
    //          and caller suppose listen for them
    // If returns true, expected that wallet will need to have password input.
    virtual bool setWalletConfig(const WalletConfig & config)  = 0;

    // Status of the node
    virtual NodeStatus getNodeStatus()  = 0;

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
    virtual void sendFile( int64_t coinNano, QString fileTx )  = 0;
    // Recieve transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx)  = 0;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse )  = 0;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    // coinNano == -1  - mean All
    virtual void sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address, QString message="", int inputConfirmationNumber=10, int changeOutputs=1 )  = 0;
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



    // -------------- Contacts

    // Get the contacts
    virtual QVector<WalletContact> getContacts()  = 0;
    // Add new contact
    virtual QPair<bool, QString> addContact( const WalletContact & contact )  = 0;
    // Remove contact. return false if not found
    virtual QPair<bool, QString> deleteContact( const QString & name )  = 0;

    // ----------- HODL
    // https://github.com/mimblewimble/grin/pull/2374

    // sign output commitment utxo. Hash must be provivded by party that want to verify the signature.
    // Late this signature can be used for verification of ounewship
    virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) = 0;

private:
signals:
    // Notification/error message
    void onNewNotificationMessage(WalletNotificationMessages::LEVEL level, QString message);

    // Update of the wallet status
    void onInitWalletStatus(InitWalletStatus status);

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
                                   QStringList errorMessages ); // error messages, if get some

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
    void onSend( bool success, QStringList errors );

    void onSlateSend( QString slate, QString mwc, QString sendAddr );
    void onSlateRecieved( QString slate, QString mwc, QString fromAddr );
    void onSlateFinalized( QString slate );

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

    void onOutputCount(QString account, int number);
    void onOutputs( QString account, int64_t height, QVector<WalletOutput> Transactions);

    void onCheckResult(bool ok, QString errors );

    // Proof results
    void onExportProof( bool success, QString fn, QString msg );
    void onVerifyProof( bool success, QString fn, QString msg );


    // Listener status listeners...
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);
};

}

Q_DECLARE_METATYPE(wallet::WalletNotificationMessages::LEVEL);
Q_DECLARE_METATYPE(wallet::InitWalletStatus);
Q_DECLARE_METATYPE(wallet::WalletTransaction);
Q_DECLARE_METATYPE(wallet::WalletOutput);

#endif // MWCWALLET_H
