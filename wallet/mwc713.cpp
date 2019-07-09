#include "mwc713.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QThread>
#include "../tries/mwc713inputparser.h"
#include "mwc713events.h"
#include <QApplication>
#include "tasks/TaskStarting.h"
#include "tasks/TaskUnlock.h"
#include "tasks/TaskInit.h"
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
        mwc713Path(_mwc713path), mwc713configPath(_mwc713configPath), appContext(_appContext) {

    currentAccount = appContext->getCurrentAccountName();
}

MWC713::~MWC713() {
    stop();
}

// Generic. Reporting fatal error that somebody will process and exit app
void MWC713::reportFatalError( QString message ) noexcept(false) {
    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::GENERIC, message );
}

void MWC713::start() noexcept(false) {
    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 at " << mwc713Path << " for config " << mwc713configPath;

    // Creating process and starting
    mwc713process = new QProcess();
    mwc713process->setWorkingDirectory( QDir::homePath() );
    mwc713process->start(mwc713Path, {"--config", mwc713configPath, "-r", mwc::PROMPTS_MWC713 }, QProcess::Unbuffered | QProcess::ReadWrite );

    bool startOk = mwc713process->waitForStarted(10000);
    if (!startOk) {
        switch (mwc713process->error())
        {
            case QProcess::FailedToStart:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start mwc713 located at " + mwc713Path );
                return;
            case QProcess::Crashed:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 crashed during start" );
                return;
            case QProcess::Timedout:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start because of timeout" );
                return;
            default:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start because of unknown error" );
                return;
        }
    }

    mwc713connect();

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
}

void MWC713::stop() {
    mwc713disconnect();

    // reset mwc713 interna; state
    initStatus = InitWalletStatus::NONE;
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

    emit onWalletBalanceUpdated();

    walletPassword = "";

    if (mwc713process) {
        executeMwc713command("exit", "");
        if (!mwc713process->waitForFinished(3000) ) {
            mwc713process->terminate();
            mwc713process->waitForFinished(5000);
        }

        delete mwc713process;
        mwc713process = nullptr;
    }

    if (inputParser) {
        delete  inputParser;
        inputParser = nullptr;
    }

    if (eventCollector) {
        delete eventCollector;
        eventCollector = nullptr;
    }


}

void MWC713::loginWithPassword(QString password) noexcept(false) {
    walletPassword = password;
    eventCollector->addTask( new TaskUnlock(this, password), TaskUnlock::TIMEOUT );
}

void MWC713::generateSeedForNewAccount(QString password) noexcept(false) {
    walletPassword = password;

    eventCollector->addTask( new TaskInit(this), TaskInit::TIMEOUT );
    eventCollector->addTask( new TaskInitPassphrase(this, password), TaskInitPassphrase::TIMEOUT );
}

void MWC713::confirmNewSeed() noexcept(false) {
    eventCollector->addTask( new TaskInitConfirm(this), TaskInitConfirm::TIMEOUT );

    // Wallet status is Ready now...
    setInitStatus(INIT_STATUS::READY);
}


void MWC713::recover(const QVector<QString> & seed, QString password) noexcept(false) {
    walletPassword = password;

    if ( initStatus==InitWalletStatus::READY )
    {
        eventCollector->addTask( new TaskRecoverFull(this, seed, password), TaskRecoverFull::TIMEOUT );
    }
    else {
        eventCollector->addTask( new TaskRecover1Type(this), TaskRecover1Type::TIMEOUT );
        eventCollector->addTask( new TaskRecover2Mnenonic(this, seed), TaskRecover2Mnenonic::TIMEOUT );
        eventCollector->addTask( new TaskRecover3Password(this, password), TaskRecover3Password::TIMEOUT );
    }

}

