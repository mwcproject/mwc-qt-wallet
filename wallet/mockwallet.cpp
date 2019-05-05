#include "mockwallet.h"
#include "../util/ioutils.h"
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QMessageBox>
#include <QThread>

namespace wallet {

// Mock wallet is not smart, constant seed is fine
static QVector<QString> MockWalletSeed{"local", "donor", "often", "upon", "copper", "minimum",
                        "message", "gossip", "vendor", "route", "rival", "brick",
                        "suffer", "gravity", "mom", "daring", "else", "exile",
                        "brush", "mansion", "shift", "load", "harbor", "close"};

const static QString settingsFileName("mwcwallet.dat");

MockWallet::MockWallet()
{
    dataPath = ioutils::initAppDataPath("mockwallet");

    accounts.push_back("default");

    WalletContact cnt;
    cnt.setData("bob", "43596834659876323897658564253876538");
    contacts.push_back(cnt);

    loadData();
}

MockWallet::~MockWallet() {
    saveData();
}

// return true is it s a first run for the wallet
bool MockWallet::isVeryFirstRun() {
    return walletPassword.length()==0;
}

Wallet::InitWalletStatus MockWallet::open(QString network, const QString & password) {
    if (! isInit ) {
        walletPassword = password;
        return Wallet::InitWalletStatus::NEED_INIT;
    }

    if ( walletPassword != QString(password) )
        return Wallet::InitWalletStatus::WRONG_PASSWORD;

    blockchainNetwork = network;

    return Wallet::InitWalletStatus::OK;
}

bool MockWallet::close() {
    return true;
}

QVector<QString> MockWallet::init() {
    // init a new wallet, generate a new seed.
    return MockWalletSeed;
}

void MockWallet::confirmNewSeed() {
    isInit = true;
}


QPair<bool, QString> MockWallet::recover(const QVector<QString> & seed ) {
    if (MockWalletSeed == seed) {
        isInit = true;
        return QPair<bool, QString>(true, "");
    }

    return QPair<bool, QString>(false, "Unable to recover the wallet with provivded seed");
}

uint MockWallet::getListeningStatus() {
    uint status = ListenState::LISTEN_OFFLINE;
    if (listenMwcBox)
        status |= ListenState::LISTEN_MWCBOX;
    if (listenKeystone)
        status |= ListenState::LISTEN_KEYSTONE;

    return status;
}

QPair<bool, QString> MockWallet::startListening(ListenState lstnState) {
    if (!lstnState)
        return QPair<bool, QString>(true,"");

    if (lstnState & ListenState::LISTEN_MWCBOX) {
        listenMwcBox = true;
        return QPair<bool, QString>(true,"");
    }
    if (lstnState & ListenState::LISTEN_KEYSTONE) {
        return QPair<bool, QString>(false, "Please start your keytone app first. Then retry to start listener");
    }

    throw core::MwcException("get Unknown ListenState value: " + QString::number(lstnState));
}

bool MockWallet::stopListening(ListenState lstnState) {
    if (!lstnState)
        return true;

    if (lstnState & ListenState::LISTEN_MWCBOX) {
        listenMwcBox = false;
        return true;
    }
    if (lstnState & ListenState::LISTEN_KEYSTONE) {
        listenKeystone = false;
        return true;
    }

    return true;
}

QPair<QString,int> MockWallet::getMwcBoxAddress() {
    return QPair<QString, int>("jkh2jhv88y7yWdo7nWtzYiCwHmFbbLxJyA5jbPr2HeTH6NamyxUr", 0);
}

void MockWallet::changeMwcBoxAddress(int idx) {
    idx=-1;
    return;
}

bool MockWallet::isForegnApiRunning() {
    return listenFogeignApi;
}

bool MockWallet::startForegnAPI(int port, QString foregnApiSecret) {
    Q_UNUSED(port);
    Q_UNUSED(foregnApiSecret);
    return true;
}

//  Get list of open account
QVector<QString> MockWallet::getAccountList() {
    return accounts;
}

// Create another account, note no delete exist for accounts
QPair<bool, QString> MockWallet::createAccount( const QString & accountName )
{
    if (std::count_if( accounts.begin(), accounts.end(), [&](QString acc) {return acc==accountName;} )>0)
        return QPair<bool, QString>(false, "Account with this name already exist");

    accounts.push_back(accountName);
    return QPair<bool, QString>(true, "");
}

// Switch to different account
QPair<bool, QString> MockWallet::switchAccount(const QString & accountName) {
    if (std::count_if( accounts.begin(), accounts.end(), [&](QString acc) {return acc==accountName;} )>0)
        return QPair<bool, QString>(true, "");

    return QPair<bool, QString>(false, "Account doesn't exist");
}

// Check and repair the wallet. Will take a while
void MockWallet::check() {
    QThread::sleep(5);
}

// Get current configuration of the wallet.
WalletConfig MockWallet::getWalletConfig() {
    return config;
}

// Update wallet config. Will be updated with non empty fields
bool MockWallet::setWalletConfig(const WalletConfig & cfg) {
    config = cfg;
    return true;
}

// Get wallet balance
WalletInfo MockWallet::getWalletBalance() {
    QThread::sleep(3);
    long coin = 1000000000L;
    WalletInfo wi;
    wi.setData("default", 20*coin, 2*coin, 1*coin, 17*coin);
    return wi;

}

// Cancel transaction
bool MockWallet::cancelTransacton(QString transactionID) {
    Q_UNUSED(transactionID);
    return true;
}

// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
WalletProofInfo  MockWallet::generateMwcBoxTransactionProof( int transactionId, QString resultingFileName ) {
    Q_UNUSED(transactionId);
    Q_UNUSED(resultingFileName);

    WalletProofInfo pi;
    pi.setData(1000000000L * 4,
               "xd7auPddUmmEzSte48a2aZ9tWkjjCppgn41pemUfcVSqjxHHZ6cT",
               "xd7sCQ9bQuQXp4yCn8GSELcuSxnpcPrPoEWJzvPBc5vxyXPQz6PJ",
               "08710be0b3fffa79b9423f8e007709a815f237dcfd31340cfa1fdfefd823dca30e",
               "099c8a166acd426481c1b09707b9e6cdabb69718ee3ca86694579bf98a42c0c80d");
    return pi;
}

// Verify the proof for transaction
WalletProofInfo  MockWallet::verifyMwcBoxTransactionProof( QString proofFileName ) {
    Q_UNUSED(proofFileName);

    WalletProofInfo pi;
    pi.setData(1000000000L * 4,
               "xd7auPddUmmEzSte48a2aZ9tWkjjCppgn41pemUfcVSqjxHHZ6cT",
               "xd7sCQ9bQuQXp4yCn8GSELcuSxnpcPrPoEWJzvPBc5vxyXPQz6PJ",
               "08710be0b3fffa79b9423f8e007709a815f237dcfd31340cfa1fdfefd823dca30e",
               "099c8a166acd426481c1b09707b9e6cdabb69718ee3ca86694579bf98a42c0c80d");
    return pi;
}

// Init send transaction with file output
QString MockWallet::sendFile( long coinNano, QString fileTx ) noexcept(false) {
    QFile file( fileTx );
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        stream << "Send stransaction for nano " << coinNano << "coins: " << endl;
    }
    return fileTx;
}

