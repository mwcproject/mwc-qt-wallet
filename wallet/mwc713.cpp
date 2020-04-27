// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mwc713.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QTimer>
#include "../tries/mwc713inputparser.h"
#include "mwc713events.h"
#include <QApplication>
#include <core/Notification.h>
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
#include "../util/Process.h"
#include "../node/MwcNodeConfig.h"
#include "../node/MwcNode.h"
#include <QCoreApplication>

namespace wallet {

// Base class for wallet state, Non managed object. Consumer suppose to delete it
class Mwc713State {
    Mwc713State();
    virtual ~Mwc713State();
};

MWC713::MWC713(QString _mwc713path, QString _mwc713configPath, core::AppContext * _appContext) :
        appContext(_appContext), mwc713Path(_mwc713path),  mwc713configPath(_mwc713configPath) {

    currentAccount = appContext->getCurrentAccountName();

    // Listening for Output Locking changes
    QObject::connect(appContext, &core::AppContext::onOutputLockChanged, this, &MWC713::onOutputLockChanged, Qt::QueuedConnection);

    void    onOutputLockChanged(QString commit);
}

MWC713::~MWC713() {
    processStop(startedMode != STARTED_MODE::INIT);
}


// Check if waaled need to be initialized or not. Will run statndalone app, wait for exit and return the result
// Check signal: onWalletState(bool initialized)
bool MWC713::checkWalletInitialized() {

    qDebug() << "checkWalletState with " << mwc713Path << " and " << mwc713configPath;

    Q_ASSERT(mwc713process==nullptr);
    mwc713process = initMwc713process( {}, {"state"}, false );

    if (mwc713process==nullptr)
        return false;

    if (!util::processWaitForFinished( mwc713process, 3000, "mwc713")) {
        mwc713process->terminate();
        util::processWaitForFinished( mwc713process, 3000, "mwc713");
    }

    QString output = mwc713process->readAll();

    processStop(false);

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

    mwc713connect(process, trackProcessExit);

    if (!envVariables.isEmpty()) {
        Q_ASSERT(envVariables.size()%2==0);
        QProcessEnvironment env;

        for (int t=1; t<envVariables.size(); t+=2 ) {
            env.insert( envVariables[t-1], envVariables[t]);
        }

        process->setProcessEnvironment(env);
    }

    QStringList params{"--config", mwc713configPath, "--disable-history" ,"-r", mwc::PROMPTS_MWC713 };
    params.append( paramsPlus );

    walletStartTime = QDateTime::currentMSecsSinceEpoch();

    QString filePath = QFileInfo(mwc713Path).canonicalFilePath();
    if (filePath.isEmpty()) {
        // file not found. Let's  report it clear way
        logger::logInfo("MWC713", "error. mwc713 canonical path is empty");

        appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR, "mwc713 executable is not found. Expected location at:\n\n" + mwc713Path );
        return nullptr;

    }

    commandLine = "'" + filePath + "'";
    for (auto & p : params) {
        if (p=="-r" ||  p==mwc::PROMPTS_MWC713 )
            continue; // skipping prompt parameter. It is not needed for troubleshouting
        commandLine += " '" + p + "'";
    }

    logger::logInfo("MWC713", "Starting new process: " + commandLine);

    process->start(mwc713Path, params, QProcess::Unbuffered | QProcess::ReadWrite );

    while ( ! process->waitForStarted( (int)(10000 * config::getTimeoutMultiplier()) ) ) {

        logger::logInfo("MWC713", "mwc713 process failed to start");

        switch (process->error())
        {
            case QProcess::FailedToStart:
                appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR, "mwc713 failed to start mwc713 located at " + mwc713Path + "\n\nCommand line:\n\n" + commandLine );
                return nullptr;
            case QProcess::Crashed:
                appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR, "mwc713 crashed during start\n\nCommand line:\n\n" + commandLine );
                return nullptr;
            case QProcess::Timedout:
                if (control::MessageBox::questionText(nullptr, "Warning", QString("Starting for mwc713 process is taking longer than expected.\nContinue to wait?") +
                                                  "\n\nCommand line:\n\n" + commandLine,
                                                  "Yes", "No", true, false) == control::MessageBox::RETURN_CODE::BTN1) {
                    config::increaseTimeoutMultiplier();
                    continue; // retry with waiting
                }
                appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR, "mwc713 takes too much time to start. Something wrong with environment.\n\nCommand line:\n\n" + commandLine );
                return nullptr;
            default:
                appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR, "mwc713 failed to start because of unknown error.\n\nCommand line:\n\n" + commandLine );
                return nullptr;
        }
    }

    return process;
}

void MWC713::resetData(STARTED_MODE _startedMode ) {
    loggedIn = false;
    startedMode = _startedMode;
    mwcMqOnline = keybaseOnline = false;
    mwcMqStarted = mwcMqStartRequested = keybaseStarted = false;
    httpOnline = false;
    httpInfo = "";
    hasHttpTls = false;
    walletPassword = "";
    outputsLines.clear();
}

// normal start. will require the password
void MWC713::start(bool loginWithLastKnownPassword)  {
    qDebug() << "MWC713::start loginWithLastKnownPassword=" << loginWithLastKnownPassword;

    resetData(STARTED_MODE::NORMAL);

    // Need to check if Tls active
    WalletConfig config = getWalletConfig();
    hasHttpTls = !config.tlsCertificateKey.isEmpty() && !config.tlsCertificateFile.isEmpty();

    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    mwc713process = initMwc713process({}, {} );
    if (mwc713process==nullptr)
        return;

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
    eventCollector->addListener( new TaskSyncProgressListener(this) );

    // And eventing magic should begin...
    if (loginWithLastKnownPassword)
        loginWithPassword(walletPassword);
}