// Current seed for runnign wallet
// Check Signals: onGetSeed(QVector<QString> seed);
void MWC713::getSeed() noexcept(false) {
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
QPair<bool,bool> MWC713::getListeningStatus() noexcept(false) {
    return QPair<bool,bool>(mwcMqOnline, keybaseOnline);
}

// Check Signal: onListeningStartResults
void MWC713::listeningStart(bool startMq, bool startKb) noexcept(false) {
    if (initStatus==InitWalletStatus::READY) {
        eventCollector->addTask( new TaskListeningStart(this, startMq,startKb), TaskListeningStart::TIMEOUT );
    }
    else {
        setListeningStartResults( startMq, startKb, // what we try to start
                                 QStringList{"mwc713 wallet not started yet"} );
    }
}

// Check signal: onListeningStopResult
void MWC713::listeningStop(bool stopMq, bool stopKb) noexcept(false) {
    qDebug() << "listeningStop: mq=" << stopMq << ",kb=" << stopKb;
    if (initStatus==InitWalletStatus::READY) {
        eventCollector->addTask( new TaskListeningStop(this, stopMq,stopKb), TaskListeningStop::TIMEOUT );
    }
    else  {
        logger::logEmit("MWC713", "setListeningStopResult", "stopMq=" + QString::number(stopMq) + " stopKb="+QString::number(stopKb));
        emit setListeningStopResult(stopMq, stopKb, QStringList{"mwc713 wallet not started yet"} );
    }
}

// Get latest Mwc MQ address that we see
QString MWC713::getLastKnownMwcBoxAddress() noexcept(false) {
    return mwcAddress;
}

// Get MWC box <address, index in the chain>
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MWC713::getMwcBoxAddress() noexcept(false) {
    eventCollector->addTask( new TaskMwcMqAddress(this,false, -1), TaskMwcMqAddress::TIMEOUT );
}

// Change MWC box address to another from the chain. idx - index in the chain.
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MWC713::changeMwcBoxAddress(int idx) noexcept(false) {
    eventCollector->addTask( new TaskMwcMqAddress(this,true, idx), TaskMwcMqAddress::TIMEOUT );
}

// Generate next box address
// Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);
void MWC713::nextBoxAddress() noexcept(false) {
    eventCollector->addTask( new TaskMwcMqAddress(this,true, -1), TaskMwcMqAddress::TIMEOUT );
}

QVector<AccountInfo>  MWC713::getWalletBalance(bool filterDeleted) const noexcept(false) {
    if (!filterDeleted)
        return accountInfo;

    QVector<AccountInfo> res;

    for (const auto & acc : accountInfo ) {
        if (!acc.isDeleted())
            res.push_back(acc);
    }

    return res;
}


// Request Wallet balance update. It is a multistep operation
// Check signal: onWalletBalanceUpdated
//          onWalletBalanceProgress
void MWC713::updateWalletBalance() noexcept(false) {
    // Steps:
    // 1 - list accounts (this call)
    // 2 - for every account get info ( see updateAccountList call )
    // 3 - restore back current account

    eventCollector->addTask( new TaskAccountList(this), TaskAccountList::TIMEOUT );
}

// Create another account, note no delete exist for accounts
// Check Signal:  onAccountCreated
void MWC713::createAccount( const QString & accountName ) noexcept(false) {

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
void MWC713::switchAccount(const QString & accountName) noexcept(false) {
    // Expected that account is in the list
    eventCollector->addTask( new TaskAccountSwitch(this, accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );
}

// Rename account
// Check Signal: onAccountRenamed(bool success, QString errorMessage);
void MWC713::renameAccount(const QString & oldName, const QString & newName) noexcept(false) {
    eventCollector->addTask( new TaskAccountRename(this, oldName, newName, false), TaskAccountRename::TIMEOUT );
}

// Check and repair the wallet. Will take a while
// Check Signals: onRecoverProgress( int progress, int maxVal );
// Check Signals: onCheckResult(bool ok, QString errors );
void MWC713::check(bool wait4listeners) noexcept(false) {
    eventCollector->addTask( new TaskCheck(this,wait4listeners), TaskCheck::TIMEOUT );
}


// Send some coins to address.
// Before send, wallet always do the switch to account to make it active
// Check signal:  onSend
void MWC713::sendTo( const wallet::AccountInfo &account, int64_t coinNano, const QString & address, QString message, int inputConfirmationNumber, int changeOutputs ) noexcept(false) {
    // switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account.accountName, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    // If listening, strting...

    eventCollector->addTask( new TaskSendMwc(this, coinNano, address, message, inputConfirmationNumber, changeOutputs), TaskSendMwc::TIMEOUT );
    // Set some funds

}


// Init send transaction with file output
// Check signal:  onSendFile
void MWC713::sendFile( int64_t coinNano, QString fileTx ) noexcept(false) {
    eventCollector->addTask( new TaskSendFile(this, coinNano, fileTx ), TaskSendFile::TIMEOUT );
}

// Recieve transaction. Will generate *.response file in the same dir
// Check signal:  onReceiveFile
void MWC713::receiveFile( QString fileTx) noexcept(false) {
    eventCollector->addTask( new TaskReceiveFile(this, fileTx), TaskReceiveFile::TIMEOUT );
}

// finalize transaction and broadcast it
// Check signal:  onFinalizeFile
void MWC713::finalizeFile( QString fileTxResponse ) noexcept(false) {
    eventCollector->addTask( new TaskFinalizeFile(this, fileTxResponse), TaskFinalizeFile::TIMEOUT );
}

// Show outputs for the wallet
// Check Signal: onOutputs( QString account, int64_t height, QVector<WalletOutput> Transactions)
void MWC713::getOutputs(QString account) noexcept(false) {
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskOutputs(this), TaskOutputs::TIMEOUT );
}

void MWC713::getTransactions(QString account) noexcept(false) {
    // Need to switch account first
    eventCollector->addTask( new TaskAccountSwitch(this, account, walletPassword, true), TaskAccountSwitch::TIMEOUT );
    eventCollector->addTask( new TaskTransactions(this), TaskTransactions::TIMEOUT );
}

// -------------- Transactions

// Set account that will receive the funds
// Check Signal:  onSetReceiveAccount( bool ok, QString AccountOrMessage );
void MWC713::setReceiveAccount(QString account) noexcept(false) {
    eventCollector->addTask( new TaskSetReceiveAccount(this, account, walletPassword), TaskSetReceiveAccount::TIMEOUT );
}


// Cancel transaction
// Check Signal:  onCancelTransacton
void MWC713::cancelTransacton(int64_t transactionID) noexcept(false) {
    eventCollector->addTask( new TaskTransCancel(this, transactionID), TaskTransCancel::TIMEOUT );
}


// Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
// Check Signal: onExportProof( bool success, QString fn, QString msg );
void MWC713::generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName ) noexcept(false) {
    eventCollector->addTask( new TaskTransExportProof(this, resultingFileName, transactionId), TaskTransExportProof::TIMEOUT );
}

// Verify the proof for transaction
// Check Signal: onVerifyProof( bool success, QString msg );
void MWC713::verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) {
    eventCollector->addTask( new TaskTransVerifyProof(this, proofFileName), TaskTransExportProof::TIMEOUT );
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

// Wallet init status
//enum INIT_STATUS {NONE, NEED_PASSWORD, NEED_SEED, WRONG_PASSWORD, READY };
void MWC713::setInitStatus( INIT_STATUS  status ) {
    switch(status) {
        case INIT_STATUS::NEED_PASSWORD:
            initStatus = InitWalletStatus::NEED_PASSWORD;
            break;
        case INIT_STATUS::NEED_SEED:
            initStatus = InitWalletStatus::NEED_INIT;
            break;
        case INIT_STATUS::WRONG_PASSWORD:
            initStatus = InitWalletStatus::WRONG_PASSWORD;
            break;
        case INIT_STATUS::READY:
            initStatus = InitWalletStatus::READY;
            break;
        default:
            initStatus = InitWalletStatus::NONE;
            break;
    }
    logger::logEmit("MWC713", "onInitWalletStatus", toString(initStatus) );
    emit onInitWalletStatus(initStatus);
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
    mwcMqOnline = online;
    logger::logEmit("MWC713", "onMwcMqListenerStatus", QString("online=") + QString::number(online));
    emit onMwcMqListenerStatus(online);

}
void MWC713::setKeybaseListeningStatus(bool online) {
    keybaseOnline = online;
    logger::logEmit("MWC713", "onKeybaseListenerStatus", QString("online=") + QString::number(online));
    emit onKeybaseListenerStatus(online);
}

void MWC713::setRecoveryResults( bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages ) {
    logger::logEmit("MWC713", "onRecoverResult", QString("started=") + QString::number(started) +
           " finishedWithSuccess=" + QString::number(finishedWithSuccess) +
           " newAddress=" + newAddress + " errorMessages size " + QString::number(errorMessages.size()) );

    if (finishedWithSuccess) {
        setInitStatus(INIT_STATUS::READY);
    }

    emit onRecoverResult(started, finishedWithSuccess, newAddress, errorMessages);

    // Update the address as well
    if ( newAddress.length()>0 ) {
        setMwcAddress(newAddress);
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
    logger::logEmit( "MWC713", "onSlateSend", slate + " with " +mwc + " to " + sendAddr );
    emit onSlateSend(slate, mwc, sendAddr);
}
void MWC713::reportSlateRecieved( QString slate, QString mwc, QString fromAddr ) {
    logger::logEmit( "MWC713", "onSlateRecieved", slate + " with " +mwc + " from " + fromAddr );
    emit onSlateRecieved( slate, mwc, fromAddr );
}
void MWC713::reportSlateFinalized( QString slate ) {
    logger::logEmit( "MWC713", "onSlateFinalized", slate );
    emit onSlateFinalized(slate);
}

void MWC713::setSendFileResult( bool success, QStringList errors, QString fileName ) {
    logger::logEmit( "MWC713", "onSendFile", "success="+QString::number(success) );
    emit onSendFile(success, errors, fileName);
}

void MWC713::setReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn ) {
    logger::logEmit( "MWC713", "onReceiveFile", "success="+QString::number(success) );
    emit onReceiveFile( success, errors, inFileName, outFn );
}

void MWC713::setFinalizeFile( bool success, QStringList errors, QString fileName ) {
    logger::logEmit( "MWC713", "onFinalizeFile", "success="+QString::number(success) );
    emit onFinalizeFile( success, errors, fileName);
}

// Transactions
void MWC713::setTransactions( QString account, int64_t height, QVector<WalletTransaction> Transactions ) {
    logger::logEmit( "MWC713", "onTransactions", "account="+account );
    emit onTransactions( account, height, Transactions );
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
    logger::logEmit( "MWC713", "onSetReceiveAccount", "ok="+QString::number(ok) );
    emit onSetReceiveAccount(ok, accountOrMessage );
}

void MWC713::setCheckResult(bool ok, QString errors) {
    logger::logEmit( "MWC713", "onCheckResult", "ok="+QString::number(ok) );
    emit onCheckResult(ok, errors );
}


/////////////////////////////////////////////////////////////////////////////////
//      mwc713  IOs

void MWC713::mwc713connect() {
    mwc713disconnect();
    Q_ASSERT(mwc713connections.isEmpty());
    Q_ASSERT(mwc713process);

    if (mwc713process) {
        mwc713connections.push_back( connect( mwc713process, &QProcess::errorOccurred, this, &MWC713::mwc713errorOccurred, Qt::QueuedConnection) );
        mwc713connections.push_back( connect( mwc713process, SIGNAL(finished(int , QProcess::ExitStatus )), this,
                                              SLOT(mwc713finished(int , QProcess::ExitStatus )) ));
        mwc713connections.push_back( connect( mwc713process, &QProcess::readyReadStandardError, this, &MWC713::mwc713readyReadStandardError, Qt::QueuedConnection) );
        mwc713connections.push_back( connect( mwc713process, &QProcess::readyReadStandardOutput, this, &MWC713::mwc713readyReadStandardOutput, Qt::QueuedConnection) );
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
        delete mwc713process;
        mwc713process = nullptr;
    }

    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::MWC7113_ERROR,
                               "mwc713 process exited. Process error: "+ QString::number(error) );

}

