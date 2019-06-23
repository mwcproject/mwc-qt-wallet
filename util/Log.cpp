#include "Log.h"
#include "ioutils.h"
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDateTime>
#include "../wallet/mwc713task.h"
#include "../control/messagebox.h"

// 1 MB is a reasonable size limit.
#define LOG_SIZE_LIMIT  1000000

namespace logger {

static LogSender *   logClient = nullptr;
static LogReciever * logServer = nullptr;

static bool logMwc713outBlocked = false;

void initLogger() {
    logClient = new LogSender(true);
    logServer = new LogReciever("mwcwallet.log");

    bool connected = QObject::connect( logClient, &LogSender::doAppend2logs, logServer, &LogReciever::onAppend2logs, Qt::QueuedConnection );
    Q_ASSERT(connected);
    Q_UNUSED(connected);

    logClient->doAppend2logs(true, "", "mwc-wallet is started..." );
}

void LogSender::log(bool addDate, const QString & prefix, const QString & line) {
    if (asyncLogging) {
        emit doAppend2logs(addDate, prefix, line);
    }
    else {
        logServer->onAppend2logs(addDate, prefix, line );
    }
}

// Create logger file with some simplest rotation
LogReciever::LogReciever(const QString & filename) {
    QString logPath = ioutils::getAppDataPath("logs");

    QString logFn = logPath + "/" + filename;

    { // Check if need to rotate
        QFileInfo fi(logFn);

        if (fi.size() > LOG_SIZE_LIMIT) {
            const QString prevLogFn = "prev_"+filename;
            QDir logDir( logPath );
            logDir.remove(prevLogFn);
            logDir.rename(filename, prevLogFn);
        }
    }

    logFile = new QFile( logFn );
    if (! logFile->open( QFile::WriteOnly | QFile::Append ) ) {
        control::MessageBox::message(nullptr, "Critical Error", "Unable to open the logger file: " + logPath );
        QApplication::quit();
        return;
    }
}
LogReciever::~LogReciever() {
    delete logFile;
}

void LogReciever::onAppend2logs(bool addDate, QString prefix, QString line ) {
    QString logLine;
    if (addDate)
        logLine += QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + " ";

    logLine += prefix + " " + line + "\n";

    logFile->write( logLine.toUtf8() );
    logFile->flush();
}

// Global methods that do logging

void blockLogMwc713out(bool blockOutput) {
    logMwc713outBlocked = blockOutput;
}


// mwc713 IOs
void logMwc713out(QString str) {
    Q_ASSERT(logClient); // call initLogger first

    if (logMwc713outBlocked) {
        logClient->doAppend2logs(true, "mwc713>>", "CENSORED");
        return;
    }

    auto lns = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    for (auto & l: lns) {
        logClient->doAppend2logs(true, "mwc713>>", l);
    }
}

void logMwc713in(QString str) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->doAppend2logs(true, "mwc713<<", str);
}

// Tasks to excecute on wmc713
void logTask( QString who, wallet::Mwc713Task * task, QString comment ) {
    Q_ASSERT(logClient); // call initLogger first
    if (task == nullptr)
        return;
    logClient->doAppend2logs(true, who, "Task " + task->toDbgString() + "  "+comment );
}

// Events activity
void logEmit(QString who, QString event, QString params) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->doAppend2logs(true, who, "emit " + event + (params.length()==0 ? "" : (" with "+params)) );
}

void logRecieve(QString who, QString event, QString params) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->doAppend2logs(true, who, "recieve " + event + (params.length()==0 ? "" : (" with "+params)) );
}

void logConnect(QString who, QString event) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->doAppend2logs(true, who, "connect to " + event);
}

void logDisconnect(QString who, QString event) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->doAppend2logs(true, who, "disconnect from " + event);
}

void logParsingEvent(wallet::WALLET_EVENTS event, QString message ) {
    Q_ASSERT(logClient); // call initLogger first
    if (logMwc713outBlocked) { // Skipping event during block pahse as well
        logClient->doAppend2logs(true, "Event>", "CENSORED" );
        return;
    }

    logClient->doAppend2logs(true, "Event>", toString(event) + " [" + message + "]" );
}


}


