#ifndef MWC713_H
#define MWC713_H

#include "wallet.h"
#include <QObject>
#include <QProcess>
#include "../core/global.h"

namespace tries {
    class Mwc713InputParser;
}

namespace wallet {

class Mwc713State;
class Mwc713EventManager;

class MWC713 : public Wallet
{
    Q_OBJECT
public:

public:
    MWC713(QString mwc713path, QString mwc713configPath);
    virtual ~MWC713() override;

    // Generic. Reporting fatal error that somebody will process and exit app
    virtual void reportFatalError( QString message ) noexcept(false) override;

    // Get all notification messages
    virtual const QVector<WalletNotificationMessages> & getWalletNotificationMessages() noexcept(false) override {return notificationMessages;}
    // Check signal: onNewNotificationMessage

    // ---- Wallet Init Phase
    virtual void start() noexcept(false) override;
    virtual void loginWithPassword(QString password) noexcept(false) override;
    // Check signal: onInitWalletStatus
    virtual InitWalletStatus getWalletStatus() noexcept(false) override {return initStatus;}


    virtual bool close() noexcept(false) override {return true;}
    virtual void generateSeedForNewAccount(QString password) noexcept(false) override;
    // Check signal: onNewSeed( seed [] )

    // Confirm that user write the passphase
    virtual void confirmNewSeed() noexcept(false) override;

    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    // NOTE: Expected that listen is stopped for both - mwc MQ & keybase
    // Recover with percentage
    // NOTE: Expected that listening will be started
    virtual void recover(const QVector<QString> & seed, QString password) noexcept(false) override;
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );

    // Current seed for runnign wallet
    // Check Signals: onGetSeed(QVector<QString> seed);
    virtual void getSeed() noexcept(false) override;

    //--------------- Listening

    // Checking if wallet is listening through services
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual QPair<bool,bool> getListeningStatus() noexcept(false) override;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startKb) noexcept(false) override;
    // Check Signal: onStartListening

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopKb) noexcept(false) override;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getLastKnownMwcBoxAddress() noexcept(false) override;

    // Get MWC box <address, index in the chain>
    virtual void getMwcBoxAddress() noexcept(false) override;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Change MWC box address to another from the chain. idx - index in the chain.
    virtual void changeMwcBoxAddress(int idx) noexcept(false) override;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Generate next box address
    virtual void nextBoxAddress() noexcept(false) override;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);


    // -------------- Accounts

    // Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
    // This info needed in many cases and we don't want spend time every time for that.
    virtual QVector<AccountInfo>  getWalletBalance(bool filterDeleted = true) const noexcept(false) override;

    virtual QString getCurrentAccountName() noexcept(false) override {return currentAccount;}


    // Request Wallet balance update. It is a multistep operation
    virtual void updateWalletBalance() noexcept(false) override;
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress


    // Create another account, note no delete exist for accounts
    virtual void createAccount( const QString & accountName ) noexcept(false) override;
    // Check Signal:  onAccountCreated

    // Switch to different account
    virtual void switchAccount(const QString & accountName) noexcept(false) override;
    // Check Signal: onAccountSwitched

    // Rename account
    // Check Signal: onAccountRenamed(bool success, QString errorMessage);
    virtual void renameAccount(const QString & oldName, const QString & newName) noexcept(false) override;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    void check() noexcept(false) override {}

    virtual WalletConfig getWalletConfig() noexcept(false) override {return WalletConfig();}
    virtual QPair<bool, QString> setWalletConfig(const WalletConfig & config) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    // Status of the node
    virtual NodeStatus getNodeStatus() noexcept(false) override {return NodeStatus();}

    // -------------- Transactions

    // Set account that will receive the funds
    // Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
    virtual void setReceiveAccount(QString account) noexcept(false) override;

    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(long transactionID) noexcept(false) override;

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    // Check Signal: onExportProof( bool success, QString fn, QString msg );
    virtual void generateMwcBoxTransactionProof( long transactionId, QString resultingFileName ) noexcept(false) override;

    // Verify the proof for transaction
    // Check Signal: onVerifyProof( bool success, QString msg );
    virtual void verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) override;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( long coinNano, QString fileTx ) noexcept(false) override;
    // Recieve transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx) noexcept(false) override;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse ) noexcept(false) override;


    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    // Check signal:  onSend
    // coinNano == -1  - mean All
    virtual void sendTo( const wallet::AccountInfo &account, long coinNano, const QString & address, QString message="", int inputConfirmationNumber=10, int changeOutputs=1 ) noexcept(false) override;

    virtual QVector<WalletOutput> getOutputs() noexcept(false) override {return QVector<WalletOutput>();}

    // Show all transactions for current account
    // Check Signal: onTransactions( QString account, long height, QVector<WalletTransaction> Transactions)
    virtual void getTransactions() noexcept(false) override;

    // -------------- Contacts

    // Get the contacts
    virtual QVector<WalletContact> getContacts() noexcept(false) override {return QVector<WalletContact>();}
    virtual QPair<bool, QString> addContact( const WalletContact & contact ) noexcept(false) override  {return QPair<bool, QString>(true,"");}
    virtual QPair<bool, QString> deleteContact( const QString & name ) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    // ----------- HODL
    virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) override {return WalletUtxoSignature();}