// Recieve transaction. Will generate *.response file in the same dir
QString MockWallet::receiveFile( QString fileTx )
{
    QString newFn = fileTx + ".response";
    QFile file( newFn );
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        stream << "response for file " << fileTx << endl;
    }
    return newFn;
}

// finalize transaction and broadcast it
QPair<bool, QString> MockWallet::finalizeFile( QString fileTxResponse ) {
    Q_UNUSED(fileTxResponse);
    return QPair<bool, QString>(true, "");
}

// Send some coins to address.
QPair<bool, QString>  MockWallet::sendTo( long coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs ) noexcept(false) {
    Q_UNUSED(coinNano);
    Q_UNUSED(address);
    Q_UNUSED(message);
    Q_UNUSED(inputConfirmationNumber);
    Q_UNUSED(changeOutputs);
    return QPair<bool, QString> (true,"");
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

// Show all transactions for account
QVector<WalletTransaction> MockWallet::getTransactions() {
    QVector<WalletTransaction>  result;

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

    result.push_back(wt);

    wt.setData(5,
       WalletTransaction::RECIEVE,
       "8454935873498593487",
       "xd7546UmmEzSte48a2aZ9tWkjjCppgn41pemUfcVSqjxHHZ6cT",
       "Apr 26 19:58:11",
       true,
       "",
       1000000000L * 10,
       true);

    result.push_back(wt);


    wt.setData(6,
       WalletTransaction::RECIEVE,
       "8454935873498593487",
       "xd7auPdkfjsld;fkfsld;jjCppgn41pemUfcVSqjxHHZ6cT",
       "Apr 27 19:01:02",
       true,
       "",
       1000000000L * 12,
       true);

    result.push_back(wt);

    return result;
}

// Get the contacts
QVector<WalletContact> MockWallet::getContacts() {
    return contacts;
}

void MockWallet::addContact( const WalletContact & contact ) {
    // check if exist first
    for (auto & cnt : contacts) {
        if (cnt.name == contact.name) {
            cnt = contact;
            return;
        }
    }
    // add new if doesn't exist
    contacts.push_back(contact);
}

bool MockWallet::deleteContact( const QString & name ) {
    for (auto ci = contacts.begin(); ci!=contacts.end(); ci++ ) {
        if (ci->name == name) {
            contacts.erase(ci);
            return true;
        }
    }
    // contact not found
    return false;
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
        QMessageBox::information(nullptr,
              "Unable to save settings",
              "Unable to save MockWallet settings to " + filePath +
              " Error: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_12);

    out << 0x57667;

    out << blockchainNetwork;
    out << isInit;
    // It is mock test wallet, that is why we can store the password
    out << walletPassword;

    out << int(accounts.size());
    for ( const QString & acc : accounts )
        out << acc;


    config.saveData(out);

    out << int(contacts.size());
    for ( const WalletContact & wltCont : contacts )
        wltCont.saveData(out);
}


bool MockWallet::loadData() {

    QFile file(dataPath + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
     }

     QDataStream in(&file);
     in.setVersion(QDataStream::Qt_5_12);

     int id = 0;
     in >> id;
     if (id!=0x57667)
         return false;

     in >> blockchainNetwork;
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

     return true;
 }

}
