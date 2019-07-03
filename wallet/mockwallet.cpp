#include "mockwallet.h"
#include "../util/ioutils.h"
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QThread>
#include <QTime>
#include <QApplication>
#include "../util/Log.h"
#include "../control/messagebox.h"

namespace wallet {

// Mock wallet is not smart, constant seed is fine
static QVector<QString> MockWalletSeed{"local", "donor", "often", "upon", "copper", "minimum",
                        "message", "gossip", "vendor", "route", "rival", "brick",
                        "suffer", "gravity", "mom", "daring", "else", "exile",
                        "brush", "mansion", "shift", "load", "harbor", "close"};

const static QString settingsFileName("mwcwallet.dat");

static QVector<QString> boxAddresses{
    "jkh2jhv88y7yWdo7nWtzYiCwHmFbbLxJyA5jbPr2HeTH",
    "kdfjiewuyrqwieudkj387423942jkkdjsfgsd938iufd",
    "eriweriutyreity43985ieujhgfdsjh43057qpojlkdj",
    "587634ijkdhgfjkdhgvqo87rt562974qrdjfkgfsdhfg"};

MockWallet::MockWallet()
{
    dataPath = ioutils::getAppDataPath("mockwallet");

    accounts.push_back("default");

    WalletContact cnt;
    cnt.setData("bob", "43596834659876323897658564253876538");
    contacts.push_back(cnt);

    for (int t=0; t<10;t++)
    { // init transactions with some default values
        WalletTransaction wt;
        wt.setData(4,
           WalletTransaction::SEND,
           "8454935873498593487",
           "xd7auPddUmmEzSte48a2aZ9tWkjjCppgn41pemUfcVSqjxHHZ6cT",
           "Apr 26 18:55:00",
           false,
           "",
           1000000000L * 9,
           true);
        transactions.push_back(wt);

        wt.setData(5,
           WalletTransaction::RECIEVE,
           "8454935873498593487",
           "xd7546UmmEzSte48a2aZ9tWkjjCppgn41pemUfcVSqjxHHZ6cT",
           "Apr 26 19:58:11",
           true,
           "",
           1000000000L * 10,
           true);
        transactions.push_back(wt);

        wt.setData(6,
           WalletTransaction::RECIEVE,
           "8454935873498593487",
           "xd7auPdkfjsld;fkfsld;jjCppgn41pemUfcVSqjxHHZ6cT",
           "Apr 27 19:01:02",
           true,
           "",
           1000000000L * 12,
           true);
        transactions.push_back(wt);
    }

    loadData();
}

MockWallet::~MockWallet() {
    saveData();
}


// Generic. Reporting fatal error that somebody will process and exit app
void MockWallet::reportFatalError( QString message ) noexcept(false) {
    control::MessageBox::message(nullptr, "Critical Error", "We get a critical error from underneath layer and need to close appclication.\nError: " + message);
    QApplication::quit();
}

// Get all notification messages
const QVector<WalletNotificationMessages> & MockWallet::getWalletNotificationMessages()
{
    return notificationMessages;
}


void MockWallet::start() noexcept(false) {
    if (isInit ) {
        initStatus = InitWalletStatus::NEED_PASSWORD;
    }
    else {
        initStatus = InitWalletStatus::NEED_INIT;
    }
    logger::logEmit("mockWallet", "onInitWalletStatus", toString(initStatus) );
    emit onInitWalletStatus(initStatus);
}

void MockWallet::loginWithPassword(QString password) noexcept(false) {
    if ( walletPassword == QString(password) ) {
        initStatus = InitWalletStatus::READY;
    }
    else {
        initStatus = InitWalletStatus::WRONG_PASSWORD;
    }
    logger::logEmit("mockWallet", "onInitWalletStatus", toString(initStatus) );
    emit onInitWalletStatus(initStatus);
}



bool MockWallet::close() {
    return true;
}

void MockWallet::generateSeedForNewAccount(QString password) {
    Q_UNUSED(password);

    // init a new wallet, generate a new seed.
    emit onNewSeed(MockWalletSeed);
}

void MockWallet::confirmNewSeed() {
    isInit = true;
}


// Recover the wallet with a mnemonic phrase
// recover wallet with a passphrase:
// Check Signals: onRecoverProgress( int progress, int maxVal );
// Check Signals: onRecoverResult(bool ok, QString newAddress );
void MockWallet::recover(const QVector<QString> & seed, QString password) noexcept(false) {
    Q_UNUSED(seed);
    Q_UNUSED(password);
    emit onRecoverProgress(10,1000);
    emit onRecoverProgress(100,1000);
    emit onRecoverProgress(900,1000);
    emit onRecoverProgress(1000,1000);

    isInit = true;

    emit onRecoverResult(true, true, "new_mwcmq_addres_ieurqkf320847hgfjhg", QStringList() );
}

//--------------- Listening

// Checking if wallet is listening through services
// return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
QPair<bool,bool> MockWallet::getListeningStatus() noexcept(false) {
    return QPair<bool,bool>(listenMwcBox, listenKeybase);
}

// Start listening through services
// Check Signal: onStartListening
void MockWallet::listeningStart(bool startMq, bool startKb) noexcept(false) {
    if (startMq)
        listenMwcBox = true;
    if (startKb)
        listenKeybase = true;

    emit onListeningStartResults( startMq, startKb, // what we try to start
            QStringList() );
}

// Stop listening through services
// Check signal: onListeningStopResult
void MockWallet::listeningStop(bool stopMq, bool stopKb) noexcept(false) {
    if (stopMq)
        listenMwcBox = false;
    if (stopKb)
        listenKeybase = false;

    emit onListeningStopResult(stopMq, stopKb, // what we try to start
                                  QStringList() );
}


// Get latest Mwc MQ address that we see
QString MockWallet::getLastKnownMwcBoxAddress() noexcept(false) {
    return boxAddresses[ currentAddressIdx % boxAddresses.size() ];
}

// Get MWC box <address, index in the chain>
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MockWallet::getMwcBoxAddress() noexcept(false) {
    emit onMwcAddressWithIndex( boxAddresses[ currentAddressIdx % boxAddresses.size() ], currentAddressIdx );
}

// Change MWC box address to another from the chain. idx - index in the chain.
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MockWallet::changeMwcBoxAddress(int idx) noexcept(false) {
    currentAddressIdx = std::max(0, idx);
    emit onMwcAddressWithIndex( boxAddresses[ currentAddressIdx % boxAddresses.size() ], currentAddressIdx );
}

// Generate next box address
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MockWallet::nextBoxAddress() noexcept(false) {
    currentAddressIdx++;
    emit onMwcAddressWithIndex( boxAddresses[ currentAddressIdx % boxAddresses.size() ], currentAddressIdx );
}


// Create another account, note no delete exist for accounts
void MockWallet::createAccount( const QString & accountName )
{
    emit onAccountCreated(accountName);
}

// Switch to different account
void MockWallet::switchAccount(const QString & accountName) {
    emit onAccountSwitched(accountName);
}

// Check and repair the wallet. Will take a while
void MockWallet::check() {
    //QThread::sleep(5);
}

// Get current configuration of the wallet.
WalletConfig MockWallet::getWalletConfig() {
    return config;
}

// Update wallet config. Will be updated with non empty fields
QPair<bool, QString> MockWallet::setWalletConfig(const WalletConfig & cfg) {
    config = cfg;
    return QPair<bool, QString>(true,"");
}

NodeStatus MockWallet::getNodeStatus() {
    NodeStatus status;
    status.setData(17, "Ready", 73264, 9352984);
    return status;
}


// Get wallet balance
QVector<AccountInfo> MockWallet::getWalletBalance(bool filterDeleted) const noexcept(false) {
    Q_UNUSED(filterDeleted);
    //QThread::sleep(3);

    int64_t m1 = 1000000000L;

    AccountInfo acc1;
    acc1.setData( "default",
            m1 * 7,
            m1, m1, m1*5, 1234, false);

    AccountInfo acc2;
    acc2.setData( "account 1",
            m1 * 7,
            m1, m1, m1*5, 1234, false);

    AccountInfo acc3;
    acc3.setData( "one more account",
            m1 * 7,
            m1, m1, m1*5, 1234, false);

    AccountInfo acc4;
    acc4.setData( "additional account",
            m1 * 7,
            m1, m1, m1*5, 1234, false);

    AccountInfo acc5;
    acc5.setData( "HODL-1",
            m1 * 7,
            m1, m1, m1*5, 1234, false);

    return QVector<AccountInfo>{acc1, acc2, acc3, acc4, acc5};
}

// Cancel transaction

// Cancel transaction
// Check Signal:  onCancelTransacton
void MockWallet::cancelTransacton(int64_t transactionID) noexcept(false) {
    emit onCancelTransacton(true, transactionID, "OK");
}

void MockWallet::generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName ) noexcept(false) {
    emit onExportProof(true, resultingFileName, "Placeholder for what this transaction mean");
}
void MockWallet::verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false)  {
    emit onVerifyProof(true, proofFileName, "Placeholder for what this transaction mean");
}

