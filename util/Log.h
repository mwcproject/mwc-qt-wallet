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

#include <QObject>
#include "../wallet/mwc713events.h"

class QFile;

namespace wallet {
    class Mwc713Task;
}

// Note, logging is not generic. This logging tracks very specific action that most critical
// for wallet interraction.
// For generic logs we are using qDebug
namespace logger {

    class LogSender : public QObject {
        Q_OBJECT
    public:
        LogSender(bool _asyncLogging) : asyncLogging(_asyncLogging) {}
        virtual ~LogSender() override {}

        void log(bool addDate, const QString & prefix, const QString & line);

    signals:
        void doAppend2logs(bool addDate, QString prefix, QString line );
    private:
        bool asyncLogging; // Use QT messaging or write directly. Direct writing might cause concurrency issues
    };

    class LogReceiver : public QObject {
        Q_OBJECT
    public:
        LogReceiver(const QString & filename);
        virtual ~LogReceiver() override;

    public slots:
        void onAppend2logs(bool addDate, QString prefix, QString line );
    private:
        QFile * logFile;
    };

    // Must be call before first log usage
    void initLogger();

    // mwc713 IOs
    void blockLogMwc713out(bool blockOutput);
    void logMwc713out(QString str); //
    void logMwc713in(QString str); //

    void logParsingEvent(wallet::WALLET_EVENTS event, QString message );

    // Tasks to excecute on wmc713
    void logTask( QString who, wallet::Mwc713Task * task, QString comment );

    // Events activity
    void logEmit(QString who, QString event, QString params);
    void logInfo(QString who, QString message);
}


#endif //GUI_WALLET_LOG_H
