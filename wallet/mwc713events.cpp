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

#include "mwc713events.h"
#include "mwc713.h"
#include "../tries/mwc713inputparser.h"
#include "mwc713task.h"
#include <QDateTime>
#include "../util/Log.h"
#include "../core/Config.h"
#include "../core/Notification.h"
#include "../control/messagebox.h"

namespace wallet {


QString toString(WALLET_EVENTS event) {
    switch (event) {
        case S_READY:           return "S_READY";
        case S_WELCOME:         return "S_WELCOME";
        case S_INIT:            return "S_INIT";
        case S_PASSWORD_ERROR:  return "S_PASSWORD_ERROR";
        case S_NEED2UNLOCK:     return "S_NEED2UNLOCK";
        case S_GENERIC_ERROR:   return "S_GENERIC_ERROR";
        case S_YOUR_MWC_ADDRESS:return "S_YOUR_MWC_ADDRESS";
        case S_MWC_ADDRESS_INDEX:return "S_MWC_ADDRESS_INDEX";
        case S_INIT_WANT_ENTER: return "S_INIT_WANT_ENTER";
        case S_ERROR:           return "S_ERROR";
        case S_LINE:            return "S_LINE";
        case S_NODE_API_ERROR:  return "S_NODE_ERROR";

        case S_LISTENER_MQ_STARTING: return "S_LISTENER_MQ_STARTING";
        case S_LISTENER_KB_STARTING: return "S_LISTENER_KB_STARTING";
        case S_LISTENER_ON:  return "S_LISTENER_ON";
        case S_LISTENER_MQ_STOPPING: return "S_LISTENER_MQ_STOPPING";
        case S_LISTENER_KB_STOPPING: return "S_LISTENER_KB_STOPPING";
        case S_LISTENER_OFF: return "S_LISTENER_OFF";
        case S_RECOVERY_STARTING: return "S_RECOVERY_STARTING";
        case S_RECOVERY_DONE:   return "S_RECOVERY_DONE";
        case S_RECOVERY_PROGRESS: return "S_RECOVERY_PROGRESS";
        case S_RECOVERY_MNEMONIC: return "S_RECOVERY_MNEMONIC";

        case S_SYNC_PROGRESS:     return "S_SYNC_PROGRESS";

        case S_GENERIC_WARNING: return "S_GENERIC_WARNING";
        case S_GENERIC_INFO:    return "S_GENERIC_INFO";

        case S_LISTENER_MQ_LOST_CONNECTION:  return "S_LISTENER_MQ_LOST_CONNECTION";
        case S_LISTENER_MQ_GET_CONNECTION:   return "S_LISTENER_MQ_GET_CONNECTION";
        case S_LISTENER_MQ_COLLISION:        return "S_LISTENER_MQ_COLLISION";
        case S_LISTENER_MQ_FAILED_TO_START:  return "S_LISTENER_MQ_FAILED_TO_START";

        case S_LISTENER_KB_LOST_CONNECTION:  return "S_LISTENER_KB_LOST_CONNECTION";
        case S_LISTENER_KB_GET_CONNECTION:   return "S_LISTENER_KB_GET_CONNECTION";

        case S_LISTENER_HTTP_STARTING:       return "S_LISTENER_HTTP_STARTING";
        case S_LISTENER_HTTP_FAILED:         return "S_LISTENER_HTTP_FAILED";

        case S_ACCOUNTS_INFO_SUM: return "S_ACCOUNTS_INFO_SUM";

        case S_SLATE_WAS_SENT_TO:    return "S_SLATE_WAS_SENT_TO";
        case S_SLATE_WAS_SENT_BACK: return "S_SLATE_WAS_SENT_BACK";
        case S_SLATE_WAS_RECEIVED_BACK: return "S_SLATE_WAS_RECEIVED_BACK";
        case S_SLATE_WAS_RECEIVED_FROM: return "S_SLATE_WAS_RECEIVED_FROM";
        case S_SLATE_WAS_FINALIZED: return "S_SLATE_WAS_FINALIZED";
        case S_SET_RECEIVE:         return "S_SET_RECEIVE";

        case S_TRANSACTION_LOG:     return "S_TRANSACTION_LOG";
        case S_OUTPUT_LOG:          return "S_OUTPUT_LOG";

        default: Q_ASSERT(false); return "Unknown";
    }
}


Mwc713EventManager::Mwc713EventManager(MWC713 * _mwc713wallet) : mwc713wallet(_mwc713wallet) , taskQMutex(QMutex::Recursive)
{
}

Mwc713EventManager::~Mwc713EventManager() {
    clear();
}

void Mwc713EventManager::clear() {
    for (auto t : listeners) {
        delete t;
    }
    listeners.clear();

    for (auto t : taskQ) {
        delete t.task;
    }
    taskQ.clear();
    events.clear();
    taskExecutionTimeLimit = 0;
}

void Mwc713EventManager::connectWith(tries::Mwc713InputParser * inputParser) {
    const bool connected = connect(inputParser, &tries::Mwc713InputParser::sgGenericEvent, this, &wallet::Mwc713EventManager::slReceiveEvent,Qt::QueuedConnection );
    Q_ASSERT(connected);
    Q_UNUSED(connected);

    startTimer(500); // timeout checking. Twice a second is good enough for us
}

// Check if task already exist
bool Mwc713EventManager::hasTask(Mwc713Task * task) {

    QMutexLocker l( &taskQMutex );

    bool found = false;
    for ( const taskInfo & ti : taskQ ) {
            if (ti.task->getTaskName() == task->getTaskName() && ti.task->getInputStr() == task->getInputStr()) {
                found = true;
                break;
            }
    }
    return found;
}


// Add task (single wallet action) to perform.
// This tale ownership of object
// Note:  if timeout <= 0, task will be executed immediately
// Return: true if task was added.  False - was ignored
void Mwc713EventManager::addTask( Mwc713Task * task, int64_t timeout ) {

    QMutexLocker l( &taskQMutex );

    // timeout multiplier will be applyed to the task because we want apply this value as late as posiible.
    // User might change it at any moment.
    taskQ.push_back(taskInfo(task,timeout));
    processNextTask();

    if (taskExecutionTimeLimit==0) {
         taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() + (int64_t)(timeout * config::getTimeoutMultiplier());
    }
}

bool Mwc713EventManager::addFirstTask( Mwc713Task * task, int64_t timeout) {
    QMutexLocker l( &taskQMutex );

    // We can push front a new tast only if current one is not running.
    Q_ASSERT( taskQ.isEmpty() || !taskQ.front().wasStarted );
    taskQ.insert(0,taskInfo(task,timeout));

    return true;
}

// Running and waiting tasks number
int Mwc713EventManager::cancelTasksInQueue() {
    QMutexLocker l( &taskQMutex );

    if (taskQ.isEmpty()) {
        return 0;
    }

    taskQ.resize(1);
    return taskQ[0].timeout;
}


// Process next task
void Mwc713EventManager::processNextTask() {
    QMutexLocker l( &taskQMutex );

    if (taskQ.empty())
        return; // Nothing to process

    // Check if we can perform the first task
    taskInfo & task = taskQ.front();
    if (!task.wasStarted) {

        events.clear();

        qDebug() << "Executing the task: " + task.task->toDbgString();
        task.wasStarted = true; // reset state first, then process
        taskExecutionTimeLimit = 0;

        QStringList taskList;
        for (const auto & t : taskQ) {
            taskList.push_back(t.task->toDbgString());
        }
        logger::logInfo( "Mwc713EventManager", "Task queue: " + taskList.join(", ") );

        logger::logTask( "Mwc713EventManager", task.task, "Starting..." );
        task.task->onStarted();

        if (task.timeout > 0) {
            // schedule the task for execution
            if (!task.task->getInputStr().isEmpty()) {
                mwc713wallet->executeMwc713command(task.task->getInputStr(), task.task->getShadowStr());
            }
            taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() +  (int64_t)(task.timeout * config::getTimeoutMultiplier());
        }
        else {
            // execute the task now. Next task will be started
            executeTask(taskQ.takeFirst());
        }
    }
}

void Mwc713EventManager::timerEvent(QTimerEvent *event) {
    QMutexLocker l( &taskQMutex );

    Q_UNUSED(event);
    if (taskExecutionTimeLimit==0)
        return;

    if (taskQ.empty()) {
        qDebug() << "INTERNAL ERROR Found timeout without a task. Ignoring the timeout";
        taskExecutionTimeLimit = 0;
        Q_ASSERT(false);
        return;
    }

    /*if (QDateTime::currentMSecsSinceEpoch() > taskExecutionTimeLimit/2) {
        mwc713wallet->executeMwc713command("");
    }*/

    QString taskName = taskQ.front().task->getTaskName();

    if (QDateTime::currentMSecsSinceEpoch() > taskExecutionTimeLimit) {
        if (control::MessageBox::questionText(nullptr, "Warning", "mwc713 command execution is taking longer than expected.\nContinue to wait?",
                                          "Yes", "No", true, false) == control::MessageBox::RETURN_CODE::BTN1) {
            config::increaseTimeoutMultiplier();
            // Update the waiting time

            // Note, here we might already have another task.
            if (!taskQ.isEmpty())
                taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() +  (int64_t)(taskQ.front().timeout * config::getTimeoutMultiplier());
            return;
        }

        // report timeout error. Do it once
        taskExecutionTimeLimit = 0;
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR,
                "mwc713 unable to process the task '" + taskName + "'" );
    }
}