public:

    tries::Mwc713InputParser * getInputParser() const { return inputParser;}

    // Feed the command to mwc713 process
    void executeMwc713command( QString cmd, QString shadowStr);
public:
    // Task Reporting methods
    enum MESSAGE_LEVEL { FATAL_ERROR, CRITICAL, WARNING, INFO, DEBUG };
    enum MESSAGE_ID {INIT_ERROR, GENERIC, MWC7113_ERROR, TASK_TIMEOUT };
    void appendNotificationMessage( MESSAGE_LEVEL level, MESSAGE_ID id, QString message );

    // Wallet init status
    enum INIT_STATUS {NONE, NEED_PASSWORD, NEED_SEED, WRONG_PASSWORD, READY };
    void setInitStatus( INIT_STATUS  initStatus );

    void setMwcAddress( QString mwcAddress ); // Set active MWC address. Listener might be offline
    void setMwcAddressWithIndex( QString mwcAddress, int idx );

    void setNewSeed( QVector<QString> seed );

    void setGettedSeed( QVector<QString> seed );

    void setListeningStartResults( bool mqTry, bool kbTry, // what we try to start
            QStringList errorMessages );

    void setListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                                QStringList errorMessages );

    void setMwcMqListeningStatus(bool online);
    void setKeybaseListeningStatus(bool online);


    void setRecoveryResults( bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages );
    void setRecoveryProgress( long progress, long limit );

    // Update account feedback
    void updateAccountList( QVector<QString> accounts );
    void updateAccountProgress(int accountIdx, int totalAccounts);
    void updateAccountFinalize(QString prevCurrentAccount);
    void createNewAccount( QString newAccountName );
    void switchToAccount( QString switchAccountName );

    void updateRenameAccount(const QString & oldName, const QString & newName, bool createSimulation,
                             bool success, QString errorMessage);

    void infoResults( QString currentAccountName, long height,
           long totalNano, long waitingConfNano, long lockedNano, long spendableNano,
                      bool mwcServerBroken );

    void setSendResults(bool success, QStringList errors);

    void reportSlateSend( QString slate, QString mwc, QString sendAddr );
    void reportSlateRecieved( QString slate, QString mwc, QString fromAddr );
    void reportSlateFinalized( QString slate );

    void setSendFileResult( bool success, QStringList errors, QString fileName );
    void setReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    void setFinalizeFile( bool success, QStringList errors, QString fileName );

    // Transactions
    void setTransactions( QString account, long height, QVector<WalletTransaction> Transactions);

    void setExportProofResults( bool success, QString fn, QString msg );
    void setVerifyProofResults( bool success, QString fn, QString msg );

    void setTransCancelResult( bool success, long transId, QString errMsg );

    void setSetReceiveAccount( bool ok, QString accountOrMessage );
private:
    void mwc713connect();
    void mwc713disconnect();

    // Update acc value at collection accounts. If account is not founf, we can add it (addIfNotFound) or skip
    void updateAccountInfo( const AccountInfo & acc, QVector<AccountInfo> & accounts, bool addIfNotFound ) const;

private slots:
    // mwc713 Process IOs
    void	mwc713errorOccurred(QProcess::ProcessError error);
    void	mwc713finished(int exitCode, QProcess::ExitStatus exitStatus);
    void	mwc713readyReadStandardError();
    void	mwc713readyReadStandardOutput();

private:
    QString mwc713Path; // path to the backed binary
    QString mwc713configPath; // config file for mwc713
    QProcess * mwc713process = nullptr;
    tries::Mwc713InputParser * inputParser = nullptr; // Parser will generate bunch of signals that wallet will listem on

    Mwc713EventManager * eventCollector = nullptr;

    // Stages (flags) of the wallet
    InitWalletStatus initStatus = InitWalletStatus::NONE;

    const int MESSAGE_SIZE_LIMIT = 10000;
    QVector<WalletNotificationMessages> notificationMessages;

    QString mwcAddress; // Address from mwc listener

    // listening statuses
    bool mwcMqOnline = false;
    bool keybaseOnline = false;

    // Connections to mwc713process
    QVector< QMetaObject::Connection > mwc713connections; // open connection to mwc713

    // Accounts with balances info
    QVector<AccountInfo> accountInfo;
    QString currentAccount = "default"; // Keep current account by name. It fit better to mwc713 interactions.

private:
    // Temprary values, local values for states
    QString walletPassword;

    QVector<AccountInfo> collectedAccountInfo;
};

}

#endif // MWC713_H
