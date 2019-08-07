#include "mwc713.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QThread>
#include "../tries/mwc713inputparser.h"
#include "mwc713events.h"
#include <QApplication>
#include "tasks/TaskStarting.h"
#include "tasks/TaskWallet.h"
#include "tasks/TaskListening.h"
#include "tasks/TaskRecover.h"
#include "tasks/TaskErrWrnInfoListener.h"
#include "tasks/TaskMwcMqAddress.h"
#include "tasks/TaskAccount.h"
#include "tasks/TaskSend.h"
#include "tasks/TaskTransaction.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/appcontext.h"
#include "../core/Config.h"
#include "../control/messagebox.h"
#include "../util/ConfigReader.h"
#include "../util/Files.h"
#include "../util/Waiting.h"

namespace wallet {

// Base class for wallet state, Non managed object. Consumer suppose to delete it
class Mwc713State {
    Mwc713State();
    virtual ~Mwc713State();
};

MWC713::MWC713(QString _mwc713path, QString _mwc713configPath, core::AppContext * _appContext) :
        appContext(_appContext), mwc713Path(_mwc713path),  mwc713configPath(_mwc713configPath) {

    currentAccount = appContext->getCurrentAccountName();
}

MWC713::~MWC713() {
    processStop(startedMode != STARTED_MODE::INIT);
}

// Generic. Reporting fatal error that somebody will process and exit app
void MWC713::reportFatalError( QString message )  {
    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::GENERIC, message );
}

// Check if waaled need to be initialized or not. Will run statndalone app, wait for exit and return the result
// Check signal: onWalletState(bool initialized)
bool MWC713::checkWalletInitialized() {

    qDebug() << "checkWalletState with " << mwc713Path << " and " << mwc713configPath;

    QProcess * process = initMwc713process( {}, {"state"}, false );

    if (process==nullptr)
        return false; // error expected to be reported by initMwc713process

    if ( !process->waitForFinished(20000) ) {
        appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to invalidate the status.\nPath: " + mwc713Path + "\nConfig:" + mwc713configPath );
        return false;
    }

    QString output = process->readAll();

    delete process;

    bool uninit = output.contains("Uninitialized");

    logger::logInfo("MWC713", QString("Wallet initialization checking status: ") + (uninit ? "Uninitialized" : "Initialized") );

    return !uninit;
}

// pass - provide password through env variable. If pass empty - nothing will be done
// paramsPlus - additional parameters for the process
QProcess * MWC713::initMwc713process(  const QStringList & envVariables, const QStringList & paramsPlus, bool trackProcessExit ) {
    // Creating process and starting
    QProcess * process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setWorkingDirectory( QDir::homePath() );

    if (!envVariables.isEmpty()) {
        Q_ASSERT(envVariables.size()%2==0);
        QProcessEnvironment env;

        for (int t=1; t<envVariables.size(); t+=2 ) {
            env.insert( envVariables[t-1], envVariables[t]);
        }

        process->setProcessEnvironment(env);
    }

    QStringList params{"--config", mwc713configPath, "-r", mwc::PROMPTS_MWC713 };
    params.append( paramsPlus );

    process->start(mwc713Path, params, QProcess::Unbuffered | QProcess::ReadWrite );

    bool startOk = process->waitForStarted(10000);
    if (!startOk) {
        switch (process->error())
        {
            case QProcess::FailedToStart:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start mwc713 located at " + mwc713Path );
                return nullptr;
            case QProcess::Crashed:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 crashed during start" );
                return nullptr;
            case QProcess::Timedout:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start because of timeout" );
                return nullptr;
            default:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start because of unknown error" );
                return nullptr;
        }
    }

    mwc713connect(process, trackProcessExit);

    return process;
}

// normal start. will require the password
void MWC713::start(bool loginWithLastKnownPassword)  {
    startedMode = STARTED_MODE::NORMAL;

    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    mwc713process = initMwc713process({}, {} );

    inputParser = new tries::Mwc713InputParser();

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);
    // Add first init task
    eventCollector->addTask( new TaskStarting(this), TaskStarting::TIMEOUT );

    // Adding permanent listeners
    eventCollector->addListener( new TaskRecoverProgressListener(this) );
    eventCollector->addListener( new TaskErrWrnInfoListener(this) );
    eventCollector->addListener( new TaskListeningListener(this) );
    eventCollector->addListener( new TaskSlatesListener(this) );

    // And eventing magic should begin...
    if (loginWithLastKnownPassword)
        loginWithPassword(walletPassword);
}

// start to init. Expected that we will exit pretty quckly
// Check signal: onNewSeed( seed [] )
void MWC713::start2init(QString password) {
    startedMode = STARTED_MODE::INIT;
    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 as init at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    mwc713process = initMwc713process({"MWC_PASSWORD", password}, {"init"} );

    inputParser = new tries::Mwc713InputParser();

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);

    // Adding permanent listeners
    eventCollector->addListener( new TaskErrWrnInfoListener(this) );

    // Init task
    eventCollector->addTask( new TaskInit(this), TaskInit::TIMEOUT );
}