// start to init. Expected that we will exit pretty quckly
// Check signal: onNewSeed( seed [] )
void MWC713::start2init(QString password) {
    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    resetData(STARTED_MODE::INIT);

    qDebug() << "Starting MWC713 as init at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting

    mwc713process = initMwc713process({"MWC_PASSWORD", password}, {"init"} );
    if (mwc713process==nullptr)
        return;

    inputParser = new tries::Mwc713InputParser();

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);

    // Adding permanent listeners
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

    resetData(STARTED_MODE::RECOVER);

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
    if (mwc713process==nullptr)
        return;

    inputParser = new tries::Mwc713InputParser();

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);
    // Add first init task
    eventCollector->addTask( new TaskRecoverFull( this), TaskRecoverFull::TIMEOUT );

    // Adding permanent listeners
    eventCollector->addListener( new TaskErrWrnInfoListener(this) );
    eventCollector->addListener( new TaskRecoverProgressListener(this) );
}

void MWC713::processStop(bool exitNicely) {
    qDebug() << "MWC713::processStop exitNicely=" << exitNicely;

    logger::logInfo("MWC713", QString("mwc713 process exiting ") + (exitNicely? "nicely" : "by killing") );

    loggedIn = false;

    mwc713disconnect();

    // reset mwc713 interna; state
    //initStatus = InitWalletStatus::NONE;
    mwcAddress = "";
    accountInfoNoLocks.clear();
    walletOutputs.clear();
    currentAccount = "default"; // Keep current account by name. It fit better to mwc713 interactions.
    collectedAccountInfo.clear();

    if (mwcMqOnline)
        emit onMwcMqListenerStatus(false);

    if (keybaseOnline)
        emit onKeybaseListenerStatus(false);

    mwcMqOnline = keybaseOnline = false;
    mwcMqStarted = mwcMqStartRequested = keybaseStarted = false;

    httpOnline = false;
    httpInfo = "";

    emit onMwcAddress("");
    emit onMwcAddressWithIndex("",1);

    if (mwc713process) {
        if (exitNicely) {
            qDebug() << "start exiting...";
            executeMwc713command("exit", "");

            if (!util::processWaitForFinished( mwc713process, 5000, "mwc713")) {
                mwc713process->terminate();
                util::processWaitForFinished( mwc713process, 5000, "mwc713");
            }
            qDebug() << "mwc713 is exited";
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

    QCoreApplication::processEvents();
}

// Check signal: onLoginResult(bool ok)
void MWC713::loginWithPassword(QString password)  {
    qDebug() << "MWC713::loginWithPassword call";
    walletPassword = password;
    eventCollector->addTask( new TaskUnlock(this, password), TaskUnlock::TIMEOUT );
}

// Return true if wallet has password. Wallet might not have password if it was created manually.
bool MWC713::hasPassword() const {
    return !walletPassword.isEmpty();
}


// Exit from the wallet. Expected that state machine will switch to Init state
// syncCall - stop NOW. Caller suppose to understand what he is doing
void MWC713::logout(bool syncCall)  {
    qDebug() << "MWC713::logout syncCall=" << syncCall;

    logger::logInfo("MWC713", QString("mwc713 process exiting with logout. syncCall=") + (syncCall?"Yes":"No") );

    logger::logEmit("MWC713", "onLogout", "" );
    emit onLogout();

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

    Mwc713Task * task = new TaskRecoverShowMnenonic(this, walletPassword );
    if ( eventCollector->hasTask(task) ) {
        delete task;
        return;
    }

    QPair<bool,bool> lsnState = getListenerStartState();

    if (lsnState.first)
        listeningStop(true, false);

    if (lsnState.second)
        listeningStop(false, true);

    eventCollector->addTask(task , TaskRecoverShowMnenonic::TIMEOUT );

    if (lsnState.first)
        listeningStart(true, false, true);

    if (lsnState.second)
        listeningStart(false, true, true);
}

QString MWC713::getPassword() {
    return walletPassword;
}

// Checking if wallet is listening through services
// return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
QPair<bool,bool> MWC713::getListenerStatus()  {
    return QPair<bool,bool>(mwcMqOnline, keybaseOnline);
}

QPair<bool,bool> MWC713::getListenerStartState()  {
    return QPair<bool,bool>(mwcMqStarted, keybaseStarted);
}

// Check Signal: onListeningStartResults
void MWC713::listeningStart(bool startMq, bool startKb, bool initialStart)  {
    qDebug() << "listeningStart: mq=" << startMq << ",kb=" << startKb;
    eventCollector->addTask( new TaskListeningStart(this, startMq,startKb, initialStart), TaskListeningStart::TIMEOUT );

    if (startMq)
        mwcMqStartRequested = true;

    if ( startMq && !config::getUseMwcMqS() ) {
        appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING, "You are using non secure version of the MWC MQ. To switch to secure MWC MQS please specify 'useMwcMqS = true' at mwc-qt-wallet config file." );
    }
}

// Check signal: onListeningStopResult
void MWC713::listeningStop(bool stopMq, bool stopKb)  {
    qDebug() << "listeningStop: mq=" << stopMq << ",kb=" << stopKb;

    if (stopMq)
        mwcMqStartRequested = false;

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

// Request http(s) listening status.
// bool - true is listening. Then next will be the address
// bool - false, not listening. Then next will be error or empty if listening is not active.
// Check signal: onHttpListeningStatus(bool listening, QString additionalInfo)
QPair<bool, QString> MWC713::getHttpListeningStatus() const {
    return QPair<bool, QString>(httpOnline, httpInfo);
}

// Return true if Tls is setted up for the wallet for http connections.
bool MWC713::hasTls() const {
    return hasHttpTls;
}

QVector<AccountInfo>  MWC713::getWalletBalance(bool filterDeleted) const  {
    QVector<AccountInfo> accountInfo = applyOutputLocksToBalance();
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

void MWC713::sync(bool showSyncProgress, bool enforce)  {
    if (enforce || QDateTime::currentMSecsSinceEpoch() - lastSyncTime > 30*1000 ) // sync interval 30 seconds - half of block mining interval
    {
        Mwc713Task * task = new TaskSync(this, showSyncProgress);
        if ( eventCollector->hasTask(task) ) {
            delete task;
            return;
        }
        eventCollector->addTask( task, TaskSync::TIMEOUT );
    }
}

// Request Wallet balance update. It is a multistep operation
// Check signal: onWalletBalanceUpdated
//          onWalletBalanceProgress
void MWC713::updateWalletBalance(bool enforceSync, bool showSyncProgress, bool skipSync)  {
    if ( !isWalletRunningAndLoggedIn() )
        return; // ignoring request

    // Check if already running
    Mwc713Task * task = new TaskAccountList(this);
    if ( eventCollector->hasTask(task) ) {
        delete task;
        return;
    }


    // Steps:
    // 1 - list accounts (this call)
    // 2 - for every account get info ( see updateAccountList call )
    // 3 - restore back current account

    if (!hasPassword()) {
        // By some reasons wallet without password can be locked by itself
        eventCollector->addTask( new TaskUnlock(this, ""), TaskUnlock::TIMEOUT );
    }

    if (!skipSync)
        sync(showSyncProgress, enforceSync);

    eventCollector->addTask( task, TaskAccountList::TIMEOUT );
}

// Create another account, note no delete exist for accounts
// Check Signal:  onAccountCreated
void MWC713::createAccount( const QString & accountName )  {

    // First try to rename one of deleted accounts.
    int delAccIdx = -1;

    for (int t=0; t<accountInfoNoLocks.size(); t++) {
        if ( accountInfoNoLocks[t].isDeleted() ) {
            delAccIdx = t;
            break;
        }
    }

    if (delAccIdx<0) {
        eventCollector->addTask( new TaskAccountCreate(this, accountName), TaskAccountCreate::TIMEOUT );
    }
    else {
        eventCollector->addTask( new TaskAccountRename(this, accountInfoNoLocks[delAccIdx].accountName, accountName, true ), TaskAccountRename::TIMEOUT );
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
    Mwc713Task * task = new TaskCheck(this,wait4listeners);
    if ( eventCollector->hasTask(task) ) {
        delete task;
        return;
    }

    eventCollector->addTask( task, TaskCheck::TIMEOUT );
}


// Send some coins to address.
// Before send, wallet always do the switch to account to make it active
// Check signal:  onSend
void MWC713::sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address,
                     const QString & apiSecret,
                     QString message, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs )  {
    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account.accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    // If listening, strting...

    eventCollector->addTask( new TaskSendMwc(this, coinNano, address, apiSecret, message, inputConfirmationNumber, changeOutputs, outputs), TaskSendMwc::TIMEOUT );
    // Set some funds
}


// Init send transaction with file output
// Check signal:  onSendFile
void MWC713::sendFile( const wallet::AccountInfo &account, int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs )  {

    if ( ! util::validateMwc713Str(fileTx, false).first ) {
        setSendFileResult( false, QStringList{"Unable to create file with name '"+fileTx+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only."} , fileTx );
        return;
    }

    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account.accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );

    eventCollector->addTask( new TaskSendFile(this, coinNano, message, fileTx, inputConfirmationNumber, changeOutputs, outputs ), TaskSendFile::TIMEOUT );
}

// Receive transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveFile
void MWC713::receiveFile( QString fileTx, QString identifier)  {
    if ( ! util::validateMwc713Str(fileTx, false).first ) {
        setReceiveFile( false, QStringList{"Unable to process file with name '"+fileTx+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only."}, fileTx, "" );
        return;
    }

    eventCollector->addTask( new TaskReceiveFile(this, fileTx, identifier), TaskReceiveFile::TIMEOUT );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeFile
void MWC713::finalizeFile( QString fileTxResponse )  {
    if ( ! util::validateMwc713Str(fileTxResponse, false).first ) {
        setFinalizeFile( false, QStringList{"Unable to process file with name '"+fileTxResponse+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only."}, fileTxResponse );
        return;
    }

    eventCollector->addTask( new TaskFinalizeFile(this, fileTxResponse), TaskFinalizeFile::TIMEOUT );
}

// submit finalized transaction. Make sense for cold storage => online node operation
// Check Signal: onSubmitFile(bool ok, String message)
void MWC713::submitFile( QString fileTx ) {
    eventCollector->addTask( new TaskSubmitFile(this, fileTx), TaskSubmitFile::TIMEOUT );
}

// Show outputs for the wallet
// Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> Transactions)
void MWC713::getOutputs(QString account, bool show_spent, bool enforceSync)  {
    sync(true, enforceSync);
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskOutputs(this, show_spent), TaskOutputs::TIMEOUT );
}

void MWC713::getTransactions(QString account, bool enforceSync)  {
    sync(true, enforceSync);
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactions(this), TaskTransactions::TIMEOUT );
}

// get Extended info for specific transaction
// Check Signal: onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages )
void MWC713::getTransactionById(QString account, int64_t txIdx ) {
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactionsById(this, txIdx), TaskTransactions::TIMEOUT );
}

// Read all transactions for all accounts. Might take time...
// Check Signal: onAllTransactions( QVector<WalletTransaction> Transactions)
// Schedule bunch of requests.
void MWC713::getAllTransactions() {
    // Requesting transactions for all accounts...
    Mwc713Task * task = new TaskAllTransactionsEnd(this);
    if ( eventCollector->hasTask(task) ) {
        delete task;
        return;
    }

    // By first task only checking if it is exist
    eventCollector->addTask( new TaskAllTransactionsStart(this), -1);

    // I f not exist, push the rest with enforcement...
    for (const AccountInfo & acc : accountInfoNoLocks ) {
            eventCollector->addTask(new TaskAccountSwitch(this, acc.accountName, walletPassword, false), TaskAccountSwitch::TIMEOUT);
            eventCollector->addTask(new TaskAllTransactions(this), TaskAllTransactions::TIMEOUT);
    }
    eventCollector->addTask( task, -1 );
}

// Get root public key with signed message. Message is optional, can be empty
// Check Signal: onRootPublicKey( QString rootPubKey, QString message, QString signature )
void MWC713::getRootPublicKey( QString message2sign ) {
    eventCollector->addTask( new TaskRootPublicKey(this, message2sign ), TaskRootPublicKey::TIMEOUT );
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
    if ( ! util::validateMwc713Str(resultingFileName, false).first ) {
        setExportProofResults( false, resultingFileName, "Unable to store file with name '"+resultingFileName+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only." );
        return;
    }

    eventCollector->addTask( new TaskTransExportProof(this, resultingFileName, transactionId), TaskTransExportProof::TIMEOUT );
}

// Verify the proof for transaction
// Check Signal: onVerifyProof( bool success, QString msg );
void MWC713::verifyMwcBoxTransactionProof( QString proofFileName )  {
    if ( ! util::validateMwc713Str(proofFileName, false).first ) {
        setVerifyProofResults( false, proofFileName, "Unable to process '"+proofFileName+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only." );
        return;
    }

    eventCollector->addTask( new TaskTransVerifyProof(this, proofFileName), TaskTransExportProof::TIMEOUT );
}

// Status of the node
// Check Signal: onNodeSatatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections )
bool MWC713::getNodeStatus() {
    if ( !isWalletRunningAndLoggedIn() )
        return false; // ignoring request

    Mwc713Task * task = new TaskNodeInfo(this);
    if ( eventCollector->hasTask(task) ) {
        delete task;
        return true;
    }

    eventCollector->addTask( task, TaskNodeInfo::TIMEOUT );
    return true;
}

// Airdrop special. Generating the next Pablic key for transaction
// wallet713> getnextkey --amount 1000000
// "Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)"
// Check Signal: onGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor);
void MWC713::getNextKey( int64_t amountNano, QString btcaddress, QString airDropAccPassword ) {
    eventCollector->addTask( new TaskGetNextKey(this,amountNano, btcaddress, airDropAccPassword ), TaskGetNextKey::TIMEOUT );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Feed the command to mwc713 process
void MWC713::executeMwc713command(QString cmd, QString shadowStr) {
    if (mwc713process==nullptr)
        return;

    Q_ASSERT(mwc713process);
    if (shadowStr.size()==0)
        logger::logMwc713in(cmd);
    else
        logger::logMwc713in( "CENSORED: " + shadowStr );

    mwc713process->write( (cmd + "\n").toLocal8Bit() );
}



void MWC713::setLoginResult(bool ok) {
    logger::logEmit("MWC713", "onLoginResult", QString::number(ok) );
    loggedIn = ok;
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
                               QStringList errorMessages, bool initialStart ) {
    logger::logEmit("MWC713", "onListeningStartResults", QString("mqTry=") + QString::number(mqTry) +
                                                         " kbTry=" + QString::number(kbTry) + " errorMessages size " + QString::number(errorMessages.size()) + " initStart=" + (initialStart?"True":"False") );

    if (mqTry)
        mwcMqStarted = true;
    if (kbTry)
        keybaseStarted = true;

    emit onListeningStartResults(mqTry, kbTry,errorMessages, initialStart );
}

void MWC713::setListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                            QStringList errorMessages ) {
    logger::logEmit("MWC713", "onListeningStopResult", QString("mqTry=") + QString::number(mqTry) +
             " kbTry=" + QString::number(kbTry) + " errorMessages size " + QString::number(errorMessages.size()) );

    if (mqTry)
        mwcMqStarted = false;
    if (kbTry)
        keybaseStarted = false;

    emit onListeningStopResult(mqTry, kbTry,errorMessages);

    if (mqTry) {
        setMwcMqListeningStatus(false, activeMwcMqsTid, true );
    }

    if (kbTry) {
        setKeybaseListeningStatus(false);
    }


}

void MWC713::setMwcMqListeningStatus(bool online, QString tid, bool startStopEvents) {

    qDebug() <<  "Call setMwcMqListeningStatus: online=" << online << " tid=" << tid << " startStopEvents=" << startStopEvents <<
                 "  activeMwcMqsTid=" << activeMwcMqsTid << "  mwcMqOnline="<<mwcMqOnline;

    if (tid!=activeMwcMqsTid) {
        if (! (startStopEvents && online) )
            return; // ignoring unknown/retired thread events
    }

    if (startStopEvents)
        activeMwcMqsTid = tid;

    if (mwcMqOnline != online) {
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, (online ? "Start " : "Stop ") + QString("listening on mwc mq") + (config::getUseMwcMqS()?"s":"") );
    }
    mwcMqOnline = online;
    logger::logEmit("MWC713", "onMwcMqListenerStatus", QString("online=") + QString::number(online));
    emit onMwcMqListenerStatus(online);

}

void MWC713::setKeybaseListeningStatus(bool online) {
    if (keybaseOnline != online) {
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, (online ? "Start " : "Stop ") + QString("listening on keybase"));
    }
    keybaseOnline = online;
    logger::logEmit("MWC713", "onKeybaseListenerStatus", QString("online=") + QString::number(online));
    emit onKeybaseListenerStatus(online);
}

// info: if online  - Address, offlone - Error message or empty.
void MWC713::setHttpListeningStatus(bool online, QString info) {
    if (online==httpOnline && info==httpInfo)
        return;

    httpOnline = online;
    httpInfo = info;

    logger::logEmit("MWC713", "onHttpListeningStatus", QString("online=") + QString::number(online) + " info="+info);
    emit onHttpListeningStatus(online, info);
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
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("MWC wallet was successfully recovered from the mnemonic"));
    }
    else {
        appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING, QString("Failed to recover from the mnemonic"));
    }
}

void MWC713::setRecoveryProgress( int64_t progress, int64_t limit ) {
    logger::logEmit("MWC713", "onRecoverProgress", QString("progress=") + QString::number(progress) +
                                              " limit=" + QString::number(limit) );
    emit onRecoverProgress( int(progress), int(limit) );
}

// Apply account list. Exploring what does wallet has
void MWC713::updateAccountList( QVector<QString> accounts ) {
    collectedAccountInfo.clear();

    core::SendCoinsParams params = appContext->getSendCoinsParams();

    QVector<AccountInfo> accountInfo;

    int idx = 0;
    for (QString acc : accounts) {
        eventCollector->addTask( new TaskAccountSwitch(this, acc, walletPassword, false), TaskAccountSwitch::TIMEOUT );
        eventCollector->addTask( new TaskAccountInfo(this, params.inputConfirmationNumber ), TaskAccountInfo::TIMEOUT );
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
    accountInfoNoLocks = collectedAccountInfo;
    collectedAccountInfo.clear();

    Q_ASSERT(hodlStatus);
    hodlStatus->finishWalletOutputs();

    QString accountBalanceStr;

    for (const auto & acc: accountInfoNoLocks) {
        accountBalanceStr += acc.toString() + ";";
    }

    logger::logEmit( "MWC713", "onWalletBalanceUpdated", "origin from updateAccountFinalize, " + accountBalanceStr );
    emit onWalletBalanceUpdated();

    // Set back the current account
    bool isDefaultFound = false;
    for (auto & acc : accountInfoNoLocks) {
        if (acc.accountName == prevCurrentAccount) {
            isDefaultFound = true;
            break;
        }
    }

    if (!isDefaultFound && accountInfoNoLocks.size()>0) {
        prevCurrentAccount = accountInfoNoLocks[0].accountName;
    }

    // !!!!!! NOTE, 'false' mean that we don't save to that account. It make sence because during such long operation
    //  somebody could change account
    eventCollector->addFirstTask( new TaskAccountSwitch(this, prevCurrentAccount, walletPassword, false), TaskAccountSwitch::TIMEOUT );
}

void MWC713::createNewAccount( QString newAccountName ) {
    // Add new account info into the list. New account is allways empty
    AccountInfo acc;
    acc.setData(newAccountName,0,0,0,0,0,false);
    accountInfoNoLocks.push_back( acc );

    logger::logEmit( "MWC713", "onAccountCreated",newAccountName);
    logger::logEmit( "MWC713", "onWalletBalanceUpdated","");

    emit onAccountCreated(newAccountName);
    emit onWalletBalanceUpdated();

    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("New account '" + newAccountName + "' was created" ));
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
    for (auto & ai : accountInfoNoLocks) {
        if (ai.accountName == oldName) {
            ai.accountName = newName;
            walletOutputs.insert(newName, walletOutputs.value(oldName));
        }
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
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("Account '" + oldName + "' was renamed to '" + newName + "'" ));
    else
        appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING, QString("Failed to rename account '" + oldName + "'" ));

    logger::logEmit( "MWC713", "onWalletBalanceUpdated","");
    emit onWalletBalanceUpdated();

}

// Update with account info
void MWC713::infoResults( QString currentAccountName, int64_t height,
                  int64_t totalConfirmedNano, int64_t waitingConfNano, int64_t waitingFinalizetinNano, int64_t lockedNano, int64_t spendableNano, bool mwcServerBroken ) {
    AccountInfo acc;
    acc.setData(currentAccountName,
                totalConfirmedNano + waitingFinalizetinNano,   // we need "Total", not "Confirmed Total"
                waitingConfNano + waitingFinalizetinNano,
                lockedNano,
                spendableNano,
                height,
                mwcServerBroken);

    int accIdx = 0;
    for ( ;accIdx<collectedAccountInfo.size(); accIdx++) {
        if (collectedAccountInfo[accIdx].accountName == currentAccountName)
            break;
    }

    if (accIdx<collectedAccountInfo.size()) {
        collectedAccountInfo[accIdx] = acc;
    }
    else {
        collectedAccountInfo.push_back(acc);
    }
}

void MWC713::setSendResults(bool success, QStringList errors, QString address, int64_t txid, QString slate) {
    logger::logEmit( "MWC713", "onSend", "success=" + QString::number(success) );
    emit onSend( success, errors, address, txid, slate );
}

void MWC713::reportSlateSendTo( QString slate, QString mwc, QString sendAddr ) {
    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("You successfully sent slate " + slate +
          " with " + mwc + " mwc to " + sendAddr ));

    logger::logEmit( "MWC713", "onSlateSend", slate + " with " +mwc + " to " + sendAddr );
    emit onSlateSendTo(slate, mwc, sendAddr);
}

void MWC713::reportSlateSendBack( QString slate,  QString sendAddr ) {
    logger::logEmit("MWC713", "onSlateSendBack", slate + " to " + sendAddr);

    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO,
                                         "Slate " + slate + " sent back to " + sendAddr + " sucessfully" );
}

