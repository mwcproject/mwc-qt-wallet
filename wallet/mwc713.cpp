#include "mwc713.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QThread>
#include "mwc713reader.h"
#include "../tries/mwc713inputparser.h"
#include "mwc713events.h"
#include <QMessageBox>
#include <QApplication>
#include "tasks/TaskStarting.h"
#include "tasks/TaskUnlock.h"

namespace wallet {

// Base class for wallet state, Non managed object. Consumer suppose to delete it
class Mwc713State {
    Mwc713State();
    virtual ~Mwc713State();
};

MWC713::MWC713(QString _mwc713path) : mwc713Path(_mwc713path) {
}

MWC713::~MWC713() {

    if (reader != nullptr) {
        reader->stopReader();
        reader->wait(MWC713_READER_WAITING_PERIOD_MS*2);
        delete reader;
        reader = nullptr;
    }

    if (mwc713process) {
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

void MWC713::start(QString network) noexcept(false) {
    // Start the binary
    Q_ASSERT(mwc713process == nullptr);
    Q_ASSERT(inputParser == nullptr);

    qDebug() << "Starting MWC713 at " << mwc713Path << " for network '" << network << "'";

    QStringList arguments;
    if (network.length()>0)
        arguments.push_back("--"+network);

    mwc713process = new QProcess();
    mwc713process->setWorkingDirectory( QDir::homePath() );
    mwc713process->start(mwc713Path, arguments);

    bool startOk = mwc713process->waitForStarted(10000);
    if (!startOk) {
        switch (mwc713process->error())
        {
            case QProcess::FailedToStart:
                appendNotificationMessage( MESSAGE_LEVEL::FATAL_ERROR, MESSAGE_ID::INIT_ERROR, "mwc713 failed to start" );
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

    inputParser = new tries::Mwc713InputParser();
    reader = new Mwc713reader(mwc713process, inputParser);

    eventCollector = new Mwc713EventManager(this);
    eventCollector->connectWith(inputParser);
    eventCollector->addTask( new TaskStarting(this), TaskStarting::TIMEOUT );

    reader->start();

    // And eventing magic should begin...
}


void MWC713::loginWithPassword(QString password, QString account) noexcept(false) {
    eventCollector->addTask( new TaskUnlock(this, password, account), TaskUnlock::TIMEOUT );
}

// Feed the command to mwc713 process
void MWC713::executeMwc713command(QString cmd) {
    Q_ASSERT(mwc713process);

    mwc713process->write( (cmd + "\n").toLocal8Bit() );
}

// Task Reporting methods
//enum MESSAGE_LEVEL { FATAL_ERROR, WARNING, INFO, DEBUG };
//enum MESSAGE_ID {INIT_ERROR, GENERIC, TASK_TIMEOUT };
void MWC713::appendNotificationMessage( MESSAGE_LEVEL level, MESSAGE_ID id, QString message ) {
    if (level == MESSAGE_LEVEL::FATAL_ERROR) {
        // Fatal error. Display message box and exiting. We don't want to continue
        QMessageBox::critical(nullptr, "mwc713 Error", "Wallet get a critical error from mwc713 process:\n" + message + "\n\nPress OK to exit the wallet" );
        QApplication::quit();
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
    emit onInitWalletStatus(initStatus);
}

void MWC713::setMwcAddress( QString _mwcAddress ) { // Set active MWC address. Listener might be offline
    mwcAddress = _mwcAddress;
    emit onMwcAddress(mwcAddress);
}


}
