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

#include "Log.h"
#include "ioutils.h"
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDateTime>
#include <QProcess>

#include "message_mapper.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include "wallet/api/MwcWalletApi.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#endif

// 10 MB is a reasonable size limit.
// Compressed will be around 1 MB.
#define LOG_SIZE_LIMIT  10000000
// Number of files for rotation
#define LOG_FILES_POOL_SIZE 50


namespace logger {

static LogSender *   logClient = nullptr;
static LogReceiver * logServer = nullptr;

//static bool logMwc713outBlocked = false;

const QString LOG_FILE_NAME = "mwcwallet.log";

QString who2str(Who who) {
    switch (who) {
        case LIB_LOGS: {
            return "LIB_LOGS";
        }
        case MWC_WALLET: {
            return "MWC_WALLET";
        }
        case NODE_CLIENT: {
            return "NODE_CLIENT";
        }
        case MWC_NODE: {
            return "MWC_NODE";
        }
        case HTTP_CLIENT: {
            return "HTTP_CLIENT";
        }
        case QT_WALLET: {
            return "QT_WALLET";
        }
        case NOTIFICATION: {
            return "NOTIFICATION";
        }
        case STATE: {
            return "STATE";
        }
        case BRIDGE: {
            return "BRIDGE";
        }
        default: {
            Q_ASSERT(false);
            return "UNKNOWN";
        }
    }
}

void initLogger( bool logsEnabled) {
    logClient = new LogSender(true);

    enableLogs(logsEnabled);

    logClient->log(true, "", "mwc-qt-wallet is started..." );
}

void cleanUpLogs() {
    // Logs expected to be disabled first
    Q_ASSERT(logServer == nullptr );
    QPair<bool,QString> logPath = ioutils::getAppDataPath("logs");
    if (!logPath.first) {
        core::getWndManager()->messageTextDlg("Error", logPath.second);
        return;
    }

    QFile::remove(logPath.second + "/" + LOG_FILE_NAME);
    QFile::remove(logPath.second + "/prev_" + LOG_FILE_NAME);
}

// enable/disable logs
void enableLogs( bool enableLogs ) {
    if (enableLogs) {
        if (logServer != nullptr )
            return;

        logServer = new LogReceiver(LOG_FILE_NAME);
        QObject::connect( logClient, &LogSender::doAppend2logs, logServer, &LogReceiver::onAppend2logs, Qt::DirectConnection); // Qt::QueuedConnection );
    }
    else {
        if (logServer == nullptr)
            return;

        delete logServer;
        logServer = nullptr;
    }
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
LogReceiver::LogReceiver(const QString & filename) :
        logFileName(filename)
{
    QPair<bool,QString> path = ioutils::getAppDataPath("logs");
    if (!path.first) {
        core::getWndManager()->messageTextDlg("Error", path.second);
        QCoreApplication::exit();
        return;
    }

    logPath = path.second;

    rotateLogFileIfNeeded();
    openLogFile();
}

LogReceiver::~LogReceiver() {
    delete logFile;
}

void LogReceiver::rotateLogFileIfNeeded() {
    QString logPathName = logPath + "/" + logFileName;
    QFileInfo fi(logPathName);

    if (fi.size() < LOG_SIZE_LIMIT)
        return;

    qDebug() << "Rotating logs file: " << logPathName;

    // First check if need to clean up
    QStringList archives = QDir(logPath).entryList( {"*.zip"} );
    if (archives.size()>LOG_FILES_POOL_SIZE) {
        // Need to delete some. Since names can be sorted,
        archives.sort(Qt::CaseSensitivity::CaseInsensitive);
        while(archives.size()>LOG_FILES_POOL_SIZE) {
            qDebug() << "Cleaning up old archive: " << archives.front();
            QFile::remove(logPath + "/" + archives.front());
            archives.pop_front();
        }
    }

    bool logFileOpen = (logFile != nullptr);
    if (logFile) {
        delete logFile;
        logFile = nullptr;
    }

    // Generate the file name
    QDateTime  now = QDateTime::currentDateTime();
    QString fn = now.toString("yyyy_MM_dd_hh_mm_ss_zzz");
    QString archiveFileName = fn +".zip";

    QString srcFileName = logPath + "/" + logFileName;
    QString resultFileName = logPath + "/" + archiveFileName;

    // Find the mwczip location. It is expected ta the same directory where mwc713 located

    qDebug() << "Creating zip archive: " << resultFileName;

    // 3 is OK for the
    mwc_api::ApiResponse<bool> res = wallet::zip_file( srcFileName, resultFileName , fn+".log");
    QDir logDir( logPath );

    if (res.hasError()) {
        core::getWndManager()->messageTextDlg("Log files rotation",
            "Unable to rotate log file at " + logPath + "\nYour previous file will be swapped with a new log data.\n\n" +
            util::mapMessage(res.error));
        const QString prevLogFn = "prev_"+logFileName;
        logDir.remove(prevLogFn);
        logDir.rename(logFileName, prevLogFn);
    }
    else {
        logDir.remove(logFileName);
    }

    if (logFileOpen)
        openLogFile();
}

void LogReceiver::openLogFile() {
    QString logFn = logPath + "/" + logFileName;
    logFile = new QFile(logFn);
    if (!logFile->open(QFile::WriteOnly | QFile::Append)) {
        core::getWndManager()->messageTextDlg("Critical Error", "Unable to open the logger file: " + logPath);
        QApplication::quit();
        return;
    }
}


void LogReceiver::onAppend2logs(bool addDate, QString prefix, QString line ) {
    counter++;
    if (counter>10000) {
        counter = 0;
        rotateLogFileIfNeeded();
    }


    QString logLine;
    if (addDate)
        logLine += QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") + " ";

    logLine += prefix + " " + line + "\n";

    if (logFile!=nullptr) {
        logFile->write( logLine.toUtf8() );
        logFile->flush();
    }
}

// Global methods that do logging

// Events activity
void logEmit(Who who, QString event, QString params) {
    // in tests there is no logger
    if (logClient) // call initLogger first
        logClient->log(true, who2str(who), "emit " + event + (params.length()==0 ? "" : (" with "+params)) );
}

void logDebug(Who who, QString message) {
#ifndef QT_NO_DEBUG
    Q_ASSERT(logClient); // call initLogger first
    logClient->log(true, who2str(who), "DEBUG " + message );
#endif
}

void logInfo(Who who, QString message) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->log(true, who2str(who), "INFO  " +  message );
}

void logError(Who who, QString message) {
    Q_ASSERT(logClient); // call initLogger first
    logClient->log(true, who2str(who), "ERROR " + message );
}

}


