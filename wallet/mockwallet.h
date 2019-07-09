#ifndef MOCKWALLET_H
#define MOCKWALLET_H

#include "wallet.h"

namespace wallet {

// Wallet simulator. Used for testign and prototyping
class MockWallet : public Wallet
{
public:
    // network: main | floo
    MockWallet();
    virtual ~MockWallet() override;

    // Generic. Reporting fatal error that somebody will process and exit app
    virtual void reportFatalError( QString message ) noexcept(false) override;

    // Get all notification messages
    virtual const QVector<WalletNotificationMessages> & getWalletNotificationMessages() noexcept(false) override;


    virtual void start() noexcept(false) override;
    virtual void loginWithPassword(QString password) noexcept(false) override;

    virtual InitWalletStatus getWalletStatus() noexcept(false) override {return initStatus;}

    virtual bool close() noexcept(false) override;

    virtual void generateSeedForNewAccount(QString password) noexcept(false) override;
    // Check signal: onNewSeed( seed [] )

    // Confirm that user write the passphase
    virtual void confirmNewSeed() noexcept(false) override;

    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    virtual void recover(const QVector<QString> & seed, QString password) noexcept(false) override;
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );

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
    virtual QString getCurrentAccountName() noexcept(false) override {return "default";}

    // Request Wallet balance update. It is a multistep operation
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress
    virtual void updateWalletBalance() noexcept(false) override {emit onWalletBalanceUpdated();}

    //  Get list of open account
    // Create another account, note no delete exist for accounts
    virtual void createAccount( const QString & accountName ) noexcept(false) override;

    // Switch to different account
    virtual void switchAccount(const QString & accountName) noexcept(false) override;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onCheckResult(bool ok, QString errors );
    void check(bool wait4listeners) noexcept(false) override;

    virtual WalletConfig getWalletConfig() noexcept(false) override;
    virtual bool setWalletConfig(const WalletConfig & config) noexcept(false) override;

    // Status of the node
    virtual NodeStatus getNodeStatus() noexcept(false) override;

    // -------------- Transactions
    virtual QVector<AccountInfo> getWalletBalance(bool filterDeleted) const noexcept(false) override;

    // Cancel transaction
    // Check Signal:  onCancelTransacton
    virtual void cancelTransacton(int64_t transactionID) noexcept(false) override;

    virtual void generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName ) noexcept(false) override;
    virtual void verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) override;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( int64_t coinNano, QString fileTx ) noexcept(false) override;
    // Recieve transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx) noexcept(false) override;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse ) noexcept(false) override;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    virtual void sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address, QString message="",
                int inputConfirmationNumber=10, int changeOutputs=1 ) noexcept(false) override;
    // Check signal:  onSend

    // Show outputs for the wallet
    // Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> Transactions)
    virtual void getOutputs(QString account) noexcept(false) override;

    // Show all transactions for current account
    // Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
    virtual void getTransactions(QString account) noexcept(false) override;

    // -------------- Contacts

    // Get the contacts
    virtual QVector<WalletContact> getContacts() noexcept(false) override;
    virtual QPair<bool, QString> addContact( const WalletContact & contact ) noexcept(false) override;
    virtual QPair<bool, QString> deleteContact( const QString & name ) noexcept(false) override;

    // ----------- HODL
    virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) override;

private:
    // data
    QString dataPath;

    bool isInit = false;// true is init
    QString walletPassword; // password that encrypt the seed

    QVector<QString> accounts;
    int selectedAccount = 0;
    WalletConfig config;

    QVector<WalletContact> contacts;

    QVector<WalletTransaction> transactions;

    int currentAddressIdx = 0;

    // Runtime data
    bool listenMwcBox = false;
    bool listenKeybase = false;
    bool listenFogeignApi = false;
    InitWalletStatus initStatus = InitWalletStatus::NONE;

    QVector<WalletNotificationMessages> notificationMessages;
private:
    bool loadData();
    void saveData() const;
};

}

#endif // MOCKWALLET_H
