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
        case S_INIT_IN_PROGRESS:return "S_INIT_IN_PROGRESS";
        case S_PASSWORD_EXPECTED:return "S_PASSWORD_EXPECTED";
        case S_PASS_PHRASE:     return "S_PASS_PHRASE";
        case S_INIT_WANT_ENTER: return "S_INIT_WANT_ENTER";
        case S_ERROR:           return "S_ERROR";
        case S_LISTENER_MQ_STARTING: return "S_LISTENER_MQ_STARTING";
        case S_LISTENER_KB_STARTING: return "S_LISTENER_KB_STARTING";
        case S_LISTENER_MQ_ON:  return "S_LISTENER_MQ_ON";
        case S_LISTENER_KB_ON:  return "S_LISTENER_KB_ON";
        case S_LISTENER_MQ_STOPPING: return "S_LISTENER_MQ_STOPPING";
        case S_LISTENER_KB_STOPPING: return "S_LISTENER_KB_STOPPING";
        case S_LISTENER_MQ_OFF: return "S_LISTENER_MQ_OFF";
        case S_LISTENER_KB_OFF: return "S_LISTENER_KB_OFF";
        case S_RECOVERY_STARTING: return "S_RECOVERY_STARTING";
        case S_RECOVERY_DONE:   return "S_RECOVERY_DONE";
        case S_RECOVERY_PROGRESS: return "S_RECOVERY_PROGRESS";
        case S_RECOVERY_MNEMONIC: return "S_RECOVERY_MNEMONIC";

        case S_GENERIC_WARNING: return "S_GENERIC_WARNING";
        case S_GENERIC_INFO:    return "S_GENERIC_INFO";

        case S_LISTENER_MQ_LOST_CONNECTION:  return "S_LISTENER_MQ_LOST_CONNECTION";
        case S_LISTENER_MQ_GET_CONNECTION: return "S_LISTENER_MQ_GET_CONNECTION";

        default: Q_ASSERT(false); return "Unknown";
    }
}


Mwc713EventManager::Mwc713EventManager(MWC713 * _mwc713wallet) : mwc713wallet(_mwc713wallet)
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
void Mwc713EventManager::addTask( Mwc713Task * task, long timeout ) {
    taskQ.push_back(taskInfo(task,timeout));
    processNextTask();

    if (taskExecutionTimeLimit==0) {
         taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() + timeout;
    }
}

// Process next task
void Mwc713EventManager::processNextTask() {
    if (taskQ.empty())
        return; // Nothing to process

    // Check if we can perform the first task
    taskInfo task = taskQ.front();

    if ( readyState ) {
        // can execute the task

        // Clearning events first. Command normally process it's own massages, not what we have in buffer
        events.clear();

        qDebug() << "Executing the task: " + task.task->toDbgString();
        readyState = false; // reset state first, then process

        log::logTask( "Mwc713EventManager", task.task, "Starting..." );

        if (!task.task->getInputStr().isEmpty()) {
            mwc713wallet->executeMwc713command(task.task->getInputStr());
        }
        taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() + task.timeout;
    }
}

void Mwc713EventManager::timerEvent(QTimerEvent *event) {
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
        evt.push_back( WEvent(event,message) );

        for ( Mwc713Task* t : listeners ) {
            if (t->processTask(evt)) {
                qDebug() << "Mwc713EventManager::sRecieveEvent was preprocessed. event=" << event << " msg='" << message << "'";
            }
        }
    }

    events.push_back(WEvent(event, message));
    qDebug() << "Mwc713EventManager::sRecieveEvent adding Event into the list. event=" << event << " msg='"
             << message << "'  New size:" << events.size();

    switch (event) {
        case S_RECOVERY_MNEMONIC:
        case S_INIT_WANT_ENTER: // Almost like ready
        case S_PASSWORD_EXPECTED: // Almost like ready
        case S_INIT: // Init is not followed by READ. So it is a ready state as well
        case S_READY: // Wallet ready and waiting for input
        {
            // We can prccess the current task
            if (!readyState) {
                taskExecutionTimeLimit = 0; // stopping timeout
                if (taskQ.empty()) {
                    qDebug() << "INTERNAL ERROR readyState at ready without a task. Nothing to execute";
                    taskExecutionTimeLimit = 0;
                    Q_ASSERT(false);
                    return;
                }
                taskInfo task = taskQ.takeFirst();
                qDebug() << "Mwc713EventManager::sRecieveEvent S_READY. Processing task '" << task.task->getTaskName()
                         << "'";

                log::logTask( "Mwc713EventManager", task.task, "Executing" );

                task.task->processTask(events);
                events.clear();
                delete task.task;
            }
            else {
                qDebug() << "Mwc713EventManager::sRecieveEvent S_READY. No task to process";
            }
            readyState = true;
            // can process next task if it is in the Q
            processNextTask();

            break;
        }
        default: {
        }
    }


    if (event == WALLET_EVENTS::S_INIT) // Init is not followed by READ. So it is a ready state as well
    {
        return slRecieveEvent( WALLET_EVENTS::S_READY , "");
    }


}


}