// Recover the wallet with a mnemonic phrase
// recover wallet with a passphrase:
// Check Signals: onRecoverProgress( int progress, int maxVal );
// Check Signals: onRecoverResult(bool ok, QString newAddress );
void MWC713::start2recover(const QVector<QString> & seed, QString password) {
    startedMode = STARTED_MODE::RECOVER;

    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 as init at " << mwc713Path << " for config " << mwc713configPath;

    QString seedStr;
    for ( auto & s : seed) {
        if (!seedStr.isEmpty())
            seedStr+=" ";
        seedStr+=s;
    }

    // Creating process and starting
    // Mnemonic will moved into variables
    mwc713process = initMwc713process({"MWC_PASSWORD", password, "MWC_MNEMONIC", seedStr}, {"recover", "--mnemonic", "env" } );

    inputParser = new tries::Mwc713InputParser();

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);
    // Add first init task
    eventCollector->addTask( new TaskRecoverFull( this), TaskRecoverFull::TIMEOUT );

    // Adding permanent listeners
    eventCollector->addListener( new TaskErrWrnInfoListener(this) );
    eventCollector->addListener( new TaskRecoverProgressListener(this) );
}

// Need for claiming process only
// Starting the wallet and get the next key
// wallet713> getnextkey --amount 1000000
// "Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)"
// Check Signal: onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage);
void MWC713::start2getnextkey( int64_t amountNano, QString btcaddress, QString airDropAccPassword ) {
    startedMode = STARTED_MODE::GET_NEXTKEY;

    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 for getnextkey at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    // Mnemonic will moved into variables
    // !!!!! Security breach
    mwc713process = initMwc713process({}, {} );
    inputParser = new tries::Mwc713InputParser();
    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);

    // Excuting the single command and then read all output

    eventCollector->addTask( new TaskStarting(this), TaskStarting::TIMEOUT );
    eventCollector->addTask( new TaskInitW(this, walletPassword ), TaskInitW::TIMEOUT );
    eventCollector->addTask( new TaskInitWpressEnter(this ), TaskInitWpressEnter::TIMEOUT );
    eventCollector->addTask( new TaskGetNextKey(this,amountNano, btcaddress, airDropAccPassword ), TaskGetNextKey::TIMEOUT );
    // then exit
    eventCollector->addTask( new TaskLogout(this), TaskLogout::TIMEOUT);
}

// Need for claiming process only
// identifier  - output from start2getnextkey
// Check Signal: onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
void MWC713::start2recieveSlate( QString recieveAccount, QString identifier, QString slateFN ) {
    startedMode = STARTED_MODE::RECIEVE_SLATE;

    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 recieveSlate at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    // Mnemonic will moved into variables
    mwc713process = initMwc713process({}, {} );

    inputParser = new tries::Mwc713InputParser();

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);

    // Adding permanent listeners
    eventCollector->addListener( new TaskErrWrnInfoListener(this) );


    // Add first init task
    eventCollector->addTask( new TaskStarting(this), TaskStarting::TIMEOUT );
    // log in
    eventCollector->addTask( new TaskUnlock(this, walletPassword), TaskUnlock::TIMEOUT );

    eventCollector->addTask( new TaskSetReceiveAccount(this, recieveAccount, walletPassword), TaskSetReceiveAccount::TIMEOUT );
    // Recieve file first
    eventCollector->addTask( new TaskReceiveFile( this, slateFN, identifier ), TaskReceiveFile::TIMEOUT );
    // then exit
    eventCollector->addTask( new TaskLogout(this), TaskLogout::TIMEOUT);
}

void MWC713::processStop(bool exitNicely) {
    mwc713disconnect();

    // reset mwc713 interna; state
    //initStatus = InitWalletStatus::NONE;
    mwcAddress = "";
    accountInfo.clear();
    currentAccount = "default"; // Keep current account by name. It fit better to mwc713 interactions.
    collectedAccountInfo.clear();

    if (mwcMqOnline)
        emit onMwcMqListenerStatus(false);

    if (keybaseOnline)
        emit onKeybaseListenerStatus(false);

    mwcMqOnline = false;
    keybaseOnline = false;

    emit onMwcAddress("");
    emit onMwcAddressWithIndex("",1);

    // No balance updated because it is a trigger that wallet is ready
    // emit onWalletBalanceUpdated();

    if (mwc713process) {
        if (exitNicely) {
            executeMwc713command("exit", "");
            if (!mwc713process->waitForFinished(3000) ) {
                mwc713process->terminate();
                mwc713process->waitForFinished(5000);
            }
        }
        else {
            // init state have to be killed. Otherwise it will create a
            // seed without verification. We don't want that
            mwc713process->kill();
        }

        mwc713process->deleteLater();
        mwc713process = nullptr;
    }

    if (inputParser) {
        inputParser->deleteLater();
        inputParser = nullptr;
    }

    if (eventCollector) {
        eventCollector->clear();
        eventCollector->deleteLater();
        eventCollector = nullptr;
    }


}

// Check signal: onLoginResult(bool ok)
void MWC713::loginWithPassword(QString password)  {
    walletPassword = password;
    eventCollector->addTask( new TaskUnlock(this, password), TaskUnlock::TIMEOUT );
}

