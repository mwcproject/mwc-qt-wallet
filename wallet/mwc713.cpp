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
#include "core/Notification.h"
#include "tasks/TaskStarting.h"
#include "tasks/TaskWallet.h"
#include "tasks/TaskListening.h"
#include "tasks/TaskRecover.h"
#include "tasks/TaskErrWrnInfoListener.h"
#include "tasks/TaskMwcMqAddress.h"
#include "tasks/TaskAccount.h"
#include "tasks/TaskSend.h"
#include "tasks/TaskTransaction.h"
#include "tasks/TaskSwap.h"
#include "../util/Log.h"
#include "../core/appcontext.h"
#include "../util/ConfigReader.h"
#include "../util/Files.h"
#include "../util/Process.h"
#include "../node/MwcNodeConfig.h"
#include "../node/MwcNode.h"
#include <QCoreApplication>
#include "../util/crypto.h"
#include "../core/WndManager.h"

namespace wallet {

// Base class for wallet state, Non managed object. Consumer suppose to delete it
class Mwc713State {
    Mwc713State();
    virtual ~Mwc713State();
};

MWC713::MWC713(QString _mwc713path, QString _mwc713configPath, core::AppContext * _appContext, node::MwcNode * _mwcNode) :
        appContext(_appContext), mwcNode(_mwcNode), mwc713Path(_mwc713path),  mwc713configPath(_mwc713configPath) {

    // Listening for Output Locking changes
    QObject::connect(appContext, &core::AppContext::onOutputLockChanged, this, &MWC713::onOutputLockChanged, Qt::QueuedConnection);

    defaultConfig = readWalletConfig( mwc::MWC713_DEFAULT_CONFIG );
}

MWC713::~MWC713() {
    processStop(startedMode != STARTED_MODE::INIT);
}


// Check if waaled need to be initialized or not. Will run statndalone app, wait for exit and return the result
// Check signal: onWalletState(bool initialized)
bool MWC713::checkWalletInitialized(bool hasSeed) {
    QString path = appContext->getCurrentWalletInstance(hasSeed);
    qDebug() << "checkWalletState with " << mwc713Path << " and " << mwc713configPath << "  Data Path: " << path;
    if (!updateWalletConfig(path, false))
        return false;

    Q_ASSERT(mwc713process==nullptr);
    mwc713process = initMwc713process( {"TOR_EXE_NAME", QCoreApplication::applicationDirPath() + QDir::separator() + TOR_NAME}, {"state"}, false );

    if (mwc713process==nullptr)
        return false;

    if (!util::processWaitForFinished( mwc713process, 3000, "mwc713")) {
        mwc713process->kill();
        util::processWaitForFinished( mwc713process, 3000, "mwc713");
    }

    QString output = mwc713process->readAll();

    processStop(false);

    bool uninit = output.contains("Uninitialized");

    logger::logInfo("MWC713", QString("Output result: ") + output );
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
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

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
                if (core::getWndManager()->questionTextDlg("Warning", QString("Starting for mwc713 process is taking longer than expected.\nContinue to wait?") +
                                  "\n\nCommand line:\n\n" + commandLine,
                                  "Yes", "No",
                                  "Wait more time and let mwc713 to start",
                                  "Stop waiting and kill mwc713 even it can corrupt its data",
                                  true, false) == core::WndManager::RETURN_CODE::BTN1) {
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
    mwcMqOnline = torOnline = false;
    mwcMqStarted = mwcMqStartRequested = torStarted = false;
    torAddress = "";
    mwcAddress = "";
    httpOnline = false;
    httpInfo = "";
    hasHttpTls = false;
    walletPasswordHash = "";
    outputsLines.clear();
    currentAccount = "default";
    recieveAccount = "default";
    currentConfig = WalletConfig();
}

// Updating config according to what is stored at the path
bool MWC713::updateWalletConfig(const QString & path, bool canStartNode) {
    WalletConfig config = getWalletConfig();

    if (config.getDataPath() != path) {
        // Path for the wallet need to be updated
        QVector<QString> network_arch_name = wallet::WalletConfig::readNetworkArchInstanceFromDataPath(path, appContext);
        Q_ASSERT(network_arch_name.size() == 3);

        QString arh = network_arch_name[1];
        if (arh != util::getBuildArch()) {
            core::getWndManager()->messageTextDlg("Error", "Wallet data at directory " + path +
                                                           " was belong to different architecture. Expecting " + util::getBuildArch() + " but get " + arh);
            return false;
        }

        // Config need to be updated with a path and a Network
        config.updateDataPath(path);
        if (!network_arch_name[0].isEmpty() )
            config.updateNetwork(network_arch_name[0]);
    }

    // Need to call setWalletConfig in order to update tor params, embedded node state, e.t.c
    setWalletConfig(config, canStartNode);

    return true;
}

// normal start. will require the password
void MWC713::start()  {
    resetData(STARTED_MODE::NORMAL);

    QString path = appContext->getCurrentWalletInstance(true);
    qDebug() << "MWC713::start for path " << path;
    if (!updateWalletConfig(path, true))
        return;

    // Need to check if Tls active
    const WalletConfig & config = getWalletConfig();
    hasHttpTls = !config.tlsCertificateKey.isEmpty() && !config.tlsCertificateFile.isEmpty();

    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    mwc713process = initMwc713process({"TOR_EXE_NAME", QCoreApplication::applicationDirPath() + QDir::separator() + TOR_NAME}, {} );
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
    eventCollector->addListener( new TaskSwapNewTradeArrive(this) );
}

// start to init. Expected that we will exit pretty quckly
// Check signal: onNewSeed( seed [] )
void MWC713::start2init(QString password) {
    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    QString path = appContext->getCurrentWalletInstance(false);
    if (!updateWalletConfig(path, false))
        return;

    resetData(STARTED_MODE::INIT);

    qDebug() << "Starting MWC713 as init at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting

    mwc713process = initMwc713process({"TOR_EXE_NAME", QCoreApplication::applicationDirPath() + QDir::separator() + TOR_NAME, "MWC_PASSWORD", password}, {"init"} );
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

    walletPasswordHash = crypto::calcHSA256Hash(password);
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

    QString path = appContext->getCurrentWalletInstance(false);
    if (!updateWalletConfig(path, true))
        return;

    qDebug() << "Starting MWC713 as init at " << mwc713Path << " for config " << mwc713configPath;

    QString seedStr;
    for ( auto & s : seed) {
        if (!seedStr.isEmpty())
            seedStr+=" ";
        seedStr+=s;
    }

    // Creating process and starting
    // Mnemonic will moved into variables
    mwc713process = initMwc713process({"TOR_EXE_NAME", QCoreApplication::applicationDirPath() + QDir::separator() + TOR_NAME, "MWC_PASSWORD", password, "MWC_MNEMONIC", seedStr}, {"recover", "--mnemonic", "env" } );
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

    walletPasswordHash = crypto::calcHSA256Hash(password);
}

void MWC713::launchExitCommand() {
    mwc713disconnect();
    executeMwc713command("exit", "");
}

void MWC713::processStop(bool exitNicely) {
    qDebug() << "MWC713::processStop exitNicely=" << exitNicely;

    logger::logInfo("MWC713", QString("mwc713 process exiting ") + (exitNicely? "nicely" : "by killing") );

    if (mwc713process) {
        // Waitiong for task Q to be empty
        // Note, event processing can change a lot for us, so we shoudl watch for variables
        if (exitNicely) {
            qDebug() << "start exiting...";

            int taskTimeout = 0;
            if (eventCollector != nullptr)
                taskTimeout += eventCollector->cancelTasksInQueue();

            // We neevr want to kill the wallet. Even there is a long precess, we want to wait. Other wise we might hit for a data corruption.
            eventCollector->addTask( new TaskExit(this), TaskExit::TIMEOUT );

            if (taskTimeout > 10000) {
                core::getWndManager()->showWalletStoppingMessage(taskTimeout);
            }

            if (!util::processWaitForFinished(mwc713process, 8000 + taskTimeout, "mwc713")) {
               mwc713process->kill();
               util::processWaitForFinished(mwc713process, 8000 + taskTimeout, "mwc713");
            }

            core::getWndManager()->hideWalletStoppingMessage();

            qDebug() << "mwc713 is exited";
        }
        else {
            mwc713disconnect();
            // init state have to be killed. Otherwise it will create a
            // seed without verification. We don't want that
            mwc713process->kill();
        }

        mwc713process->deleteLater();
        mwc713process = nullptr;
    }

    loggedIn = false;

    mwc713disconnect();

    // reset mwc713 interna; state
    //initStatus = InitWalletStatus::NONE;
    mwcAddress = "";
    accountInfoNoLocks.clear();
    walletOutputs.clear();
    currentAccount = "default"; // Keep current account by name. It fit better to mwc713 interactions.
    collectedAccountInfo.clear();

    emit onListenersStatus(false, false);

    mwcMqOnline = false;
    mwcMqStarted = mwcMqStartRequested = false;

    httpOnline = false;
    httpInfo = "";

    emit onMwcAddress("");
    emit onMwcAddressWithIndex("",1);
    emit onTorAddress("");

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
    walletPasswordHash = crypto::calcHSA256Hash(password);
    eventCollector->addTask( new TaskUnlock(this, password), TaskUnlock::TIMEOUT );
}

// Return true if wallet has password. Wallet might not have password if it was created manually.
bool MWC713::hasPassword() const {
    return !walletPasswordHash.isEmpty();
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
void MWC713::getSeed(const QString & walletPassword)  {
    // Need stop listeners first

    Mwc713Task * task = new TaskRecoverShowMnenonic(this, walletPassword );
    if ( eventCollector->hasTask(task) ) {
        delete task;
        return;
    }

    ListenerStatus lsnState = getListenerStartState();
    listeningStop(lsnState.mqs, lsnState.tor);

    eventCollector->addTask(task , TaskRecoverShowMnenonic::TIMEOUT );

    listeningStart(lsnState.mqs, lsnState.tor, true);
}

QString MWC713::getPasswordHash() {
    return walletPasswordHash;
}

// Checking if wallet is listening through services
ListenerStatus MWC713::getListenerStatus()  {
    return ListenerStatus(mwcMqOnline, torOnline);
}

ListenerStatus MWC713::getListenerStartState()  {
    return ListenerStatus(mwcMqStarted, torStarted);
}

// Check Signal: onListeningStartResults
void MWC713::listeningStart(bool startMq, bool startTor, bool initialStart)  {
    qDebug() << "listeningStart: mq=" << startMq << ", tor=" << startTor;

    if (startMq)
        eventCollector->addTask( new TaskListeningStart(this,startMq,false,initialStart), TaskListeningStart::TIMEOUT );
    if (startTor)
        eventCollector->addTask( new TaskListeningStart(this,false, startTor,initialStart), TaskListeningStart::TIMEOUT );

    if (startMq)
        mwcMqStartRequested = true;
}

// Check signal: onListeningStopResult
void MWC713::listeningStop(bool stopMq, bool stopTor)  {
    qDebug() << "listeningStop: mq=" << stopMq << ",stopTor=" << stopTor;

    if (stopMq)
        mwcMqStartRequested = false;

    if (stopMq)
        eventCollector->addTask( new TaskListeningStop(this, stopMq, false), TaskListeningStop::TIMEOUT );
    if (stopTor)
        eventCollector->addTask( new TaskListeningStop(this, false, stopTor), TaskListeningStop::TIMEOUT );
}

// Get latest Mwc MQ address that we see
QString MWC713::getMqsAddress()  {
    return mwcAddress;
}

QString MWC713::getTorAddress()  {
    return torAddress;
}

// Request proof address for files
void MWC713::requestFileProofAddress() {
    eventCollector->addTask( new TaskFileProofAddress(this), TaskFileProofAddress::TIMEOUT );
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
void MWC713::switchAccount(const QString & accountName)  {
    // Expected that account is in the list
    // Allways do switch because double processing is fine, it is quick and it can eliminate possible issues
    currentAccount = accountName;
    const WalletConfig & config = getWalletConfig();

    appContext->setCurrentAccountName( config.getDataPath(), accountName);
    eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}

// Rename account
// Check Signal: onAccountRenamed(bool success, QString errorMessage);
void MWC713::renameAccount(const QString & oldName, const QString & newName)  {
    eventCollector->addTask( new TaskAccountRename(this, oldName, newName, false), TaskAccountRename::TIMEOUT );
    if (oldName == currentAccount) {
        switchAccount("default");
    }
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
void MWC713::sendTo( const QString &account, int64_t coinNano, const QString & address,
                     const QString & apiSecret,
                     QString message, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs,
                     bool fluff, int ttl_blocks, bool generateProof, QString expectedproofAddress )  {
    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskSendMwc(this, coinNano, address, apiSecret, message, inputConfirmationNumber, changeOutputs, outputs, fluff, ttl_blocks, generateProof, expectedproofAddress), TaskSendMwc::TIMEOUT );
    if (account != currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}


// Init send transaction with file output
// Check signal:  onSendFile
void MWC713::sendFile( const QString &account, int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, int ttl_blocks, bool generateProof )  {

    if ( ! util::validateMwc713Str(fileTx, false).first ) {
        setSendFileResult( false, QStringList{"Unable to create file with name '"+fileTx+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only."} , fileTx );
        return;
    }

    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskSendFile(this, coinNano, message, fileTx, inputConfirmationNumber, changeOutputs, outputs, ttl_blocks, generateProof ), TaskSendFile::TIMEOUT );
    if (account != currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
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
void MWC713::finalizeFile( QString fileTxResponse, bool fluff )  {
    if ( ! util::validateMwc713Str(fileTxResponse, false).first ) {
        setFinalizeFile( false, QStringList{"Unable to process file with name '"+fileTxResponse+"' because it has non ASCII (Latin1) symbols. Please use different file path with basic symbols only."}, fileTxResponse );
        return;
    }

    eventCollector->addTask( new TaskFinalizeFile(this, fileTxResponse, fluff), TaskFinalizeFile::TIMEOUT );
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
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskOutputs(this, show_spent), TaskOutputs::TIMEOUT );
    if (account!=currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}

void MWC713::getTransactions(QString account, bool enforceSync)  {
    sync(true, enforceSync);
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactions(this), TaskTransactions::TIMEOUT );
    if (account!=currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}

// get Extended info for specific transaction
// Check Signal: onTransactionById( bool success, QString account, int64_t height, WalletTransaction transaction, QVector<WalletOutput> outputs, QVector<QString> messages )
void MWC713::getTransactionById(QString account, int64_t txIdx ) {
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactionsById(this, txIdx), TaskTransactions::TIMEOUT );
    if (account != currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
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
            eventCollector->addTask(new TaskAccountSwitch(this, acc.accountName), TaskAccountSwitch::TIMEOUT);
            eventCollector->addTask(new TaskAllTransactions(this), TaskAllTransactions::TIMEOUT);
    }
    eventCollector->addTask( task, -1 );
}

// Get root public key with signed message. Message is optional, can be empty
// Check Signal: onRootPublicKey( QString rootPubKey, QString message, QString signature )
void MWC713::getRootPublicKey( QString message2sign ) {
    eventCollector->addTask( new TaskRootPublicKey(this, message2sign ), TaskRootPublicKey::TIMEOUT );
}

void MWC713::repost(QString account, int index, bool fluff) {
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskRepost(this, index, fluff), TaskRepost::TIMEOUT );
    if (account!=currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}

// Request all running swap trades.
// Check Signal: void onRequestSwapTrades(QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error);
void MWC713::requestSwapTrades(QString cookie) {
    eventCollector->addTask( new TaskGetSwapTrades(this, cookie), TaskGetSwapTrades::TIMEOUT );
}

// Delete the swap trade
// Check Signal: void onDeleteSwapTrade(QString swapId, QString errMsg)
void MWC713::deleteSwapTrade(QString swapId) {
    eventCollector->addTask( new TaskDeleteSwapTrade(this, swapId), TaskDeleteSwapTrade::TIMEOUT );
}

// Create a new Swap trade deal.
// Check Signal: void onCreateNewSwapTrade(tag, dryRun, QVector<QString> params, QString swapId, QString err);
void MWC713::createNewSwapTrade(QString account,
                                int min_confirmations, // minimum number of confimations
                                QString mwcAmount, QString secAmount, QString secondary,
                                QString redeemAddress,
                                bool sellerLockFirst,
                                int messageExchangeTimeMinutes,
                                int redeemTimeMinutes,
                                int mwcConfirmationNumber,
                                int secondaryConfirmationNumber,
                                QString communicationMethod,
                                QString communicationAddress,
                                QString electrum_uri1,
                                QString electrum_uri2,
                                bool dryRun,
                                QString tag,
                                QVector<QString> params ) {

    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );

    eventCollector->addTask(new TaskCreateNewSwapTrade(this, min_confirmations, // minimum number of confimations
                                                       mwcAmount, secAmount, secondary.toLower(),
                                                       redeemAddress,
                                                       sellerLockFirst,
                                                       messageExchangeTimeMinutes,
                                                       redeemTimeMinutes,
                                                       mwcConfirmationNumber,
                                                       secondaryConfirmationNumber,
                                                       communicationMethod,
                                                       communicationAddress,
                                                       electrum_uri1,
                                                       electrum_uri2,
                                                       dryRun, tag, params), TaskCreateNewSwapTrade::TIMEOUT);

    if (account!=currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}

// Cancel the trade
// Check Signal: void onCancelSwapTrade(QString swapId, QString error);
void MWC713::cancelSwapTrade(QString swapId) {
    eventCollector->addTask( new TaskCancelSwapTrade(this, swapId), TaskCancelSwapTrade::TIMEOUT );
}

// Request details about this trade.
// Check Signal: void onRequestTradeDetails( SwapTradeInfo swap )
void MWC713::requestTradeDetails(QString swapId) {
    eventCollector->addTask( new TaskTradeDetails(this, swapId), TaskTradeDetails::TIMEOUT );
}

// Adjust swap stade values. params are optional
// Check Signal: onAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg);
void MWC713::adjustSwapData( QString swapId, QString adjustCmd, QString param1, QString param2 ) {
    eventCollector->addTask( new TaskAdjustTrade(this, swapId, adjustCmd, param1, param2), TaskAdjustTrade::TIMEOUT );
}

// Perform a auto swap step for this trade.
// Check Signal: void onPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
//                       QVector<SwapExecutionPlanRecord> executionPlan,
//                       QVector<SwapJournalMessage> tradeJournal,
//                       QString error );
void MWC713::performAutoSwapStep( QString swapId ) {
    eventCollector->addTask( new TaskPerformAutoSwapStep(this, swapId), TaskPerformAutoSwapStep::TIMEOUT );
}

// Backup/export swap trade data file
// Check Signal: onBackupSwapTradeData(QString swapId, QString exportedFileName, QString errorMessage)
void MWC713::backupSwapTradeData(QString swapId, QString backupFileName) {
    eventCollector->addTask( new TaskBackupSwapTradeData(this, swapId, backupFileName), TaskBackupSwapTradeData::TIMEOUT );
}

// Restore/import swap trade from the file
// Check Signal: onRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage);
void MWC713::restoreSwapTradeData(QString filename) {
    eventCollector->addTask( new TaskRestoreSwapTradeData(this, filename), TaskRestoreSwapTradeData::TIMEOUT );
}

void MWC713::requestRecieverWalletAddress(QString url, QString apiSecret) {
    eventCollector->addTask( new TaskRequestRecieverWalletAddress(this, url, apiSecret), TaskRequestRecieverWalletAddress::TIMEOUT );
}

// -------------- Transactions

// Set account that will receive the funds
// Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
void MWC713::setReceiveAccount(QString account)  {
    eventCollector->addTask( new TaskSetReceiveAccount(this, account), TaskSetReceiveAccount::TIMEOUT );
}


// Cancel transaction
// Check Signal:  onCancelTransacton
void MWC713::cancelTransacton(QString account, int64_t txIdx)  {
    eventCollector->addTask( new TaskAccountSwitch(this, account), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransCancel(this, txIdx, account), TaskTransCancel::TIMEOUT );
    if (account!=currentAccount)
        eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
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
    if (ok) {
        // Setting receive acocunt...
        const WalletConfig & config = getWalletConfig();
        switchAccount(appContext->getCurrentAccountName(config.getDataPath()));
        setReceiveAccount( appContext->getReceiveAccount(config.getDataPath()) );
    }
    loggedIn = ok;
    emit onLoginResult(ok);

}

void MWC713::setGetNextKeyResult( bool success, QString identifier, QString publicKey, QString errorMessage, QString btcaddress, QString airDropAccPasswor) {
    logger::logEmit("MWC713", "onGetNextKeyResult", QString::number(success) + " " + identifier + " " + publicKey + " " + errorMessage + " " + btcaddress + " " + airDropAccPasswor );
    emit onGetNextKeyResult(success, identifier, publicKey, errorMessage, btcaddress, airDropAccPasswor);
}

void MWC713::setTorAddress( QString _torAddress) {
    torAddress = _torAddress;
    logger::logEmit("MWC713", "onTorAddress", torAddress );
    emit onTorAddress(torAddress);
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

void MWC713::setFileProofAddress( QString address ) {
    logger::logEmit("MWC713", "onFileProofAddress", address );
    emit onFileProofAddress(address);
}

void MWC713::setNewSeed( QVector<QString> seed ) {
    logger::logEmit("MWC713", "onNewSeed", "????" );
    emit onNewSeed(seed);
}

void MWC713::setGettedSeed( QVector<QString> seed ) {
    logger::logEmit("MWC713", "onGetSeed", "????" );
    emit onGetSeed(seed);
}


void MWC713::setListeningStartResults( bool mqTry, bool torTry, // what we try to start
                               QStringList errorMessages, bool initialStart ) {
    logger::logEmit("MWC713", "onListeningStartResults", QString("mqTry=") + QString::number(mqTry) +
                                                         " torTry=" + QString::number(torTry) + " errorMessages size " + QString::number(errorMessages.size()) + " initStart=" + (initialStart?"True":"False") );

    if (mqTry)
        mwcMqStarted = true;
    if (torTry)
        torStarted = true;

    emit onListeningStartResults(mqTry, torTry, errorMessages, initialStart );
}

void MWC713::setListeningStopResult(bool mqTry, bool torTry, // what we try to stop
                            QStringList errorMessages ) {
    logger::logEmit("MWC713", "onListeningStopResult", QString("mqTry=") + QString::number(mqTry) +
             " torTry=" + QString::number(torTry) + " errorMessages size " + QString::number(errorMessages.size()) );

    if (mqTry)
        mwcMqStarted = false;
    if (torTry) {
        torStarted = false;
        setTorAddress("");
    }

    emit onListeningStopResult(mqTry, torTry, errorMessages);

    if (mqTry) {
        setMwcMqListeningStatus(false, activeMwcMqsTid, true );
    }
    if (torTry) {
        setTorListeningStatus(false);
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
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, (online ? "Start " : "Stop ") + QString("listening on MWC MQS") );
    }
    mwcMqOnline = online;
    logger::logEmit("MWC713", "onListenersStatus", QString(mwcMqOnline ? "true" : "false") + " " + QString(torOnline ? "true" : "false") );
    emit onListenersStatus(mwcMqOnline, torOnline);

}

void MWC713::setTorListeningStatus(bool online) {
    if (torOnline != online) {
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, (online ? "Start " : "Stop ") + QString(" Tor listener"));
    }
    torOnline = online;
    logger::logEmit("MWC713", "onListenersStatus", QString(mwcMqOnline ? "true" : "false") + " " + QString(torOnline ? "true" : "false") );
    emit onListenersStatus(mwcMqOnline, torOnline);
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
        appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, QString("MWC Wallet was successfully recovered from the mnemonic"));
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
    int taskIdx = 0;
    for (QString acc : accounts) {
        eventCollector->addTask( new TaskAccountSwitch(this, acc), TaskAccountSwitch::TIMEOUT, taskIdx++ );
        eventCollector->addTask( new TaskAccountInfo(this, params.inputConfirmationNumber ), TaskAccountInfo::TIMEOUT, taskIdx++ );
        eventCollector->addTask( new TaskAccountProgress(this, idx++, accounts.size() ), -1, taskIdx++ ); // Updating the progress
    }
    eventCollector->addTask( new TaskAccountListFinal(this), -1, taskIdx++ ); // Finalize the task
    // Final will switch back to current account
}

void MWC713::updateAccountProgress(int accountIdx, int totalAccounts) {
    logger::logEmit( "MWC713", "onWalletBalanceProgress", "accountIdx=" + QString::number(accountIdx) + " totalAccounts=" + QString::number(totalAccounts) );
    emit onWalletBalanceProgress( accountIdx, totalAccounts );
}

void MWC713::updateAccountFinalize() {
    accountInfoNoLocks = collectedAccountInfo;
    collectedAccountInfo.clear();

    QString accountBalanceStr;

    for (const auto & acc: accountInfoNoLocks) {
        accountBalanceStr += acc.toString() + ";";
    }

    logger::logEmit( "MWC713", "onWalletBalanceUpdated", "origin from updateAccountFinalize, " + accountBalanceStr );
    emit onWalletBalanceUpdated();

    // Set back the current account
    bool isDefaultFound = false;
    for (auto & acc : accountInfoNoLocks) {
        if (acc.accountName == currentAccount) {
            isDefaultFound = true;
            break;
        }
    }

    if (!isDefaultFound && accountInfoNoLocks.size()>0) {
        currentAccount = accountInfoNoLocks[0].accountName;
    }

    // !!!!!! NOTE, 'false' mean that we don't save to that account. It make sence because during such long operation
    //  somebody could change account
    eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT, 0 );
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

void MWC713::setSendResults(bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc) {
    if (success) {
        appendNotificationMessage(notify::MESSAGE_LEVEL::INFO, QString("You successfully sent slate " + slate +
                                                                       " with " + mwc + " MWC to " + address));
    }

    logger::logEmit( "MWC713", "onSend", "success=" + QString::number(success) );
    emit onSend( success, errors, address, txid, slate, mwc );
    updateWalletBalance(false,true);
}


void MWC713::reportSlateReceivedFrom( QString slate, QString mwc, QString fromAddr, QString message ) {
    QString msg = "Congratulations! You received " +mwc+ " MWC from " + fromAddr;
    if (!message.isEmpty()) {
        msg += " with message " + message + ".";
    }
    msg +=  " Slate:" + slate;
    appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, msg );

    emit onSlateReceivedFrom(slate, mwc, fromAddr, message );

    updateWalletBalance(false,true);

    if (!appContext->getNotificationWindowsEnabled()) {
        // only display the message dialog if notification windows are not enabled
        core::getWndManager()->messageHtmlDlg("Congratulations!",
           "You received <b>" + mwc + "</b> MWC<br>" +
           (message.isEmpty() ? "" : "Description: " + message + "<br>" ) +
           "<br>From: " + fromAddr +
           "<br>Slate: " + slate);
    }
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


void MWC713::setOutputs( QString account, bool show_spent, int64_t height, QVector<WalletOutput> outputs) {
    setWalletOutputs( account, outputs);
    logger::logEmit( "MWC713", "onOutputs", "account="+account );
    emit onOutputs( account, show_spent, height, outputs );
}

void MWC713::setExportProofResults( bool success, QString fn, QString msg ) {
    logger::logEmit( "MWC713", "onExportProof", "success="+QString::number(success) );
    emit onExportProof( success, fn, msg );
}

void MWC713::setVerifyProofResults( bool success, QString fn, QString msg ) {
    logger::logEmit( "MWC713", "onVerifyProof", "success="+QString::number(success) );
    emit onVerifyProof(success, fn, msg);
}

void MWC713::setTransCancelResult( bool success, const QString & account, int64_t transId, QString errMsg ) {
    logger::logEmit( "MWC713", "onCancelTransacton", "success="+QString::number(success) );
    emit onCancelTransacton(success, account, transId, errMsg);
}

void MWC713::setSetReceiveAccount( bool ok, QString accountOrMessage ) {
    if (ok) {
        appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                  QString("Set receive account: '" + accountOrMessage + "'"));
        recieveAccount = accountOrMessage;
        const WalletConfig & config = getWalletConfig();
        appContext->setReceiveAccount(config.getDataPath(), recieveAccount);
    }

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
        logger::logEmit("MWC713", "onListenersStatus", QString(mwcMqOnline ? "true" : "false") + " " + QString(torOnline ? "true" : "false") );
        emit onListenersStatus(mwcMqOnline, torOnline);
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
        eventCollector->addTask( new TaskListeningStart(this,true,false,false), TaskListeningStart::TIMEOUT );

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
    // switching back to current account
    eventCollector->addTask( new TaskAccountSwitch(this, currentAccount), TaskAccountSwitch::TIMEOUT );
}

void MWC713::updateSyncProgress(double progressPercent) {
    logger::logEmit("MWC713", "onUpdateSyncProgress", QString::number(progressPercent) );

    emit onUpdateSyncProgress(progressPercent);
}

void MWC713::updateSyncAsDone() {
    lastSyncTime = QDateTime::currentMSecsSinceEpoch();
}

void MWC713::setRequestSwapTrades( QString cookie, QVector<wallet::SwapInfo> swapTrades, QString error ) {
    logger::logEmit("MWC713", "onRequestSwapTrades", "Cookie:" + cookie + " Trades: " + QString::number(swapTrades.size()) + ", Error: " + error );
    emit onRequestSwapTrades( cookie, swapTrades, error );
}

void MWC713::setDeleteSwapTrade(QString swapId, QString errMsg) {
    logger::logEmit("MWC713", "onDeleteSwapTrade", swapId + ", " + errMsg );
    emit onDeleteSwapTrade( swapId, errMsg );
}

void MWC713::setCreateNewSwapTrade( QString tag, bool dryRun, QVector<QString> params, QString swapId, QString errMsg ) {
    logger::logEmit("MWC713", "onCreateNewSwapTrade", tag + ", " + (dryRun?"dryRun:ON, ":"") + swapId + ", " + errMsg );
    emit onCreateNewSwapTrade( tag, dryRun, params, swapId, errMsg );

}

void MWC713::setCancelSwapTrade(QString swapId, QString errMsg) {
    logger::logEmit("MWC713", "onCancelSwapTrade", swapId + ", " + errMsg );
    emit onCancelSwapTrade(swapId, errMsg );
}

void MWC713::setRequestTradeDetails( SwapTradeInfo swap,
                                     QVector<SwapExecutionPlanRecord> executionPlan,
                                     QString currentAction,
                                     QVector<SwapJournalMessage> tradeJournal,
                                     QString error ) {
    logger::logEmit("MWC713", "onRequestSwapDetails", swap.swapId + ", " + error );
    emit onRequestTradeDetails( swap, executionPlan, currentAction, tradeJournal, error );
}

void MWC713::setAdjustSwapData(QString swapId, QString adjustCmd, QString errMsg) {
    logger::logEmit("MWC713", "onAdjustSwapData", swapId + ", " + adjustCmd + ", " + errMsg );
    emit onAdjustSwapData( swapId, adjustCmd, errMsg );
}

void MWC713::setPerformAutoSwapStep(QString swapId, QString stateCmd, QString currentAction, QString currentState,
                            QVector<SwapExecutionPlanRecord> executionPlan,
                            QVector<SwapJournalMessage> tradeJournal,
                            QString error ) {
    logger::logEmit( "MWC713", "onPerformAutoSwapStep", swapId + ", " + stateCmd + ", " + currentAction + ", " + currentState + ", " + error );

    emit onPerformAutoSwapStep(swapId, stateCmd, currentAction, currentState,
                               executionPlan,
                               tradeJournal,
                               error );
}

// Notificaiton that nee Swap trade offer was recieved.
void MWC713::notifyAboutNewSwapTrade(QString currency, QString swapId) {
    logger::logEmit( "MWC713", "onNewSwapTrade", currency + ", " + swapId );
    emit onNewSwapTrade(currency, swapId);
}

void MWC713::setBackupSwapTradeData(QString swapId, QString backupFileName, QString errorMessage) {
    logger::logEmit( "MWC713", "onBackupSwapTradeData", swapId + ", " + backupFileName + ", " + errorMessage );
    emit onBackupSwapTradeData(swapId, backupFileName, errorMessage);
}

void MWC713::setRestoreSwapTradeData(QString swapId, QString importedFilename, QString errorMessage) {
    logger::logEmit( "MWC713", "onRestoreSwapTradeData", swapId + ", " + importedFilename + ", " + errorMessage );
    emit onRestoreSwapTradeData( swapId, importedFilename, errorMessage );
}

void MWC713::setRequestRecieverWalletAddress(QString url, QString address, QString error) {
    logger::logEmit( "MWC713", "onRequestRecieverWalletAddress", url + ", " + address + ", " + error );
    emit onRequestRecieverWalletAddress(url, address, error);
}


void MWC713::setRepost(int txIdx, QString err) {
    logger::logEmit( "MWC713", "onRepost", err );
    emit onRepost(txIdx, err);
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

    // Checking if foreign API enable but it is not default fro the TOR
    if (config.hasForeignApi() && !(appContext->isAutoStartTorEnabled() && config.hasForeignApi() &&
       config.foreignApiAddress == "127.0.0.1:3415" && config.foreignApiSecret.isEmpty() && !config.hasTls()))
    {
        errorMessage += "\n\nYou have activated foreign API and it might be a reason for this issue. Foreign API is deactivated, please try to restart the wallet";
        config.foreignApi = false;
        saveWalletConfig(config, nullptr, nullptr, false );
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

    if (startedMode == STARTED_MODE::RECOVER) {
        // We are good, just a wrong passphrase. We need to report it correctly.
        // let's feed outputsLines to the parser
        inputParser->processInput(outputsLines.join("\n") );
        inputParser->processInput("\n" + mwc::PROMPTS_MWC713 + "\n");
    }
    else {
        appendNotificationMessage(notify::MESSAGE_LEVEL::FATAL_ERROR,
                                  errorMessage);
    }
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
        core::getWndManager()->messageTextDlg("Read failure", "Unable to read mwc713 configuration from " + source );
        return WalletConfig();
    }

    QString network = mwc713config.getString("chain");
    QString dataPath = mwc713config.getString("wallet713_data_path");
    QString mwcmqsDomain = mwc713config.getString("mwcmqs_domain");

    bool foreignApi = mwc713config.getString("foreign_api") == "true";
    QString foreignApiAddress = mwc713config.getString("foreign_api_address");
    QString foreignApiSecret = mwc713config.getString("foreign_api_secret");
    QString tlsCertificateFile = mwc713config.getString("tls_certificate_file");
    QString tlsCertificateKey = mwc713config.getString("tls_certificate_key");

    if (foreignApiAddress.isEmpty())
        foreignApi = false;

    if (dataPath.isEmpty() ) {
        core::getWndManager()->messageTextDlg("Read failure", "Not able to find all expected mwc713 configuration values at " + source );
        return WalletConfig();
    }

    QString nodeURI     = mwc713config.getString("mwc_node_uri");
    QString nodeSecret  = mwc713config.getString("mwc_node_secret");

    return WalletConfig().setData( network, dataPath, mwcmqsDomain,
            foreignApi, foreignApiAddress, foreignApiSecret, tlsCertificateFile, tlsCertificateKey );
}


// Get current configuration of the wallet. will read from wallet713.toml file
const WalletConfig & MWC713::getWalletConfig()  {
    if (!currentConfig.isDefined())
        currentConfig = readWalletConfig();

    return currentConfig;
}

// Get configuration form the resource file.
const WalletConfig & MWC713::getDefaultConfig()  {
    return defaultConfig;
}


//static
bool MWC713::saveWalletConfig(const WalletConfig & config, core::AppContext * appContext, node::MwcNode * mwcNode, bool canStartNode ) {
    if (!config.isDefined()) {
        Q_ASSERT(false);
        logger::logInfo("MWC713", "Failed to update the config, because it is invalid:\n" + config.toString());
        return true;
    }

    QString mwc713confFN = config::getMwc713conf();

    QStringList confLines = util::readTextFile(mwc713confFN, true, false);
    // Updating the config with new values

    QStringList newConfLines;

    // TODO  Clean up keybase_binary & keybase_listener_auto_start
    // First need to wait until mwc713 will stor to support keybase
    QStringList prefixesToCheck{"wallet713_data_path", "keybase_binary", "mwcmqs_domain",
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

    // toml format doesn't allow to end section and return to the main one. So we need to keep the structure
    int appentIdx = newConfLines.size();
    for (int j=0; j<appentIdx; j++) {
        if (newConfLines[j].trimmed().startsWith('[')) {
            appentIdx = j;
            break;
        }
    }

    newConfLines.insert(appentIdx, "chain = \"" + config.getNetwork() + "\"");
    newConfLines.insert(appentIdx, "wallet713_data_path = \"" + config.getDataPath() + "\"");

    if (!config.mwcmqsDomainEx.isEmpty())
        newConfLines.insert(appentIdx, "mwcmqs_domain = \"" + config.mwcmqsDomainEx + "\"");

    if (config.hasForeignApi() && !config.foreignApiAddress.isEmpty()) {
        newConfLines.insert(appentIdx,"foreign_api = true");
        newConfLines.insert(appentIdx,"foreign_api_address = \"" + config.foreignApiAddress + "\"");

        if (!config.foreignApiSecret.isEmpty())
            newConfLines.insert(appentIdx,"foreign_api_secret = \"" + config.foreignApiSecret + "\"");

        if (!config.tlsCertificateFile.isEmpty() && !config.tlsCertificateKey.isEmpty()) {
            newConfLines.insert(appentIdx,"tls_certificate_file = \"" + config.tlsCertificateFile + "\"");
            newConfLines.insert(appentIdx,"tls_certificate_key = \"" + config.tlsCertificateKey + "\"");
        }
    }

    if ( !config::isOnlineWallet()) {
        newConfLines.insert(appentIdx,"grinbox_listener_auto_start = false");
        newConfLines.insert(appentIdx,"keybase_listener_auto_start = false");
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
                newConfLines.insert(appentIdx,"mwc_node_uri = \"http://127.0.0.1:13413\"");
                newConfLines.insert(appentIdx,"mwc_node_secret = \"" + nodeConfig.secret + "\"");
                needLocalMwcNode = true;
                break;
            }
            case wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CUSTOM:
                newConfLines.insert(appentIdx,"mwc_node_uri = \"" + connection.mwcNodeURI + "\"");
                newConfLines.insert(appentIdx,"mwc_node_secret = \"" + connection.mwcNodeSecret + "\"");
                break;
            default:
                Q_ASSERT(false);
        }

        // Update node by demand
        if ( ! needLocalMwcNode ) {
            // stopping because we don't need it...
            if (mwcNode->isRunning()) {
                mwcNode->stop();
            }
        }
        else {
            if (mwcNode->isRunning()) {
                if (mwcNode->getCurrentNetwork() != config.getNetwork()) {
                    mwcNode->stop();
                }
            }

            if (!mwcNode->isRunning() && canStartNode) {
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
bool MWC713::setWalletConfig( const WalletConfig & _config, bool canStartNode ) {
    WalletConfig config = _config;

    // Checking if Tor is active. Then we will activate Foreign API.  Or if Foreign API active wrong way, we will disable Tor
    if (appContext->isAutoStartTorEnabled()) {
        if (!config.hasForeignApi()) {
            // Expected to do that silently. It is a migration case
            config.setForeignApi(true,"127.0.0.1:3415","", "","");
        }
        else {
            // Check if Foreign API has HTTPS. Tor doesn't support it
            if (config.hasTls()) {
                core::getWndManager()->messageTextDlg("Unable to start Tor",
                                                      "Your Foreign API is configured to use TLS certificated. Tor doesn't support HTTPS connection.\n\n"
                                                      "Because of that Tor will not be started. You can review your configuration at Wallet Settings page.");
                appContext->setAutoStartTorEnabled(false);
            }
        }
    }

    if ( !saveWalletConfig( config, appContext, mwcNode, canStartNode ) ) {
        core::getWndManager()->messageTextDlg("Update Config failure", "Not able to update mwc713 configuration at " + config::getMwc713conf() );
        return false;
    }

    currentConfig = config;

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
