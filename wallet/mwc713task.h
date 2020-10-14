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
    QString shadowStr; // If defined, will print this string into the logs instead of the task output
};

// Some event utils

// Filter events by type
QVector< WEvent > filterEvents(const QVector<WEvent> & events, WALLET_EVENTS type );

// Print events into the string
QString printEvents(const QVector<WEvent> & events);

}

#endif // MWC713TASK_H