// Exit from the wallet. Expected that state machine will switch to Init state
// syncCall - stop NOW. Caller suppose to understand what he is doing
void MWC713::logout(bool syncCall)  {
    if (syncCall)
        processStop(true);
    else 
        eventCollector->addTask( new TaskStop(this), TaskStop::TIMEOUT );
}


void MWC713::confirmNewSeed()  {
    // Just pressing the enter
    executeMwc713command( "", "press ENTER");
    // Wallet status is Ready now...
//    setInitStatus(INIT_STATUS::READY);
}


// Current seed for runnign wallet
// Check Signals: onGetSeed(QVector<QString> seed);
void MWC713::getSeed()  {
    // Need stop listeners first

    QPair<bool,bool> lsnStatus = getListeningStatus();

    if (lsnStatus.first)
        listeningStop(true, false);

    if (lsnStatus.second)
        listeningStop(false, true);

    eventCollector->addTask( new TaskRecoverShowMnenonic(this, walletPassword ), TaskRecoverShowMnenonic::TIMEOUT );

    if (lsnStatus.first)
        listeningStart(true, false);

    if (lsnStatus.second)
        listeningStart(false, true);
}


// Checking if wallet is listening through services
// return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
QPair<bool,bool> MWC713::getListeningStatus()  {
    return QPair<bool,bool>(mwcMqOnline, keybaseOnline);
}

// Check Signal: onListeningStartResults
void MWC713::listeningStart(bool startMq, bool startKb)  {
    qDebug() << "listeningStart: mq=" << startMq << ",kb=" << startKb;
    eventCollector->addTask( new TaskListeningStart(this, startMq,startKb), TaskListeningStart::TIMEOUT );
}

// Check signal: onListeningStopResult
void MWC713::listeningStop(bool stopMq, bool stopKb)  {
    qDebug() << "listeningStop: mq=" << stopMq << ",kb=" << stopKb;
    eventCollector->addTask( new TaskListeningStop(this, stopMq,stopKb), TaskListeningStop::TIMEOUT );
}

// Get latest Mwc MQ address that we see
QString MWC713::getLastKnownMwcBoxAddress()  {
    return mwcAddress;
}

// Get MWC box <address, index in the chain>
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MWC713::getMwcBoxAddress()  {
    eventCollector->addTask( new TaskMwcMqAddress(this,false, -1), TaskMwcMqAddress::TIMEOUT );
}

// Change MWC box address to another from the chain. idx - index in the chain.
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MWC713::changeMwcBoxAddress(int idx)  {
    eventCollector->addTask( new TaskMwcMqAddress(this,true, idx), TaskMwcMqAddress::TIMEOUT );
}

// Generate next box address
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MWC713::nextBoxAddress()  {
    eventCollector->addTask( new TaskMwcMqAddress(this,true, -1), TaskMwcMqAddress::TIMEOUT );
}

QVector<AccountInfo>  MWC713::getWalletBalance(bool filterDeleted) const  {
    if (!filterDeleted)
        return accountInfo;

    QVector<AccountInfo> res;

    for (const auto & acc : accountInfo ) {
        if (!acc.isDeleted())
            res.push_back(acc);
    }

    if (res.isEmpty()) {
        // add default with 0 balance
        AccountInfo ai;
        ai.setData("default", 0, 0, 0, 0, 0, false);
        res.push_back(ai);
    }

    return res;
}


// Request Wallet balance update. It is a multistep operation
// Check signal: onWalletBalanceUpdated
//          onWalletBalanceProgress
void MWC713::updateWalletBalance()  {
    // Steps:
    // 1 - list accounts (this call)
    // 2 - for every account get info ( see updateAccountList call )
    // 3 - restore back current account

    eventCollector->addTask( new TaskAccountList(this), TaskAccountList::TIMEOUT );
}

// Create another account, note no delete exist for accounts
// Check Signal:  onAccountCreated
void MWC713::createAccount( const QString & accountName )  {

    // First try to rename one of deleted accounts.
    int delAccIdx = -1;

    for (int t=0; t<accountInfo.size(); t++) {
        if ( accountInfo[t].isDeleted() ) {
            delAccIdx = t;
            break;
        }
    }

    if (delAccIdx<0) {
        eventCollector->addTask( new TaskAccountCreate(this, accountName), TaskAccountInfo::TIMEOUT );
    }
    else {
        eventCollector->addTask( new TaskAccountRename(this, accountInfo[delAccIdx].accountName, accountName, true ), TaskAccountRename::TIMEOUT );
    }



}

