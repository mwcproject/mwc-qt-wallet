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

    class LogReciever : public QObject {
        Q_OBJECT
    public:
        LogReciever(const QString & filename);
        virtual ~LogReciever() override;

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
    void logRecieve(QString who, QString event, QString params);
    void logConnect(QString who, QString event);
    void logDisconnect(QString who, QString event);
}


#endif //GUI_WALLET_LOG_H