// Init send transaction with file output
// Check signal:  onSendFile
void MockWallet::sendFile( int64_t coinNano, QString fileTx ) noexcept(false) {
    emit onSendFile(true, QStringList(), fileTx);
}

// Recieve transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveFile
void MockWallet::receiveFile( QString fileTx) noexcept(false) {
    emit onReceiveFile( true, QStringList(), fileTx, fileTx+".response"  );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeFile
void MockWallet::finalizeFile( QString fileTxResponse ) noexcept(false) {
    emit onFinalizeFile( true, QStringList(), fileTxResponse );
}

// Send some coins to address.
// Before send, wallet always do the switch to account to make it active
// Check signal:  onSend
void MockWallet::sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address, QString message,
                     int inputConfirmationNumber, int changeOutputs ) noexcept(false) {
    Q_UNUSED(coinNano);
    Q_UNUSED(address);
    Q_UNUSED(message);
    Q_UNUSED(inputConfirmationNumber);
    Q_UNUSED(changeOutputs);

    WalletTransaction wt;
    // last is expected
    wt.setData(transactions.back().txIdx + 1,
               WalletTransaction::SEND,
               QString::number( transactions.size()*189765 ),
               address,
               QTime::currentTime().toString(),
               false,
               "",
               coinNano,
               false);
    transactions.push_back(wt);
    emit onSend( true, QStringList() );
}