void MWC713::reportSlateReceivedBack( QString slate, QString mwc, QString fromAddr ) {
    logger::logEmit( "MWC713", "reportSlateReceivedBack", slate + " with " +mwc + " from " + fromAddr );

    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO,
                                         "Slate " + slate + " received back from " + fromAddr + " for " + mwc + " mwc");

    emit onSlateReceivedBack(slate, mwc, fromAddr);

    // Request balace refresh
    updateWalletBalance(false,true);
}

void MWC713::reportSlateReceivedFrom( QString slate, QString mwc, QString fromAddr, QString message ) {
    QString msg = "Congratulations! You received " +mwc+ " mwc from " + fromAddr;
    if (!message.isEmpty()) {
        msg += " with message " + message + ".";
    }
    msg +=  " Slate:" + slate;
    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, msg );

    emit onSlateReceivedFrom(slate, mwc, fromAddr, message );

    updateWalletBalance(false,true);

    // Show message box with congrats. Message bot should work from any point. No needs to block locking or what ever we have
    control::MessageBox::messageHTML(nullptr, "Congratulations!",
           "You received <b>" + mwc + "</b> mwc<br>" +
           (message.isEmpty() ? "" : "Description: " + message + "<br>" ) +
           "<br>From: " + fromAddr +
           "<br>Slate: " + slate);

}