// Switch to different account
// Check Signal: onAccountSwitched
void MWC713::switchAccount(const QString & accountName)  {
    // Expected that account is in the list
    eventCollector->addTask( new TaskAccountSwitch(this, accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );
}

// Rename account
// Check Signal: onAccountRenamed(bool success, QString errorMessage);
void MWC713::renameAccount(const QString & oldName, const QString & newName)  {
    eventCollector->addTask( new TaskAccountRename(this, oldName, newName, false), TaskAccountRename::TIMEOUT );
}

// Check and repair the wallet. Will take a while
// Check Signals: onRecoverProgress( int progress, int maxVal );
// Check Signals: onCheckResult(bool ok, QString errors );
void MWC713::check(bool wait4listeners)  {
    eventCollector->addTask( new TaskCheck(this,wait4listeners), TaskCheck::TIMEOUT );
}


// Send some coins to address.
// Before send, wallet always do the switch to account to make it active
// Check signal:  onSend
void MWC713::sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs )  {
    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account.accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    // If listening, strting...

    eventCollector->addTask( new TaskSendMwc(this, coinNano, address, message, inputConfirmationNumber, changeOutputs), TaskSendMwc::TIMEOUT );
    // Set some funds

}


// Init send transaction with file output
// Check signal:  onSendFile
void MWC713::sendFile( const wallet::AccountInfo &account, int64_t coinNano, QString message, QString fileTx )  {
    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account.accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );

    eventCollector->addTask( new TaskSendFile(this, coinNano, message, fileTx ), TaskSendFile::TIMEOUT );
}

// Recieve transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveFile
void MWC713::receiveFile( QString fileTx)  {
    eventCollector->addTask( new TaskReceiveFile(this, fileTx), TaskReceiveFile::TIMEOUT );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeFile
void MWC713::finalizeFile( QString fileTxResponse )  {
    eventCollector->addTask( new TaskFinalizeFile(this, fileTxResponse), TaskFinalizeFile::TIMEOUT );
}

// Get total number of Outputs
// Check Signal: onOutputCount(int number)
void MWC713::getOutputCount(QString account)  {
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskOutputCount(this, account), TaskOutputCount::TIMEOUT );
}

// Show outputs for the wallet
// Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> Transactions)
void MWC713::getOutputs(QString account, int offset, int number)  {
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskOutputs(this, offset, number), TaskOutputs::TIMEOUT );
}

// Get total number of Transactions
// Check Signal: onTransactionCount(int number)
void MWC713::getTransactionCount(QString account) {
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactionCount(this, account), TaskTransactions::TIMEOUT );
}

void MWC713::getTransactions(QString account, int offset, int number)  {
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactions(this, offset, number), TaskTransactions::TIMEOUT );
}

// -------------- Transactions

// Set account that will receive the funds
// Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
void MWC713::setReceiveAccount(QString account)  {
    eventCollector->addTask( new TaskSetReceiveAccount(this, account, walletPassword), TaskSetReceiveAccount::TIMEOUT );
}


// Cancel transaction
// Check Signal:  onCancelTransacton
void MWC713::cancelTransacton(int64_t transactionID)  {
    eventCollector->addTask( new TaskTransCancel(this, transactionID), TaskTransCancel::TIMEOUT );
}


// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
// Check Signal: onExportProof( bool success, QString fn, QString msg );
void MWC713::generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName )  {
    eventCollector->addTask( new TaskTransExportProof(this, resultingFileName, transactionId), TaskTransExportProof::TIMEOUT );
}

// Verify the proof for transaction
// Check Signal: onVerifyProof( bool success, QString msg );
void MWC713::verifyMwcBoxTransactionProof( QString proofFileName )  {
    eventCollector->addTask( new TaskTransVerifyProof(this, proofFileName), TaskTransExportProof::TIMEOUT );
}

// Status of the node
// Check Signal: onNodeSatatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections )
void MWC713::getNodeStatus() {
    eventCollector->addTask( new TaskNodeInfo(this), TaskNodeInfo::TIMEOUT );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Feed the command to mwc713 process
void MWC713::executeMwc713command(QString cmd, QString shadowStr) {
    Q_ASSERT(mwc713process);
    if (shadowStr.size()==0)
        logger::logMwc713in(cmd);
    else
        logger::logMwc713in( "CENSORED: " + shadowStr );

    mwc713process->write( (cmd + "\n").toLocal8Bit() );
}



// Task Reporting methods
//enum MESSAGE_LEVEL { FATAL_ERROR, WARNING, INFO, DEBUG }
//enum MESSAGE_ID {INIT_ERROR, GENERIC, TASK_TIMEOUT };
void MWC713::appendNotificationMessage( MESSAGE_LEVEL level, MESSAGE_ID id, QString message ) {
    Q_UNUSED(id);
    if (level == MESSAGE_LEVEL::FATAL_ERROR) {
        // Fatal error. Display message box and exiting. We don't want to continue
        control::MessageBox::message(nullptr, "mwc713 Error", "Wallet get a critical error from mwc713 process:\n" + message + "\n\nPress OK to exit the wallet" );
        mwc::closeApplication();
        return;
    }

    // Message is not fatal, adding it into the logs
    WalletNotificationMessages::LEVEL wlevel = WalletNotificationMessages::LEVEL::DEBUG;
    if (level == MESSAGE_LEVEL::CRITICAL)
        wlevel = WalletNotificationMessages::ERROR;
    else if (level == MESSAGE_LEVEL::WARNING)
        wlevel = WalletNotificationMessages::WARNING;
    else if (level == MESSAGE_LEVEL::INFO)
        wlevel = WalletNotificationMessages::INFO;

    WalletNotificationMessages msg(wlevel, message);
    notificationMessages.push_back( msg );

    while(notificationMessages.size()>MESSAGE_SIZE_LIMIT)
        notificationMessages.pop_front();

    logger::logEmit( "MWC713", "onNewNotificationMessage", msg.toString() );
    emit onNewNotificationMessage(msg.level, msg.message);
}

void MWC713::setLoginResult(bool ok) {
    logger::logEmit("MWC713", "onLoginResult", QString::number(ok) );
    emit onLoginResult(ok);

}

void MWC713::setGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor) {
    logger::logEmit("MWC713", "onGetNextKeyResult", QString::number(success) + " " + identifier + " " + publicKey + " " + errorMessage + " " + btcaddress + " " + airDropAccPasswor );
    emit onGetNextKeyResult(success, identifier, publicKey, errorMessage, btcaddress, airDropAccPasswor);
}


