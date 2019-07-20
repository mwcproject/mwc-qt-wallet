#include "mwc713events.h"
#include "mwc713.h"
#include "../tries/mwc713inputparser.h"
#include "mwc713task.h"
#include <QDateTime>
#include "../util/Log.h"

namespace wallet {


QString toString(WALLET_EVENTS event) {
    switch (event) {
        case S_READY:           return "S_READY";
        case S_WELCOME:         return "S_WELCOME";
        case S_INIT:            return "S_INIT";
        case S_PASSWORD_ERROR:  return "S_PASSWORD_ERROR";
        case S_NEED2UNLOCK:     return "S_NEED2UNLOCK";
        case S_BOX_LISTENER:    return "S_BOX_LISTENER";
        case S_GENERIC_ERROR:   return "S_GENERIC_ERROR";
        case S_YOUR_MWC_ADDRESS:return "S_YOUR_MWC_ADDRESS";
        case S_MWC_ADDRESS_INDEX:return "S_MWC_ADDRESS_INDEX";
        case S_PASSWORD_EXPECTED:return "S_PASSWORD_EXPECTED";
        case S_PASS_PHRASE:     return "S_PASS_PHRASE";
        case S_INIT_WANT_ENTER: return "S_INIT_WANT_ENTER";
        case S_ERROR:           return "S_ERROR";
        case S_LINE:            return "S_LINE";

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

        case S_GENERIC_WARNING: return "S_GENERIC_WARNING";
        case S_GENERIC_INFO:    return "S_GENERIC_INFO";

        case S_LISTENER_MQ_LOST_CONNECTION:  return "S_LISTENER_MQ_LOST_CONNECTION";
        case S_LISTENER_MQ_GET_CONNECTION: return "S_LISTENER_MQ_GET_CONNECTION";

        case S_ACCOUNTS_INFO_SUM: return "S_ACCOUNTS_INFO_SUM";

        case S_SLATE_WAS_SENT:      return "S_SLATE_WAS_SENT";
        case S_SLATE_WAS_RECEIVED:  return "S_SLATE_WAS_RECEIVED";
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
    for (auto t : listeners) {
        delete t;
    }
    listeners.clear();

    for (auto t : taskQ) {
        delete t.task;
    }
    taskQ.clear();
}

void Mwc713EventManager::connectWith(tries::Mwc713InputParser * inputParser) {
    const bool connected = connect(inputParser, &tries::Mwc713InputParser::sgGenericEvent, this, &wallet::Mwc713EventManager::slRecieveEvent,Qt::QueuedConnection );
    Q_ASSERT(connected);
    Q_UNUSED(connected);

    startTimer(500); // timeout checking. Twice a second is good enough for us
}

// Add task (single wallet action) to perform.
// This tale ownership of object
// Note:  if timeout <= 0, task will be executed immediately
void Mwc713EventManager::addTask( Mwc713Task * task, int64_t timeout ) {

    QMutexLocker l( &taskQMutex );

    // check if task is allready in the Q.
    bool found = false;
    for ( const taskInfo & ti : taskQ ) {
        if (ti.task->getTaskName() == task->getTaskName() && ti.task->getInputStr() == task->getInputStr()) {
            found = true;
            break;
        }
    }

    if (found) {
        // not execute the task, it is allready in the Q
        delete task;
        return;
    }

    taskQ.push_back(taskInfo(task,timeout));
    processNextTask();

    if (taskExecutionTimeLimit==0) {
         taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() + timeout;
    }
}

// Process next task
void Mwc713EventManager::processNextTask() {
    QMutexLocker l( &taskQMutex );

    if (taskQ.empty())
        return; // Nothing to process

    // Check if we can perform the first task
    taskInfo & task = taskQ.front();
    if (!task.wasProcessed) {

        events.clear();

        qDebug() << "Executing the task: " + task.task->toDbgString();
        task.wasProcessed = true; // reset state first, then process

        logger::logTask( "Mwc713EventManager", task.task, "Starting..." );
        task.task->onStarted();

        if (task.timeout > 0) {
            // schedule the task for execution
            if (!task.task->getInputStr().isEmpty()) {
                mwc713wallet->executeMwc713command(task.task->getInputStr(), task.task->getShadowStr());
            }
            taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() + task.timeout;
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

    if (QDateTime::currentMSecsSinceEpoch() > taskExecutionTimeLimit) {
        // report timeout error. Do it once
        taskExecutionTimeLimit = 0;
        taskInfo task = taskQ.front();
        mwc713wallet->appendNotificationMessage(  MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::TASK_TIMEOUT,
                "mwc713 unable to process the task '" + task.task->getTaskName() + "'" );
    }
}


// Events reciever
void Mwc713EventManager::slRecieveEvent( WALLET_EVENTS event, QString message) {

    // Preprocess event with listeners
    {
        QVector<WEvent> evt;
        evt.push_back(WEvent(event, message));

        for (Mwc713Task *t : listeners) {
            if (t->processTask(evt)) {
                qDebug() << "Mwc713EventManager::sRecieveEvent was preprocessed. event=" << event << " msg='" << message
                         << "'";
            }
        }
    }

    QMutexLocker l( &taskQMutex );

    if (taskQ.isEmpty())
        return;

    events.push_back(WEvent(event, message));
    qDebug() << "Mwc713EventManager::sRecieveEvent adding Event into the list. event=" << event << " msg='"
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

    task.task->processTask(events);
    events.clear();
    delete task.task;

    processNextTask();
}


}