void MWC713::reportSlateFinalized( QString slate ) {

    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("Slate finalized : "+slate ));

    logger::logEmit( "MWC713", "onSlateFinalized", slate );
    emit onSlateFinalized(slate);

    // Request balance refresh
    updateWalletBalance(false,true);
}

void MWC713::setSendFileResult( bool success, QStringList errors, QString fileName ) {

    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("File transaction was initiated for "+ fileName ));

    logger::logEmit( "MWC713", "onSendFile", "success="+QString::number(success) );
    emit onSendFile(success, errors, fileName);
}

void MWC713::setReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn ) {
    if (success) {
        appendNotificationMessage(notify::MESSAGE_LEVEL::INFO, QString("File receive transaction was processed for " + inFileName));
    }

    logger::logEmit( "MWC713", "onReceiveFile", "success="+QString::number(success) );
    emit onReceiveFile( success, errors, inFileName, outFn );
}

void MWC713::setFinalizeFile( bool success, QStringList errors, QString fileName ) {
    if (success) {
        appendNotificationMessage(notify::MESSAGE_LEVEL::INFO, QString("File finalized for " + fileName));
    }

    logger::logEmit( "MWC713", "onFinalizeFile", "success="+QString::number(success) );
    emit onFinalizeFile( success, errors, fileName);
}