// Events receiver
void Mwc713EventManager::slReceiveEvent( WALLET_EVENTS event, QString message) {

    // Preprocess event with listeners
    {
        QVector<WEvent> evt;
        evt.push_back(WEvent(event, message));

        for (Mwc713Task *t : listeners) {
            if (t->processTask(evt)) {
                qDebug() << "Mwc713EventManager::sReceiveEvent was preprocessed. event=" << event << " msg='" << message
                         << "'";
            }
        }
    }

    QMutexLocker l( &taskQMutex );

    if (taskQ.isEmpty())
        return;

    events.push_back(WEvent(event, message));
    qDebug() << "Mwc713EventManager::sReceiveEvent adding Event into the list. event=" << event << " msg='"
             << message << "'  New size:" << events.size();


    if (!taskQ.front().task->getReadyEvents().contains(event))
        return; // still waiting for events

    executeTask(taskQ.takeFirst());
}

void Mwc713EventManager::executeTask(taskInfo task) {
    // Got the acceptable final event
    taskExecutionTimeLimit = 0; // stopping timeout
    qDebug() << "Processing task '" << task.task->getTaskName() << "'";

    logger::logTask("Mwc713EventManager", task.task, "Executing");

    // Reset before processing because processing might tale some time
    QVector<WEvent> evts(events);
    events.clear();

    task.task->processTask(evts);
    delete task.task;

    processNextTask();
}


}