void MWC713::setMwcAddress( QString _mwcAddress ) { // Set active MWC address. Listener might be offline
    mwcAddress = _mwcAddress;
    logger::logEmit("MWC713", "onMwcAddress", mwcAddress );
    emit onMwcAddress(mwcAddress);
}

void MWC713::setMwcAddressWithIndex( QString _mwcAddress, int idx ) {
    mwcAddress = _mwcAddress;
    logger::logEmit("MWC713", "setMwcAddressWithIndex", mwcAddress + ", idx=" + QString::number(idx) );
    emit onMwcAddress(mwcAddress);
    emit onMwcAddressWithIndex(mwcAddress, idx);
}


void MWC713::setNewSeed( QVector<QString> seed ) {
    logger::logEmit("MWC713", "onNewSeed", "????" );
    emit onNewSeed(seed);
}

void MWC713::setGettedSeed( QVector<QString> seed ) {
    logger::logEmit("MWC713", "onGetSeed", "????" );
    emit onGetSeed(seed);
}


void MWC713::setListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                               QStringList errorMessages ) {
    logger::logEmit("MWC713", "onListeningStartResults", QString("mqTry=") + QString::number(mqTry) +
            " kbTry=" + QString::number(kbTry) + " errorMessages size " + QString::number(errorMessages.size()) );
    emit onListeningStartResults(mqTry, kbTry,errorMessages);
}

void MWC713::setListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                            QStringList errorMessages ) {
    logger::logEmit("MWC713", "onListeningStopResult", QString("mqTry=") + QString::number(mqTry) +
             " kbTry=" + QString::number(kbTry) + " errorMessages size " + QString::number(errorMessages.size()) );
    emit onListeningStopResult(mqTry, kbTry,errorMessages);

}

void MWC713::setMwcMqListeningStatus(bool online) {
    if (mwcMqOnline != online) {
        appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, (online ? "Start " : "Stop ") + QString("listening on mwc mq"));
    }
    mwcMqOnline = online;
    logger::logEmit("MWC713", "onMwcMqListenerStatus", QString("online=") + QString::number(online));
    emit onMwcMqListenerStatus(online);

}
void MWC713::setKeybaseListeningStatus(bool online) {
    if (keybaseOnline != online) {
        appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, (online ? "Start " : "Stop ") + QString("listening on keybase"));
    }
    keybaseOnline = online;
    logger::logEmit("MWC713", "onKeybaseListenerStatus", QString("online=") + QString::number(online));
    emit onKeybaseListenerStatus(online);
}

void MWC713::setRecoveryResults( bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages ) {
    logger::logEmit("MWC713", "onRecoverResult", QString("started=") + QString::number(started) +
           " finishedWithSuccess=" + QString::number(finishedWithSuccess) +
           " newAddress=" + newAddress + " errorMessages size " + QString::number(errorMessages.size()) );

    emit onRecoverResult(started, finishedWithSuccess, newAddress, errorMessages);

    // Update the address as well
    if ( newAddress.length()>0 ) {
        setMwcAddress(newAddress);
    }

    if (finishedWithSuccess) {
        appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("MWC wallet was successfully recovered from the mnemonic"));
    }
    else {
        appendNotificationMessage( MESSAGE_LEVEL::WARNING, MESSAGE_ID::GENERIC, QString("Failed to recover from the mnemonic"));
    }
}

void MWC713::setRecoveryProgress( int64_t progress, int64_t limit ) {
    logger::logEmit("MWC713", "onRecoverProgress", QString("progress=") + QString::number(progress) +
                                              " limit=" + QString::number(limit) );
    emit onRecoverProgress( int(progress), int(limit) );
}

// Apply accout list. Explory what does wallet has
void MWC713::updateAccountList( QVector<QString> accounts ) {
    collectedAccountInfo.clear();

    core::SendCoinsParams params = appContext->getSendCoinsParams();

    int idx = 0;
    for (QString acc : accounts) {
        eventCollector->addTask( new TaskAccountSwitch(this, acc, walletPassword, false), TaskAccountSwitch::TIMEOUT );
        eventCollector->addTask( new TaskAccountInfo(this, params.inputConfirmationNumber, idx>0), TaskAccountInfo::TIMEOUT );
        eventCollector->addTask( new TaskAccountProgress(this, idx++, accounts.size() ), -1 ); // Updating the progress
    }
    eventCollector->addTask( new TaskAccountListFinal(this, currentAccount), -1 ); // Finalize the task
    // Final will switch back to current account
}