void MWC713::setSubmitFile(bool success, QString message, QString fileName) {
    if (success) {
        appendNotificationMessage(notify::MESSAGE_LEVEL::INFO, QString("Published transaction for " + fileName));
    }

    logger::logEmit( "MWC713", "setSubmitFile", "success="+QString::number(success) );
    emit onSubmitFile(success, message, fileName);
}

void MWC713::setTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions ) {
    logger::logEmit( "MWC713", "onTransactions", "account="+account );
    emit onTransactions( account, height, Transactions );
}

void MWC713::setTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages ) {
    logger::logEmit( "MWC713", "onTransactionById", QString("success=")+(success?"true":"false") + " transaction=" + transaction.toStringShort() + " outputs: " + QString::number(outputs.size()) + " messages: " + QString::number(messages.size()) );
    emit onTransactionById( success, account, height, transaction, outputs, messages );
}


void MWC713::setOutputs( QString account, int64_t height, QVector<WalletOutput> outputs) {
    setWalletOutputs( account, outputs);
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
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("Set receive account: '" + accountOrMessage + "'" ));

    logger::logEmit( "MWC713", "onSetReceiveAccount", "ok="+QString::number(ok) );
    emit onSetReceiveAccount(ok, accountOrMessage );
}

void MWC713::setCheckResult(bool ok, QString errors) {

    if (ok)
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Account re-sync was finished successfully.");
    else
        appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING, "Account re-sync was failed.");

    logger::logEmit( "MWC713", "onCheckResult", "ok="+QString::number(ok) );
    emit onCheckResult(ok, errors );
}

