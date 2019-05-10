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

    // return true is it s a first run for the wallet
    bool isVeryFirstRun() noexcept(false) override;

    Wallet::InitWalletStatus open(QString network, const QString & password) noexcept(false) override;
    bool close() noexcept(false) override;
    QVector<QString> init() noexcept(false) override;
    // Confirm that user write the passphase
    void confirmNewSeed() noexcept(false) override;

    QPair<bool, QString> recover(const QVector<QString> & seed) noexcept(false) override;

    //--------------- Listening
    uint getListeningStatus() noexcept(false) override;
    QPair<bool, QString> startListening(ListenState lstnState) noexcept(false) override;
    bool stopListening(ListenState lstnState) noexcept(false) override;
    QPair<QString,int> getMwcBoxAddress() noexcept(false) override;
    void changeMwcBoxAddress(int idx) noexcept(false) override;

    // --------------- Foreign API
    bool isForegnApiRunning() noexcept(false) override;
    bool startForegnAPI(int port, QString foregnApiSecret) noexcept(false) override;

    // -------------- Accounts

    //  Get list of open account
    QVector<QString> getAccountList() noexcept(false) override;

    // Create another account, note no delete exist for accounts
    QPair<bool, QString> createAccount( const QString & accountName ) noexcept(false) override;

    // Switch to different account
    QPair<bool, QString> switchAccount(const QString & accountName) noexcept(false) override;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    void check() noexcept(false) override;

    WalletConfig getWalletConfig() noexcept(false) override;
    bool setWalletConfig(const WalletConfig & config) noexcept(false) override;

    // Status of the node
    NodeStatus getNodeStatus() noexcept(false) override;

    // -------------- Transactions
    WalletInfo getWalletBalance() noexcept(false) override;
    bool cancelTransacton(QString transactionID) noexcept(false) override;

    WalletProofInfo  generateMwcBoxTransactionProof( int transactionId, QString resultingFileName ) noexcept(false) override;
    WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) override;

    QPair<bool, QString> sendFile( long coinNano, QString fileTx ) noexcept(false) override;
    QPair<bool, QString> receiveFile( QString fileTx, QString responseFileName ) noexcept(false) override;
    QPair<bool, QString> finalizeFile( QString fileTxResponse ) noexcept(false) override;

    QPair<bool, QString> sendTo( long coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs ) noexcept(false) override;

    QVector<WalletOutput> getOutputs() noexcept(false) override;
    // numOfTransactions - transaction limit to return. <=0 - get all transactions
    QVector<WalletTransaction> getTransactions(int numOfTransactions=-1) noexcept(false) override;

    // -------------- Contacts

    // Get the contacts
    QVector<WalletContact> getContacts() noexcept(false) override;
    void addContact( const WalletContact & contact ) noexcept(false) override;
    bool deleteContact( const QString & name ) noexcept(false) override;

    // ----------- HODL
    WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) override;

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

    bool listenMwcBox = false;
    bool listenKeystone = false;
    bool listenFogeignApi = false;

private:
    bool loadData();
    void saveData() const;
};

}

#endif // MOCKWALLET_H
