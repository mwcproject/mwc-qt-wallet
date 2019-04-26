#ifndef MWCWALLET_H
#define MWCWALLET_H

#include <QVector>
#include <QString>
#include "../util/mwcexception.h"

struct WalletInfo {
    QString accountName;
    // in nano coins
    long total;
    long awaitingConfirmation;
    long lockedByPrevTransaction;
    long currentlySpendable;
};

struct WalletConfig {
    QString dataPath;
    QString mwcboxDomain;
    int mwcboxPort;
    QString mwcNodeURI;
    QString mwcNodeSecret;

};

struct WalletContact {
    QString name;
    QString address;
};

struct WalletOutput {
    enum STATUS {Unconfirmed, Confirmed};

    QString outputCommitment;
    long    MMRIndex;
    long    mlockHeight;
    bool    lockedUntil;
    STATUS  status;
    bool    coinbase;
    long    numOfConfirms;
    long    ValueNano;
    long    txIdx;
};

struct WalletTransaction {
    enum TRANSACTION_TYPE {SEND=1, RECIEVE=2, CANCELLED=0x8000};

    long    txIdx;
    uint    transactionType;
    QString txid;
    QString address;
    QString creationTime;
    bool    confirmed;
    QString confirmationTime;
    long    coinNano; // Net diffrence
    bool    proof;
};

struct WalletProofInfo {
    long coinsNano;
    QString fromAddress;
    QString toAddress;
    QString output;
    QString kernel;
};

struct WalletUtxoSignature {
    long coinNano; // Output amount
    QString messageHash;
    QString pubKeyCompressed;
    QString messageSignature;
};


// Interface to wallet functionality
// can throw MwcException to signal errors
class Wallet
{
public:
    Wallet();
    virtual ~Wallet();

    // Open the wallet
    enum InitWalletStatus {OK, NEED_INIT};
    virtual InitWalletStatus open(const char * password) noexcept(false)  = 0;

    // Close the wallet and release the process
    virtual bool close() noexcept(false) = 0;

    // Create the wallet, generate the seed. Return the words to recover the wallet
    virtual QVector<QString> init() noexcept(false) = 0;

    // Recover the wallet with a mnemonic phrase
    // return: <success, ErrorMessage>
    virtual QPair<bool, QString> recover(QVector<QString> ) noexcept(false) = 0;

    //--------------- Listening

    // Checking if wallet is listening through services
    enum ListenState {LISTEN_OFFLINE = 0, LISTEN_MWCBOX = 0x1, LISTEN_KEYSTONE = 0x2};
    virtual uint getListeningStatus() noexcept(false) = 0;

    // Start listening through services
    virtual bool startListening(ListenState lstnState) noexcept(false) = 0;

    // Stop listening through services
    virtual bool stopListening(ListenState lstnState) noexcept(false) = 0;

    // Get MWC box <address, index in the chain>
    virtual QPair<QString,int> getMwcBoxAddress() noexcept(false) = 0;

    // Change MWC box address to another from the chain. idx - index in the chain.
    virtual void changeMwcBoxAddress(int idx) noexcept(false) = 0;

    // --------------- Foreign API

    // Check if foregn API is running
    virtual bool isForegnApiRunning() noexcept(false) = 0;

    // Setup foregn API. Needed to recieve money though HTTP. Start that service
    virtual bool startForegnAPI(int port, QString foregnApiSecret) noexcept(false) = 0;

    // -------------- Accounts

    //  Get list of open account
    virtual QVector<QString> getAccount() noexcept(false) = 0;

    // Create another account, note no delete exist for accounts
    virtual bool createAccount( const QString & accountName ) noexcept(false) = 0;

    // Switch to different account
    virtual bool switchAccount(const QString & accountName) noexcept(false) = 0;

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    virtual void check() noexcept(false) = 0;

    // Get current configuration of the wallet.
    virtual WalletConfig getWalletConfig() noexcept(false) = 0;

    // Update wallet config. Will be updated with non empty fields
    virtual bool setWalletConfig(const WalletConfig & config) noexcept(false) = 0;

    // -------------- Transactions

    // Get wallet balance
    virtual WalletInfo getWalletBalance() noexcept(false) = 0;

    // Cancel transaction
    virtual bool cancelTransacton(QString transactionID) noexcept(false) = 0;

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    virtual WalletProofInfo  generateMwcBoxTransactionProof( int transactionId, QString resultingFileName ) noexcept(false) = 0;
    // Verify the proof for transaction
    virtual WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) = 0;

    // Init send transaction with file output
    virtual void sendFile( long coinNano, QString fileTx ) noexcept(false) = 0;
    // Recieve transaction. Will generate *.response file in the same dir
    virtual void receiveFile( QString fileTx ) noexcept(false) = 0;
    // finalize transaction and broadcast it
    virtual void finalizeFile( QString fileTxResponse ) noexcept(false) = 0;

    // Send some coins to address.
    virtual void sendTo( long coinNano, const QString & address, QString message="", int inputConfirmationNumber=-1, int changeOutputs=-1 ) noexcept(false) = 0;

    // Show outputs for the wallet
    virtual QVector<WalletOutput> getOutputs() noexcept(false) = 0;
    // Show all transactions for account
    virtual QVector<WalletTransaction> getTransactions() noexcept(false) = 0;

    // -------------- Contacts

    // Get the contacts
    virtual QVector<WalletContact> getContacts() noexcept(false) = 0;
    // Add new contact
    virtual void addContact( const WalletContact & contact ) noexcept(false) = 0;
    // Remove contact. return false if not found
    virtual bool deleteContact( const QString & name ) noexcept(false) = 0;

    // ----------- HODL
    // https://github.com/mimblewimble/grin/pull/2374

    // sign output commitment utxo. Hash must be provivded by party that want to verify the signature.
    // Late this signature can be used for verification of ounewship
    virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) = 0;


};

#endif // MWCWALLET_H