void MWC713::setNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    logger::logEmit( "MWC713", "setNodeStatus", "online="+QString::number(online) + " NodeHeight="+QString::number(nodeHeight) + " PeerHeight="+QString::number(peerHeight) +
                          " totalDifficulty=" + QString::number(totalDifficulty) + " connections=" + QString::number(connections) );
    emit onNodeStatus( online, errMsg, nodeHeight, peerHeight, totalDifficulty, connections );
}

void MWC713::setRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature ) {
    logger::logEmit( "MWC713", "onRootPublicKey", "rootPubKey=XXXXX message=" + message + " signature=" + signature );
    emit onRootPublicKey( success, errMsg, rootPubKey, message, signature );
}


void MWC713::notifyListenerMqCollision() {
    if (!mwcMqOnline && !mwcMqStarted)
        return;  // False alarm. Can happen with network problems. mwc MQS was already stopped, now we are restarting.

    logger::logEmit("MWC713", "onListenerMqCollision", "");

    mwcMqStarted = false;
    mwcMqStartRequested = false;
    emit onListenerMqCollision();

    if (mwcMqOnline) {
        mwcMqOnline = false;
        emit onMwcMqListenerStatus(false);
    }
}

void MWC713::notifyMqFailedToStart() {
    logger::logInfo("MWC713", "notifyMqFailedToStart processed");
    if (mwcMqStarted) {
        mwcMqStarted = false;
        emit onListeningStopResult(true, false, {} );
    }

    if (mwcMqStartRequested ) {
        // schedule the restart in 5 seconds
        QTimer::singleShot( 1000*5, this, &MWC713::restartMQsListener );
    }
}

void  MWC713::restartMQsListener() {
    if (mwcMqStartRequested && !mwcMqStarted) {
        qDebug() << "Try to restart MQs Listener after failure";
        eventCollector->addTask( new TaskListeningStart(this, true,false, false), TaskListeningStart::TIMEOUT );

    }
}


void MWC713::processAllTransactionsStart() {
    collectedTransactions.clear();
}

void MWC713::processAllTransactionsAppend(const QVector<WalletTransaction> & trVector) {
    collectedTransactions.append(trVector);
}

void MWC713::processAllTransactionsEnd() {
    emit onAllTransactions(collectedTransactions);
    collectedTransactions.clear();
}

void MWC713::updateSyncProgress(double progressPercent) {
    logger::logEmit("MWC713", "onUpdateSyncProgress", QString::number(progressPercent) );

    emit onUpdateSyncProgress(progressPercent);
}

void MWC713::updateSyncAsDone() {
    lastSyncTime = QDateTime::currentMSecsSinceEpoch();
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
    logger::logInfo("MWC713", "Unable to start mwc713 process. ProcessError=" + QString::number(error) );
    qDebug() << "Unable to start mwc713 process. ProcessError=" << error;

    if (mwc713process) {
        QString stdoutStr = mwc713process->readAllStandardOutput();
        logger::logInfo("MWC713", "stdout: " + stdoutStr );
        QString stderrStr = mwc713process->readAllStandardError();
        logger::logInfo("MWC713", "stderr: " + stderrStr );

        util::updateEventList(outputsLines, stdoutStr);
        util::updateEventList(outputsLines, stderrStr);

        mwc713process->deleteLater();
        mwc713process = nullptr;
    }

    appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR,
                     "mwc713 process exited. Process error: "+ QString::number(error) +
                     + "\n\nCommand line:\n\n" + commandLine);

}

