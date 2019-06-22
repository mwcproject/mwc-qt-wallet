#ifndef MWC713TASK_H
#define MWC713TASK_H

#include <QString>
#include "mwc713events.h"
#include <QSet>

namespace wallet {

class MWC713;
class WalletEventCollector;

// Base class of all tasks
class Mwc713Task
{
public:
    // shadowStr - specify if data in this task is private. That string will be used into the logs.
    Mwc713Task(QString taskName, QString inputStr, MWC713 * wallet713, QString shadowStr);
    virtual ~Mwc713Task();

    const QString & getTaskName() const {return taskName;}

    virtual QSet<WALLET_EVENTS> getReadyEvents() = 0; // Set of final events that can trigger task execution and completion

    virtual void onStarted() {}

    // Will be called from 'Ready' for normal tasks
    // Or in order as events coming for filtering tasks
    // Return true if data was processed. In this case processed evenets will be dropped
    virtual bool processTask(const QVector<WEvent> & events ) = 0;

    // Check if task require input. Tasks with valid input can be run only
    // from 'ready' state
    bool hasInput() const {return !inputStr.isEmpty();}

    const QString & getInputStr() const {return inputStr;}
    const QString & getShadowStr() const {return shadowStr;}

    virtual QString toDbgString() const {
        if (shadowStr.isEmpty())
            return "Mwc713Task(name="+taskName + (inputStr.isEmpty()?"":", Input="+inputStr)+")";
        else
            return "Mwc713Task("+shadowStr+")";
    }

protected:
    QString taskName;

    // wallet to call back regarding the state change
    MWC713 * wallet713;
    QString inputStr; // string (command) to feed to a wallet
    QString shadowStr; // If difined, will represend this task into the logs
};

// Some event utils

// Filter events by type
QVector< WEvent > filterEvents(const QVector<WEvent> & events, WALLET_EVENTS type );

// Print events into the string
QString printEvents(const QVector<WEvent> & events);

}

#endif // MWC713TASK_H
