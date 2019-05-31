#include "mwc713events.h"
#include "mwc713.h"
#include "../tries/mwc713inputparser.h"
#include "mwc713task.h"
#include <QDateTime>

namespace wallet {

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
}

// Process next task
void Mwc713EventManager::processNextTask() {
    if (taskQ.empty())
        return; // Nothing to process

    // Check if we can perform the first task
    taskInfo task = taskQ.front();

    if ( readyState ) {
        // can execute the task
        qDebug() << "Executing the task: " + task.task->toDbgString();
        readyState = false; // reset state first, then process

        if (!task.task->getInputStr().isEmpty()) {
            mwc713wallet->executeMwc713command(task.task->getInputStr());
            taskExecutionTimeLimit = QDateTime::currentMSecsSinceEpoch() + task.timeout;
        }
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
        evt.push_back(event);

        bool delEvent = false;
        for ( Mwc713Task* t : listeners ) {
            if (t->processTask(evt))
                delEvent = true;
        }

        if (delEvent) { // Delete - mean that we are done with processing.
            qDebug() << "Mwc713EventManager::sRecieveEvent was preprocessed. event=" << event << " msg='" << message << "'";
            return;
        }
    }


    switch (event) {
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
            events.push_back(WEvent(event, message));
            qDebug() << "Mwc713EventManager::sRecieveEvent adding Event into the list. event=" << event << " msg='"
                     << message << "'  New size:" << events.size();
        }
    }
}


}