void MWC713::updateAccountProgress(int accountIdx, int totalAccounts) {
    logger::logEmit( "MWC713", "onWalletBalanceProgress", "accountIdx=" + QString::number(accountIdx) + " totalAccounts=" + QString::number(totalAccounts) );
    emit onWalletBalanceProgress( accountIdx, totalAccounts );
}

void MWC713::updateAccountFinalize(QString prevCurrentAccount) {
    accountInfo = collectedAccountInfo;
    logger::logEmit( "MWC713", "updateAccountFinalize","");
    emit onWalletBalanceUpdated();

    // Set back the current account
    bool isDefaultFound = false;
    for (auto & acc : accountInfo) {
        if (acc.accountName == prevCurrentAccount) {
            isDefaultFound = true;
            break;
        }
    }

    if (!isDefaultFound && accountInfo.size()>0) {
        prevCurrentAccount = accountInfo[0].accountName;
    }

    // !!!!!! NOTE, 'false' mean that we don't save to that account. It make sence because during such long operation
    //  somebody could change account
    eventCollector->addTask( new TaskAccountSwitch(this, prevCurrentAccount, walletPassword, false), TaskAccountSwitch::TIMEOUT );
}

void MWC713::createNewAccount( QString newAccountName ) {
    // Add new account info into the list. New account is allways empty
    AccountInfo acc;
    acc.setData(newAccountName,0,0,0,0,0,false);
    accountInfo.push_back( acc );

    logger::logEmit( "MWC713", "onAccountCreated",newAccountName);
    logger::logEmit( "MWC713", "onWalletBalanceUpdated","");

    emit onAccountCreated(newAccountName);
    emit onWalletBalanceUpdated();

    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("New account '" + newAccountName + "' was created" ));
}

void MWC713::switchToAccount( QString switchAccountName, bool makeAccountCurrent ) {
    logger::logEmit( "MWC713", "onAccountSwitched",switchAccountName);
    if (makeAccountCurrent) {
        currentAccount = switchAccountName;
        appContext->setCurrentAccountName(currentAccount);
    }

    emit onAccountSwitched(switchAccountName);
}

void MWC713::updateRenameAccount(const QString & oldName, const QString & newName, bool createSimulation,
                         bool success, QString errorMessage) {

    // Apply rename step, we don't want to rescan because of that.
    for (auto & ai : accountInfo) {
        if (ai.accountName == oldName)
            ai.accountName = newName;
    }

    if (createSimulation) {
        logger::logEmit( "MWC713", "onAccountCreated",newName);

        emit onAccountCreated(newName);
    }
    else {
        logger::logEmit("MWC713", "onAccountRenamed",
                        oldName + " to " + newName + " success=" + QString::number(success) + " err=" + errorMessage);
        emit onAccountRenamed(success, errorMessage);
    }

    if (success)
        appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("Account '" + oldName + "' was renamed to '" + newName + "'" ));
    else
        appendNotificationMessage( MESSAGE_LEVEL::WARNING, MESSAGE_ID::GENERIC, QString("Failed to rename account '" + oldName + "'" ));

    logger::logEmit( "MWC713", "onWalletBalanceUpdated","");
    emit onWalletBalanceUpdated();

}

// Update with account info
void MWC713::infoResults( QString currentAccountName, int64_t height,
                  int64_t totalNano, int64_t waitingConfNano, int64_t lockedNano, int64_t spendableNano, bool mwcServerBroken ) {
    AccountInfo acc;
    acc.setData(currentAccountName,
                totalNano,
                waitingConfNano,
                lockedNano,
                spendableNano,
                height,
                mwcServerBroken);

    updateAccountInfo( acc, collectedAccountInfo, true );
    updateAccountInfo( acc, accountInfo, false );
}

void MWC713::setSendResults(bool success, QStringList errors) {
    logger::logEmit( "MWC713", "onSend", "success=" + QString::number(success) );
    emit onSend( success, errors );
}

void MWC713::reportSlateSend( QString slate, QString mwc, QString sendAddr ) {
    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("You successfully sent " + mwc + " mwc to " + sendAddr ));

    logger::logEmit( "MWC713", "onSlateSend", slate + " with " +mwc + " to " + sendAddr );
    emit onSlateSend(slate, mwc, sendAddr);
}
void MWC713::reportSlateRecieved( QString slate, QString mwc, QString fromAddr ) {
    logger::logEmit( "MWC713", "onSlateRecieved", slate + " with " +mwc + " from " + fromAddr );

    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("You received " + mwc+ " mwc form " + fromAddr ));

    emit onSlateRecieved( slate, mwc, fromAddr );

    // Request balace refresh
    updateWalletBalance();
}

void MWC713::reportSlateFinalized( QString slate ) {

    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("Slate finalized : "+slate ));

    logger::logEmit( "MWC713", "onSlateFinalized", slate );
    emit onSlateFinalized(slate);

    // Request balace refresh
    updateWalletBalance();
}