void MWC713::mwc713finished(int exitCode, QProcess::ExitStatus exitStatus) {
    logger::logMwc713out("Exit with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );

    qDebug() << "mwc713 is exiting with exit code " << exitCode << ", exitStatus=" << exitStatus;

    if (mwc713process) {
        delete mwc713process;
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

// Get current configuration of the wallet. will read from wallet713.toml file
WalletConfig MWC713::getWalletConfig() noexcept(false) {

    QString mwc713confFN = core::Config::getMwc713conf();

    util::ConfigReader  mwc713config;

    if (!mwc713config.readConfig(mwc713confFN) ) {
        control::MessageBox::message(nullptr, "Read failure", "Unable to read mwc713 configuration from " + mwc713confFN );
        return WalletConfig();
    }

    QString dataPath = mwc713config.getString("wallet713_data_path");
    QString keyBasePath = mwc713config.getString("keybase_binary");
    QString mwcmqDomain = mwc713config.getString("mwcmq_domain");

    if (dataPath.isEmpty() || keyBasePath.isEmpty() || mwcmqDomain.isEmpty()) {
        control::MessageBox::message(nullptr, "Read failure", "Not able to find all expected mwc713 configuration values at " + mwc713confFN );
        return WalletConfig();
    }

    return WalletConfig().setData(dataPath, mwcmqDomain, keyBasePath);
}

// Update wallet config. Will update config and restart the wmc713.
// Note!!! Caller is fully responsible for input validation. Normally mwc713 will sart, but some problems might exist
//          and caller suppose listen for them
bool MWC713::setWalletConfig(const WalletConfig & config) noexcept(false) {
    if (!config.isDefined())
        return false;

    QString mwc713confFN = core::Config::getMwc713conf();

    QStringList confLines = util::readTextFile( mwc713confFN );
    // Updating the config with new values

    for (QString & ln : confLines) {
        if (ln.startsWith("wallet713_data_path"))
            ln = "wallet713_data_path = \"" + config.dataPath + "\"";
        else if (ln.startsWith("keybase_binary"))
            ln = "keybase_binary = \"" + config.keyBasePath + "\"";
        else if (ln.startsWith("mwcmq_domain"))
            ln = "mwcmq_domain = \"" + config.mwcmqDomain + "\"";
    }

    if (!util::writeTextFile( mwc713confFN, confLines )) {
        control::MessageBox::message(nullptr, "Read failure", "Not able to find all expected mwc713 configuration values at " + mwc713confFN );
        return false;
    }

    util::Waiting w; // Host verifucation might tale time, what is why waiting here

    // Stopping the wallet. Start will be done by init state
    stop();
    return true;
}



}
