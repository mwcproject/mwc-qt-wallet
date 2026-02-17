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

#ifndef GUI_WALLET_LOG_H
#define GUI_WALLET_LOG_H

#include <QString>

class QFile;

namespace wallet {
    class Mwc713Task;
}

// Note, logging is not generic. This logging tracks very specific action that most critical
// for wallet interraction.
// For generic logs we are using qDebug
namespace logger {

    enum Who {
        LIB_LOGS,
        MWC_WALLET,
        NODE_CLIENT,
        MWC_NODE,
        HTTP_CLIENT,
        QT_WALLET,
        NOTIFICATION,
        STATE,
        BRIDGE,
    };

    QString who2str(Who who);

    class LogSender {
    public:
        LogSender() = default;
        ~LogSender() = default;

        void log(bool addDate, const QString & prefix, const QString & line);
    };

    class LogReceiver {
    public:
        LogReceiver(const QString & filename);
        ~LogReceiver();

    public:
        void onAppend2logs(bool addDate, QString prefix, QString line );
    private:
        void rotateLogFileIfNeeded();
        void openLogFile();
    private:
        QString logPath;
        const QString logFileName;
        QFile * logFile = nullptr;
        int counter = 0;
    };

    // Must be call before first log usage
    void initLogger(bool logsEnabled);

    // Events activity
    void logEmit(Who who, QString event, QString params);

    void logInfo(Who who, QString message);
    void logDebug(Who who, QString message);
    void logError(Who who, QString message);

    // enable/disable logs
    void enableLogs( bool enableLogs );
    // clean all logs
    void cleanUpLogs();
}


#endif //GUI_WALLET_LOG_H