void MWC713::mwc713finished(int exitCode, QProcess::ExitStatus exitStatus) {
    logger::logInfo("MWC713", "mwc713 exited with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );

    qDebug() << "mwc713 is exiting with exit code " << exitCode << ", exitStatus=" << exitStatus;

    if (mwc713process) {
        QString stdoutStr = mwc713process->readAllStandardOutput();
        logger::logInfo("MWC713", "stdout: " + stdoutStr );
        QString stderrStr = mwc713process->readAllStandardError();
        logger::logInfo("MWC713", "stderr: " + stderrStr );

        util::updateEventList(outputsLines, stdoutStr);
        util::updateEventList(outputsLines, stderrStr);

        mwc713process->deleteLater();
        mwc713process = nullptr;
    }

    // Check if foreign API is enabled. That might be a problem for start. Example, pem Keys
    WalletConfig config = getWalletConfig();
    QString errorMessage = "mwc713 process exited due some unexpected error.\nmwc713 exit code: " + QString::number(exitCode);

    if (config.hasForeignApi()) {
        errorMessage += "\n\nYou have activated foreign API and it might be a reason for this issue. Foreign API is deactivated, please try to restart the wallet";
        config.foreignApi = false;
        saveWalletConfig(config, nullptr, nullptr );
    }
    else {
        if (QDateTime::currentMSecsSinceEpoch() - walletStartTime < 1000L * 15) {
            // Very likely that wallet wasn't be able to start. Lets update the message with mode details

            QString walletErrMsg;
            if (outputsLines.size()>0) {
                // Check if there are erorrs or warnings...
                QList<QString> filteredOutput;
                for (const auto & ln : outputsLines) {
                    if (ln.contains("Error", Qt::CaseInsensitive) || ln.contains("Warning", Qt::CaseInsensitive))
                        filteredOutput.push_back(ln);
                }
                if ( filteredOutput.isEmpty() )
                    filteredOutput = outputsLines;

                Q_ASSERT(filteredOutput.size()>0);
                for (const auto & ln : filteredOutput)
                    walletErrMsg += ln + "\n";
            }
            else {
                walletErrMsg = "Please check if you have enough space at your home disk or there are any antivirus preventing mwc713 to start.\n";
            }

            errorMessage +=
                    "\n\n" + walletErrMsg + "\n" +
                    "You might use command line for troubleshooting:\n\n" + commandLine;
        }
    }

    appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR,
                               errorMessage );
}

void MWC713::mwc713readyReadStandardError() {
    qDebug() << "get mwc713readyReadStandardError call !!!";
    Q_ASSERT(mwc713process);
    if (mwc713process) {
        QString str( ioutils::FilterEscSymbols( mwc713process->readAllStandardError() ) );

        appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL,
                                   "mwc713 process report error:\n" + str );
    }
}

void MWC713::mwc713readyReadStandardOutput() {
    if (mwc713process==nullptr)
        return;

    QString str( ioutils::FilterEscSymbols( mwc713process->readAllStandardOutput() ) );
    qDebug() << "Get output:" << str;
    logger::logMwc713out(str);

    // Let's filter out the possible prompt from the editor it can be located anywhere
    // To filter out:  'wallet713>'
    auto lns = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

    QString filteredStr;
    for (auto ln : lns) {
        if (ln.startsWith("wallet713>"))
            ln = ln.mid(strlen("wallet713>")).trimmed();

        if (!filteredStr.isEmpty())
            filteredStr += "\n";
        filteredStr += ln;

        if (!ln.isEmpty()) {
            while(outputsLines.size()>outputsLinesBufferSize)
                outputsLines.pop_front();

            outputsLines.push_back(ln);
        }
    }

    if (str.size()>0 && (str[0] == '\n' || str[0] == '\r') )
        filteredStr = "\n" + filteredStr;

    if ( str.size()>0 && (str[str.size()-1] == '\n' || str[str.size()-1] == '\r') ) {
        filteredStr += "\n";
    }

    inputParser->processInput(filteredStr);
}

/////////////////////////////////////////////////////////////////////////
// Read config from the file
// static
WalletConfig MWC713::readWalletConfig(QString source) {
    if (source.isEmpty())
        source = config::getMwc713conf();

    util::ConfigReader  mwc713config;

    if (!mwc713config.readConfig(source) ) {
        control::MessageBox::messageText(nullptr, "Read failure", "Unable to read mwc713 configuration from " + source );
        return WalletConfig();
    }

    QString network = mwc713config.getString("chain");
    QString dataPath = mwc713config.getString("wallet713_data_path");
    QString keyBasePath = mwc713config.getString("keybase_binary");
    QString mwcmqDomain  = mwc713config.getString("mwcmq_domain");
    QString mwcmqsDomain = mwc713config.getString("mwcmqs_domain");

    bool foreignApi = mwc713config.getString("foreign_api") == "true";
    QString foreignApiAddress = mwc713config.getString("foreign_api_address");
    QString foreignApiSecret = mwc713config.getString("foreign_api_secret");
    QString tlsCertificateFile = mwc713config.getString("tls_certificate_file");
    QString tlsCertificateKey = mwc713config.getString("tls_certificate_key");

    if (foreignApiAddress.isEmpty())
        foreignApi = false;

    if (dataPath.isEmpty() ) {
        control::MessageBox::messageText(nullptr, "Read failure", "Not able to find all expected mwc713 configuration values at " + source );
        return WalletConfig();
    }

    QString nodeURI     = mwc713config.getString("mwc_node_uri");
    QString nodeSecret  = mwc713config.getString("mwc_node_secret");

    return WalletConfig().setData( network, dataPath, mwcmqDomain, mwcmqsDomain, keyBasePath,
            foreignApi, foreignApiAddress, foreignApiSecret, tlsCertificateFile, tlsCertificateKey );
}


// Get current configuration of the wallet. will read from wallet713.toml file
WalletConfig MWC713::getWalletConfig()  {
    return readWalletConfig();
}

// Get configuration form the resource file.
WalletConfig MWC713::getDefaultConfig()  {
    return readWalletConfig( mwc::MWC713_DEFAULT_CONFIG );
}