void MWC713::setSendFileResult( bool success, QStringList errors, QString fileName ) {

    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("File transaction was initiated for "+ fileName ));

    logger::logEmit( "MWC713", "onSendFile", "success="+QString::number(success) );
    emit onSendFile(success, errors, fileName);
}

void MWC713::setReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn ) {
    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("File receive transaction was processed for "+ inFileName ));

    logger::logEmit( "MWC713", "onReceiveFile", "success="+QString::number(success) );
    emit onReceiveFile( success, errors, inFileName, outFn );
}

void MWC713::setFinalizeFile( bool success, QStringList errors, QString fileName ) {
    appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("File finalized for "+ fileName ));

    logger::logEmit( "MWC713", "onFinalizeFile", "success="+QString::number(success) );
    emit onFinalizeFile( success, errors, fileName);
}

// Transactions
void MWC713::updateTransactionCount(QString account, int number) {
    logger::logEmit( "MWC713", "onTransactionCount", "number=" + QString::number(number) );
    emit onTransactionCount( account, number );
}

void MWC713::setTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions ) {
    logger::logEmit( "MWC713", "onTransactions", "account="+account );
    emit onTransactions( account, height, Transactions );
}

void MWC713::updateOutputCount(QString account, int number) {
    logger::logEmit( "MWC713", "onOutputCount", "number=" + QString::number(number) );
    emit onOutputCount( account, number );
}

void MWC713::setOutputs( QString account, int64_t height, QVector<WalletOutput> outputs) {
    logger::logEmit( "MWC713", "onOutputs", "account="+account );
    emit onOutputs( account, height, outputs );
}

void MWC713::setExportProofResults( bool success, QString fn, QString msg ) {
    logger::logEmit( "MWC713", "onExportProof", "success="+QString::number(success) );
    emit onExportProof( success, fn, msg );
}

void MWC713::setVerifyProofResults( bool success, QString fn, QString msg ) {
    logger::logEmit( "MWC713", "onVerifyProof", "success="+QString::number(success) );
    emit onVerifyProof(success, fn, msg);
}

void MWC713::setTransCancelResult( bool success, int64_t transId, QString errMsg ) {
    logger::logEmit( "MWC713", "onCancelTransacton", "success="+QString::number(success) );
    emit onCancelTransacton(success, transId, errMsg);
}

void MWC713::setSetReceiveAccount( bool ok, QString accountOrMessage ) {
    if (ok)
        appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, QString("Set receive account: '" + accountOrMessage + "'" ));

    logger::logEmit( "MWC713", "onSetReceiveAccount", "ok="+QString::number(ok) );
    emit onSetReceiveAccount(ok, accountOrMessage );
}

void MWC713::setCheckResult(bool ok, QString errors) {

    if (ok)
        appendNotificationMessage( MESSAGE_LEVEL::INFO, MESSAGE_ID::GENERIC, "Account re-sync was finished successfully.");
    else
        appendNotificationMessage( MESSAGE_LEVEL::WARNING, MESSAGE_ID::GENERIC, "Account re-sync was failed.");

    logger::logEmit( "MWC713", "onCheckResult", "ok="+QString::number(ok) );
    emit onCheckResult(ok, errors );
}

void MWC713::setNodeStatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections ) {
    logger::logEmit( "MWC713", "onNodeSatatus", "online="+QString::number(online) + " height="+QString::number(height) +
                          " totalDifficulty=" + QString::number(totalDifficulty) + " connections=" + QString::number(connections) );
    emit onNodeStatus( online, errMsg, height, totalDifficulty, connections );
}

/////////////////////////////////////////////////////////////////////////////////
//      mwc713  IOs

void MWC713::mwc713connect(QProcess * process, bool trackProcessExit) {
    mwc713disconnect();
    Q_ASSERT(mwc713connections.isEmpty());
    Q_ASSERT(process);

    if (process) {
        mwc713connections.push_back( connect( process, &QProcess::errorOccurred, this, &MWC713::mwc713errorOccurred, Qt::QueuedConnection) );

        if (trackProcessExit) {
            mwc713connections.push_back(connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                                                SLOT(mwc713finished(int, QProcess::ExitStatus))));
            mwc713connections.push_back( connect( process, &QProcess::readyReadStandardError, this, &MWC713::mwc713readyReadStandardError, Qt::QueuedConnection) );
            mwc713connections.push_back( connect( process, &QProcess::readyReadStandardOutput, this, &MWC713::mwc713readyReadStandardOutput, Qt::QueuedConnection) );
        }

    }
}

void MWC713::mwc713disconnect() {
    for (auto & cnt : mwc713connections) {
        disconnect(cnt);
    }

    mwc713connections.clear();
}

void MWC713::mwc713errorOccurred(QProcess::ProcessError error) {
    logger::logMwc713out("ERROR OCCURRED. Error = " + QString::number(error)  );

    qDebug() << "ERROR OCCURRED. Error = " << error;

    if (mwc713process) {
        mwc713process->deleteLater();
        mwc713process = nullptr;
    }

    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::MWC7113_ERROR,
                               "mwc713 process exited. Process error: "+ QString::number(error) );

}

