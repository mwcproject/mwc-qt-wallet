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
#include "tasks/TaskInitPassphrase.h"
#include "tasks/TaskInitConfirm.h"
#include "tasks/TaskListeningStart.h"
#include "tasks/TaskListeningStop.h"
#include "tasks/TaskRecoverFull.h"
#include "tasks/TaskRecover1Type.h"
#include "tasks/TaskRecover2Mnenonic.h"
#include "tasks/TaskRecover3Password.h"
#include "tasks/TaskRecoverProgressListener.h"
#include "tasks/TaskErrWrnInfoListener.h"
#include "tasks/TaskListeningListener.h"
#include "tasks/TaskMwcMqAddress.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../control/messagebox.h"

namespace wallet {

// Base class for wallet state, Non managed object. Consumer suppose to delete it
class Mwc713State {
    Mwc713State();
    virtual ~Mwc713State();
};

MWC713::MWC713(QString _mwc713path, QString _mwc713configPath) : mwc713Path(_mwc713path), mwc713configPath(_mwc713configPath) {
}

MWC713::~MWC713() {
    mwc713disconnect();

    if (mwc713process) {
        mwc713process->terminate();
        mwc713process->waitForFinished(5000);
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

// Generic. Reporting fatal error that somebody will process and exit app
void MWC713::reportFatalError( QString message ) noexcept(false) {
    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::GENERIC, message );
}

void MWC713::start() noexcept(false) {
    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 at " << mwc713Path << " for config " << mwc713configPath;

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

    // And eventing magic should begin...
}


void MWC713::loginWithPassword(QString password, QString account) noexcept(false) {
    eventCollector->addTask( new TaskUnlock(this, password, account), TaskUnlock::TIMEOUT );
}

void MWC713::generateSeedForNewAccount(QString password) noexcept(false) {
    eventCollector->addTask( new TaskInit(this), TaskInit::TIMEOUT );
    eventCollector->addTask( new TaskInitPassphrase(this, password), TaskInitPassphrase::TIMEOUT );
}

void MWC713::confirmNewSeed() noexcept(false) {
    eventCollector->addTask( new TaskInitConfirm(this), TaskInitConfirm::TIMEOUT );

    // Wallet status is Ready now...
    setInitStatus(INIT_STATUS::READY);
}


void MWC713::recover(const QVector<QString> & seed, QString password) noexcept(false) {

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
    if (initStatus==InitWalletStatus::READY) {
        eventCollector->addTask( new TaskListeningStop(this, stopMq,stopKb), TaskListeningStop::TIMEOUT );
    }
    else  {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Feed the command to mwc713 process
void MWC713::executeMwc713command(QString cmd) {
    Q_ASSERT(mwc713process);
    log::logMwc713in(cmd);
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

    log::logEmit( "MWC713", "onNewNotificationMessage", msg.toString() );
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
    log::logEmit("MWC713", "onInitWalletStatus", toString(initStatus) );
    emit onInitWalletStatus(initStatus);
}

void MWC713::setMwcAddress( QString _mwcAddress ) { // Set active MWC address. Listener might be offline
    mwcAddress = _mwcAddress;
    log::logEmit("MWC713", "onMwcAddress", mwcAddress );
    emit onMwcAddress(mwcAddress);
}

void MWC713::setMwcAddressWithIndex( QString _mwcAddress, int idx ) {
    mwcAddress = _mwcAddress;
    log::logEmit("MWC713", "setMwcAddressWithIndex", mwcAddress + ", idx=" + QString::number(idx) );
    emit onMwcAddress(mwcAddress);
    emit onMwcAddressWithIndex(mwcAddress, idx);
}


void MWC713::setNewSeed( QVector<QString> seed ) {
    log::logEmit("MWC713", "onNewSeed", "????" );
    emit onNewSeed(seed);
}

void MWC713::setListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                               QStringList errorMessages ) {
    log::logEmit("MWC713", "onListeningStartResults", QString("mqTry=") + QString::number(mqTry) +
            " kbTry=" + QString::number(kbTry) + " errorMessages size " + QString::number(errorMessages.size()) );
    emit onListeningStartResults(mqTry, kbTry,errorMessages);
}

void MWC713::setListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                            QStringList errorMessages ) {
    log::logEmit("MWC713", "onListeningStopResult", QString("mqTry=") + QString::number(mqTry) +
             " kbTry=" + QString::number(kbTry) + " errorMessages size " + QString::number(errorMessages.size()) );
    emit onListeningStopResult(mqTry, kbTry,errorMessages);

}

void MWC713::setMwcMqListeningStatus(bool online) {
    mwcMqOnline = online;
    log::logEmit("MWC713", "onMwcMqListenerStatus", QString("online=") + QString::number(online));
    emit onMwcMqListenerStatus(online);

}
void MWC713::setKeybaseListeningStatus(bool online) {
    keybaseOnline = online;
    log::logEmit("MWC713", "onKeybaseListenerStatus", QString("online=") + QString::number(online));
    emit onKeybaseListenerStatus(online);
}

void MWC713::setRecoveryResults( bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages ) {
    log::logEmit("MWC713", "onRecoverResult", QString("started=") + QString::number(started) +
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

void MWC713::setRecoveryProgress( long progress, long limit ) {
    log::logEmit("MWC713", "onRecoverProgress", QString("progress=") + QString::number(progress) +
                                              " limit=" + QString::number(limit) );
    emit onRecoverProgress( int(progress), int(limit) );
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
    log::logMwc713out("ERROR OCCURRED. Error = " + QString::number(error)  );

    qDebug() << "ERROR OCCURRED. Error = " << error;

    if (mwc713process) {
        delete mwc713process;
        mwc713process = nullptr;
    }

    appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::MWC7113_ERROR,
                               "mwc713 process exited. Process error: "+ QString::number(error) );

}

void MWC713::mwc713finished(int exitCode, QProcess::ExitStatus exitStatus) {
    log::logMwc713out("Exit with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );

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
    log::logMwc713out(str);
    inputParser->processInput(str);
}


}