//static
bool MWC713::saveWalletConfig(const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode ) {
    if (!config.isDefined()) {
        Q_ASSERT(false);
        logger::logInfo("MWC713", "Failed to update the config, because it is invalid:\n" + config.toString());
        return true;
    }

    QString mwc713confFN = config::getMwc713conf();

    QStringList confLines = util::readTextFile(mwc713confFN, false);
    // Updating the config with new values

    QStringList newConfLines;

    QStringList prefixesToCheck{"wallet713_data_path", "keybase_binary", "mwcmq_domain", "mwcmqs_domain",
                                "chain", "grinbox_listener_auto_start", "keybase_listener_auto_start",
                                "foreign_api", "foreign_api_address", "foreign_api_secret",
                                "tls_certificate_file", "tls_certificate_key"};

    if ((appContext != nullptr)) {
        prefixesToCheck.push_back("mwc_node_uri");
        prefixesToCheck.push_back("mwc_node_secret");
    }

    //bool hasMwcNodeInfo = (appContext != nullptr);

    for (QString &ln : confLines) {

        bool hasPrefix = false;
        for (const auto &prefix : prefixesToCheck) {
            if (ln.startsWith(prefix)) {
                hasPrefix = true;
                break;
            }
        }

        if (!hasPrefix) {
            // keep whatever we have here
            newConfLines.append(ln);
        }
    }

    if (!config::isOnlineNode() && appContext != nullptr) {
        // point of that setting to restore for switch from online node to wallet
        appContext->setWallet713DataPathWithNetwork(config.getDataPath(), config.getNetwork());
    }

    newConfLines.append("chain = \"" + config.getNetwork() + "\"");
    newConfLines.append("wallet713_data_path = \"" + config.getDataPath() + "\"");
    if (config.keyBasePath.length() > 0)
        newConfLines.append("keybase_binary = \"" + config.keyBasePath + "\"");

    if (!config.mwcmqDomainEx.isEmpty())
        newConfLines.append("mwcmq_domain = \"" + config.mwcmqDomainEx + "\"");

    if (!config.mwcmqsDomainEx.isEmpty())
        newConfLines.append("mwcmqs_domain = \"" + config.mwcmqsDomainEx + "\"");

    if (config.hasForeignApi() && !config.foreignApiAddress.isEmpty()) {
        newConfLines.append("foreign_api = true");
        newConfLines.append("foreign_api_address = \"" + config.foreignApiAddress + "\"");

        if (!config.foreignApiSecret.isEmpty())
            newConfLines.append("foreign_api_secret = \"" + config.foreignApiSecret + "\"");

        if (!config.tlsCertificateFile.isEmpty() && !config.tlsCertificateKey.isEmpty()) {
            newConfLines.append("tls_certificate_file = \"" + config.tlsCertificateFile + "\"");
            newConfLines.append("tls_certificate_key = \"" + config.tlsCertificateKey + "\"");
        }
    }

    if ( !config::isOnlineWallet()) {
        newConfLines.append("grinbox_listener_auto_start = false");
        newConfLines.append("keybase_listener_auto_start = false");
    }

    // Update connection node...
    if (appContext != nullptr && mwcNode != nullptr) {
        bool needLocalMwcNode = false;

        wallet::MwcNodeConnection connection = appContext->getNodeConnection(config.getNetwork());
        switch (connection.connectionType) {
            case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD:
                break;
            case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL: {
                node::MwcNodeConfig nodeConfig = node::getCurrentMwcNodeConfig(connection.localNodeDataPath,
                                                                               config.getNetwork());
                newConfLines.append("mwc_node_uri = \"http://127.0.0.1:13413\"");
                newConfLines.append("mwc_node_secret = \"" + nodeConfig.secret + "\"");
                needLocalMwcNode = true;
                break;
            }
            case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CUSTOM:
                newConfLines.append("mwc_node_uri = \"" + connection.mwcNodeURI + "\"");
                newConfLines.append("mwc_node_secret = \"" + connection.mwcNodeSecret + "\"");
                break;
            default:
                Q_ASSERT(false);
        }

        // allways stop because config migth change
        if (mwcNode->isRunning()) {
            mwcNode->stop();
        }

        if (needLocalMwcNode) {
            if (!mwcNode->isRunning()) {
                mwcNode->start(connection.localNodeDataPath, config.getNetwork());
            }
        }
    }

    // Escape back slashes for toml
    for (auto & ln : newConfLines) {
        ln.replace("\\", "\\\\"); // escaping all backslashes
    }

    logger::logInfo("MWC713", "Updating mwc713 config with:\n" + newConfLines.join("\n") );

    return util::writeTextFile( mwc713confFN, newConfLines );
}

// Update wallet config. Will update config and restart the mwc713.
// Note!!! Caller is fully responsible for input validation. Normally mwc713 will sart, but some problems might exist
//          and caller suppose listen for them
bool MWC713::setWalletConfig( const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode ) {

    if ( !saveWalletConfig( config, appContext, mwcNode ) ) {
        control::MessageBox::messageText(nullptr, "Update Config failure", "Not able to update mwc713 configuration at " + config::getMwc713conf() );
        return false;
    }

    emit onConfigUpdate();

    // Stopping the wallet. Start will be done by init state and caller is responsible for that
    processStop(true); // sync if ok for this call
    return true;
}

void MWC713::onOutputLockChanged(QString commit) {
    qDebug() << "MWC713 Get onOutputLockChanged for " << commit;

    logger::logEmit( "MWC713", "onWalletBalanceUpdated", "origin from onOutputLockChanged, commit=" + commit );
    emit onWalletBalanceUpdated();

}

// process accountInfoNoLocks, apply locked outputs
QVector<AccountInfo> MWC713::applyOutputLocksToBalance() const {
    if (!appContext->isLockOutputEnabled())
        return accountInfoNoLocks;

    // Locks are enabled, need to firter all outputs...

    QVector<AccountInfo> accountInfoWithLocks;
    int confNumber = appContext->getSendCoinsParams().inputConfirmationNumber;

    for (AccountInfo ai : accountInfoNoLocks) {

        // Checking Outputs if they locked
        const QVector<wallet::WalletOutput> & accountOutputs = walletOutputs.value(ai.accountName);
        for ( const wallet::WalletOutput & out : accountOutputs ) {
            int64_t dh = ai.height - out.blockHeight.toLongLong();
            if (dh < int64_t(confNumber) )
                continue;

            if (appContext->isLockedOutputs(out.outputCommitment)) {
                ai.lockedByPrevTransaction += out.valueNano;
                ai.currentlySpendable -= out.valueNano;
            }
        }
        accountInfoWithLocks.push_back(ai);
    }
    return accountInfoWithLocks;
}

void MWC713::setWalletOutputs( const QString & account, const QVector<wallet::WalletOutput> & outputs) {
    walletOutputs[account] = outputs;
}


}
