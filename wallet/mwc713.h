#ifndef MWC713_H
#define MWC713_H

#include "wallet.h"
#include <QObject>

class QProcess;

namespace tries {
    class Mwc713InputParser;
}

namespace wallet {

// MWC version that we are expecting to ship
const QString WMC_713_VERSION("2.0.0");

class Mwc713State;
class Mwc713reader;
class Mwc713EventManager;

class MWC713 : public Wallet
{
    Q_OBJECT
public:

public:
    MWC713(QString mwc713path);
    virtual ~MWC713() override;

    // Generic. Reporting fatal error that somebody will process and exit app
    virtual void reportFatalError( QString message ) noexcept(false) override;

    // Get all notification messages
    virtual const QVector<WalletNotificationMessages> & getWalletNotificationMessages() noexcept(false) override {return notificationMessages;}
    // Check signal: onNewNotificationMessage

    // ---- Wallet Init Phase
    virtual void start(QString network) noexcept(false) override;
    virtual void loginWithPassword(QString password, QString account) noexcept(false) override;
    // Check signal: onInitWalletStatus
    virtual InitWalletStatus getWalletStatus() noexcept(false) override {return initStatus;}


    virtual bool close() noexcept(false) override {return true;}
    virtual QVector<QString> generateSeedForNewAccount() noexcept(false) override {return QVector<QString>();}
    // Confirm that user write the passphase
    virtual void confirmNewSeed() noexcept(false) override {}

    virtual QPair<bool, QString> recover(const QVector<QString> & seed) noexcept(false) override {return QPair<bool, QString>(true,"");}

    //--------------- Listening
    virtual uint getListeningStatus() noexcept(false) override {return 0;}
    virtual QPair<bool, QString> startListening(ListenState lstnState) noexcept(false) override  {return QPair<bool, QString>(true,"");}
    virtual QPair<bool, QString> stopListening(ListenState lstnState) noexcept(false) override  {return QPair<bool, QString>(true,"");}
    virtual QPair<QString,int> getMwcBoxAddress() noexcept(false) override  {return QPair<QString,int>("",0);}
    virtual void changeMwcBoxAddress(int idx) noexcept(false) override {}
    virtual void nextBoxAddress() noexcept(false) override {}


    // --------------- Foreign API
    virtual bool isForegnApiRunning() noexcept(false) override {return false;}
    virtual QPair<bool,QString> startForegnAPI(int port, QString foregnApiSecret) noexcept(false) override {return QPair<bool, QString>(true,"");}
    virtual QPair<bool,QString> stopForeignAPI() noexcept(false) override {return QPair<bool, QString>(true,"");}

    // -------------- Accounts

    //  Get list of open account
    virtual QVector<QString> getAccountList() noexcept(false) override {return QVector<QString>();}

    // Create another account, note no delete exist for accounts
    virtual QPair<bool, QString> createAccount( const QString & accountName ) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    // Switch to different account
    virtual QPair<bool, QString> switchAccount(const QString & accountName) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    void check() noexcept(false) override {}

    virtual WalletConfig getWalletConfig() noexcept(false) override {return WalletConfig();}
    virtual QPair<bool, QString> setWalletConfig(const WalletConfig & config) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    // Status of the node
    virtual NodeStatus getNodeStatus() noexcept(false) override {return NodeStatus();}

    // -------------- Transactions
    virtual WalletInfo getWalletBalance() noexcept(false) override {return WalletInfo();}
    virtual bool cancelTransacton(QString transactionID) noexcept(false) override {return true;}

    virtual WalletProofInfo  generateMwcBoxTransactionProof( long transactionId, QString resultingFileName ) noexcept(false) override {return WalletProofInfo();}
    virtual WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) override {return WalletProofInfo();}

    virtual QPair<bool, QString> sendFile( long coinNano, QString fileTx ) noexcept(false) override  {return QPair<bool, QString>(true,"");}
    virtual QPair<bool, QString> receiveFile( QString fileTx, QString responseFileName ) noexcept(false) override  {return QPair<bool, QString>(true,"");}
    virtual QPair<bool, QString> finalizeFile( QString fileTxResponse ) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    virtual QPair<bool, QString> sendTo( long coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs ) noexcept(false) override  {return QPair<bool, QString>(true,"");}

    virtual QVector<WalletOutput> getOutputs() noexcept(false) override {return QVector<WalletOutput>();}
    // numOfTransactions - transaction limit to return. <=0 - get all transactions
    virtual QVector<WalletTransaction> getTransactions(int numOfTransactions=-1) noexcept(false) override {return QVector<WalletTransaction>();}

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
    void executeMwc713command(QString cmd);
public:
    // Task Reporting methods
    enum MESSAGE_LEVEL { FATAL_ERROR, CRITICAL, WARNING, INFO, DEBUG };
    enum MESSAGE_ID {INIT_ERROR, GENERIC, TASK_TIMEOUT };
    void appendNotificationMessage( MESSAGE_LEVEL level, MESSAGE_ID id, QString message );

    // Wallet init status
    enum INIT_STATUS {NONE, NEED_PASSWORD, NEED_SEED, WRONG_PASSWORD, READY };
    void setInitStatus( INIT_STATUS  initStatus );

    void setMwcAddress( QString mwcAddress ); // Set active MWC address. Listener might be offline
private:

private:
    QString mwc713Path; // path to the backed binary
    QProcess * mwc713process = nullptr;
    Mwc713reader * reader = nullptr;
    tries::Mwc713InputParser * inputParser = nullptr; // Parser will generate bunch of signals that wallet will listem on

    Mwc713EventManager * eventCollector = nullptr;

    // Stages (flags) of the wallet
    InitWalletStatus initStatus = InitWalletStatus::NONE;

    const int MESSAGE_SIZE_LIMIT = 10000;
    QVector<WalletNotificationMessages> notificationMessages;

    QString mwcAddress; // Address from mwc listener
};

}

#endif // MWC713_H
