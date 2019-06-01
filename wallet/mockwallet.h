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


    virtual void start(QString network) noexcept(false) override;
    virtual void loginWithPassword(QString password, QString account) noexcept(false) override;

    virtual InitWalletStatus getWalletStatus() noexcept(false) override {return initStatus;}

    virtual bool close() noexcept(false) override;
    virtual QVector<QString> generateSeedForNewAccount() noexcept(false) override;
    // Confirm that user write the passphase
    virtual void confirmNewSeed() noexcept(false) override;

    virtual QPair<bool, QString> recover(const QVector<QString> & seed) noexcept(false) override;

    //--------------- Listening
    virtual uint getListeningStatus() noexcept(false) override;
    virtual QPair<bool, QString> startListening(ListenState lstnState) noexcept(false) override;
    virtual QPair<bool, QString> stopListening(ListenState lstnState) noexcept(false) override;
    virtual QPair<QString,int> getMwcBoxAddress() noexcept(false) override;
    virtual void changeMwcBoxAddress(int idx) noexcept(false) override;
    virtual void nextBoxAddress() noexcept(false) override;


    // --------------- Foreign API
    virtual bool isForegnApiRunning() noexcept(false) override;
    virtual QPair<bool,QString> startForegnAPI(int port, QString foregnApiSecret) noexcept(false) override;
    virtual QPair<bool,QString> stopForeignAPI() noexcept(false) override;

    // -------------- Accounts

    //  Get list of open account
    virtual QVector<QString> getAccountList() noexcept(false) override;

    // Create another account, note no delete exist for accounts
    virtual QPair<bool, QString> createAccount( const QString & accountName ) noexcept(false) override;

    // Switch to different account
    virtual QPair<bool, QString> switchAccount(const QString & accountName) noexcept(false) override;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    void check() noexcept(false) override;

    virtual WalletConfig getWalletConfig() noexcept(false) override;
    virtual QPair<bool, QString> setWalletConfig(const WalletConfig & config) noexcept(false) override;

    // Status of the node
    virtual NodeStatus getNodeStatus() noexcept(false) override;

    // -------------- Transactions
    virtual WalletInfo getWalletBalance() noexcept(false) override;
    virtual bool cancelTransacton(QString transactionID) noexcept(false) override;

    virtual WalletProofInfo  generateMwcBoxTransactionProof( long transactionId, QString resultingFileName ) noexcept(false) override;
    virtual WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) override;

    virtual QPair<bool, QString> sendFile( long coinNano, QString fileTx ) noexcept(false) override;
    virtual QPair<bool, QString> receiveFile( QString fileTx, QString responseFileName ) noexcept(false) override;
    virtual QPair<bool, QString> finalizeFile( QString fileTxResponse ) noexcept(false) override;

    virtual QPair<bool, QString> sendTo( long coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs ) noexcept(false) override;

    virtual QVector<WalletOutput> getOutputs() noexcept(false) override;
    // numOfTransactions - transaction limit to return. <=0 - get all transactions
    virtual QVector<WalletTransaction> getTransactions(int numOfTransactions=-1) noexcept(false) override;

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

    QString blockchainNetwork;

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
    bool listenKeystone = false;
    bool listenFogeignApi = false;
    InitWalletStatus initStatus = InitWalletStatus::NONE;

    QVector<WalletNotificationMessages> notificationMessages;
private:
    bool loadData();
    void saveData() const;
};

}

#endif // MOCKWALLET_H