// Show outputs for the wallet
QVector<WalletOutput> MockWallet::getOutputs() noexcept(false) {

    QVector<WalletOutput> result;

    WalletOutput wi;
    wi.setData("08710be0b3fffa79b9423f8e007709a815f237dcfd31340cfa1fdfefd823dca30e",
                23,
                3,
                true,
                WalletOutput::STATUS::Confirmed,
                false,
                4,
                1000000000L*9/2,
                9);

    result.push_back(wi);

    wi.status = WalletOutput::STATUS::Unconfirmed;
    result.push_back(wi);

    wi.outputCommitment = "0478532478593247852397592376590379475";
    result.push_back(wi);

    return result;
}

// Show all transactions for current account
// Check Signal: onTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions)
void MockWallet::getTransactions() noexcept(false) {
    emit onTransactions( "default", 1234, transactions );
}

// Get the contacts
QVector<WalletContact> MockWallet::getContacts() {
    return contacts;
}

QPair<bool, QString>  MockWallet::addContact( const WalletContact & contact ) {
    // check if exist first
    for (auto & cnt : contacts) {
        if (cnt.name == contact.name) {
            cnt = contact;
            return QPair<bool, QString>(false, "Contact for " + contact.name + " allready exist.");
        }
    }
    // add new if doesn't exist
    contacts.push_back(contact);
    return QPair<bool, QString>(true, "");
}

QPair<bool, QString>  MockWallet::deleteContact( const QString & name ) {
    for (auto ci = contacts.begin(); ci!=contacts.end(); ci++ ) {
        if (ci->name == name) {
            contacts.erase(ci);
            return QPair<bool, QString>(true, "");
        }
    }
    // contact not found
    return QPair<bool, QString>(false, "Not found contact '" + name + "' to delete.");
}

WalletUtxoSignature MockWallet::sign_utxo( const QString & utxo, const QString & hash ) {
    Q_UNUSED(utxo);
    Q_UNUSED(hash);
    WalletUtxoSignature signature;
    signature.setData(1000000000L*4, // Output amount
                 "495638497565837325468534",
                 "478593JHFUYRT*&^UTG*%T&FYUTFR",
                 "9854390843095827609674096");
    return signature;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void MockWallet::saveData() const {
    QString filePath = dataPath + "/" + settingsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        control::MessageBox::message(nullptr,
              "Error",
              "Unable to save MockWallet settings to " + filePath +
              "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x57670;

    out << isInit;
    // It is mock test wallet, that is why we can store the password
    out << walletPassword;

    out << int(accounts.size());
    for ( const QString & acc : accounts )
        out << acc;

    out << selectedAccount;

    config.saveData(out);

    out << int(contacts.size());
    for ( const WalletContact & wltCont : contacts )
        wltCont.saveData(out);

    out << currentAddressIdx;
}


bool MockWallet::loadData() {

    QFile file(dataPath + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
     }

     QDataStream in(&file);
     in.setVersion(QDataStream::Qt_5_7);

     int id = 0;
     in >> id;
     if (id!=0x57670)
         return false;

     in >> isInit;
     // It is mock test wallet, that is why we can store the password
     in >> walletPassword;

     int size = 0;
     in >> size;
     if (size<0 || size>100000)
         return false;

     accounts.resize(size);
     for ( QString & acc : accounts )
         in >> acc;

     if (id>=0x57668)
         in >> selectedAccount;

     if (!config.loadData(in))
         return false;

     size = 0;
     in >> size;
     if (size<0 || size>100000)
         return false;

     contacts.resize(size);
     for ( WalletContact & wltCont : contacts ) {
         if (!wltCont.loadData(in)) {
             contacts.clear();
             return false;
         }
     }

     if (id>=0x57669)
         in >> currentAddressIdx;

     return true;
 }

}