void MWC713::mwc713finished(int exitCode, QProcess::ExitStatus exitStatus) {
    logger::logMwc713out("Exit with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );

    qDebug() << "mwc713 is exiting with exit code " << exitCode << ", exitStatus=" << exitStatus;

    if (mwc713process) {
        mwc713process->deleteLater();
        mwc713process = nullptr;
    }

    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::MWC7113_ERROR,
                               "mwc713 process exited due some unexpected error. mwc713 exit code: " + QString::number(exitCode) );
}

void MWC713::mwc713readyReadStandardError() {
    qDebug() << "get mwc713readyReadStandardError call !!!";
    Q_ASSERT(mwc713process);
    if (mwc713process) {
        QString str( ioutils::FilterEscSymbols( mwc713process->readAllStandardError() ) );

        appendNotificationMessage( MESSAGE_LEVEL::CRITICAL, MESSAGE_ID::MWC7113_ERROR,
                                   "mwc713 process report error:\n" + str );
    }
}

void MWC713::mwc713readyReadStandardOutput() {
    QString str( ioutils::FilterEscSymbols( mwc713process->readAllStandardOutput() ) );
    qDebug() << "Get output:" << str;
    logger::logMwc713out(str);
    inputParser->processInput(str);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//      Utils

// Update acc value at collection accounts. If account is not founf, we can add it (addIfNotFound) or skip
void MWC713::updateAccountInfo( const AccountInfo & acc, QVector<AccountInfo> & accounts, bool addIfNotFound ) const {

    for (auto & a : accounts) {
        if (a.accountName == acc.accountName) {
            a = acc;
            return;
        }
    }

    if (addIfNotFound)
        accounts.push_back(acc);

}

/////////////////////////////////////////////////////////////////////////
// Read config from the file
WalletConfig MWC713::readWalletConfig(QString source) const {
    util::ConfigReader  mwc713config;

    if (!mwc713config.readConfig(source) ) {
        control::MessageBox::message(nullptr, "Read failure", "Unable to read mwc713 configuration from " + source );
        return WalletConfig();
    }

    QString dataPath = mwc713config.getString("wallet713_data_path");
    QString keyBasePath = mwc713config.getString("keybase_binary");
    QString mwcmqDomain = mwc713config.getString("mwcmq_domain");

    if (dataPath.isEmpty() || keyBasePath.isEmpty() || mwcmqDomain.isEmpty()) {
        control::MessageBox::message(nullptr, "Read failure", "Not able to find all expected mwc713 configuration values at " + source );
        return WalletConfig();
    }

    return WalletConfig().setData(dataPath, mwcmqDomain, keyBasePath,
                                  mwc713config.getString("mwc_node_uri"),
                                  mwc713config.getString("mwc_node_secret") );
}


// Get current configuration of the wallet. will read from wallet713.toml file
WalletConfig MWC713::getWalletConfig()  {
    return readWalletConfig(config::getMwc713conf());
}

// Get configuration form the resource file.
WalletConfig MWC713::getDefaultConfig()  {
    return readWalletConfig( mwc::MWC713_DEFAULT_CONFIG );
}


// Update wallet config. Will update config and restart the wmc713.
// Note!!! Caller is fully responsible for input validation. Normally mwc713 will sart, but some problems might exist
//          and caller suppose listen for them
bool MWC713::setWalletConfig(const WalletConfig & config)  {
    if (!config.isDefined())
        return false;

    QString mwc713confFN = config::getMwc713conf();

    QStringList confLines = util::readTextFile( mwc713confFN );
    // Updating the config with new values

    QStringList newConfLines;

    for (QString & ln : confLines) {
        if ( ln.trimmed().isEmpty())
            continue; // skipping empty lines

        if (ln.startsWith("wallet713_data_path") || ln.startsWith("keybase_binary") || ln.startsWith("mwcmq_domain") ||
                                ln.startsWith("mwc_node_uri") || ln.startsWith("mwc_node_secret") ) {
            continue; // skippping the line. Will apply later
        }
        else {
            // keep whatever we have here
            newConfLines.append(ln);
        }
    }


    newConfLines.append("wallet713_data_path = \"" + config.dataPath + "\"");
    newConfLines.append("keybase_binary = \"" + config.keyBasePath + "\"");
    newConfLines.append("mwcmq_domain = \"" + config.mwcmqDomain + "\"");
    if ( !config.mwcNodeURI.isEmpty() && !config.mwcNodeSecret.isEmpty() ) {
        newConfLines.append("mwc_node_uri = \"" + config.mwcNodeURI + "\"");
        newConfLines.append("mwc_node_secret = \"" + config.mwcNodeSecret + "\"");
    }

    if (!util::writeTextFile( mwc713confFN, newConfLines )) {
        control::MessageBox::message(nullptr, "Read failure", "Not able to find all expected mwc713 configuration values at " + mwc713confFN );
        return false;
    }

    util::Waiting w; // Host verifucation might tale time, what is why waiting here

    // Stopping the wallet. Start will be done by init state and caller is responsible for that
    processStop(true); // sync if ok for this call
    return true;
}



}
