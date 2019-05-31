#ifndef MWC713TASK_H
#define MWC713TASK_H

#include <QString>
#include "mwc713events.h"

namespace wallet {

class MWC713;
class WalletEventCollector;

// Base class of all tasks
class Mwc713Task
{
public:
    Mwc713Task(QString taskName, QString inputStr, MWC713 * wallet713);
    virtual ~Mwc713Task();

    const QString & getTaskName() const {return taskName;}

    // Will be called from 'Ready' for normal tasks
    // Or in order as events coming for filtering tasks
    // Return true if data was processed. In this case processed evenets will be dropped
    virtual bool processTask(const QVector<WEvent> & events ) = 0;

    // Check if task require input. Tasks with valid input can be run only
    // from 'ready' state
    bool hasInput() const {return !inputStr.isEmpty();}

    const QString & getInputStr() const {return inputStr;}

    virtual QString toDbgString() const { return "Mwc713Task(name="+taskName + ", Input="+inputStr+")"; }
protected:
    QString taskName;

    // wallet to call back regarding the state change
    MWC713 * wallet713;

    QString inputStr; // string (command) to feed to a wallet
};

// Some event utils

// Filter events by type
QVector< WEvent > filterEvents(const QVector<WEvent> & events, WALLET_EVENTS type );

// Print events into the string
QString printEvents(const QVector<WEvent> & events);

}

#endif // MWC713TASK_H
