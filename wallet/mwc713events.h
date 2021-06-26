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

#ifndef WALLETEVENTS_H
#define WALLETEVENTS_H

#include <QString>
#include <QVector>
#include <QObject>
#include <QMutex>

namespace tries {
    class Mwc713InputParser;
}

namespace wallet {

class Mwc713Task;
class MWC713;

enum TASK_PRIORITY {
    TASK_IDLE = 1, // Task that are running in the background
    TASK_NORMAL = 2, // Normal priority
    TASK_NOW = 3,  // Quick response is expected
};

// Wallet event codes
enum WALLET_EVENTS {
    S_READY=1, // Wallet ready and waiting for intoput
    S_WELCOME=2, // Welcome string.                         Message: version
    S_INIT=3,    // Init choice request. Wallet need seed
    S_PASSWORD_ERROR=4, // Incorrect password error
    S_NEED2UNLOCK=5, // Need to unlock with password and optional account
    S_GENERIC_ERROR=7, // error printed by wallet           Message: error
    S_GENERIC_WARNING=8, // warning printed by wallet           Message: error
    S_GENERIC_INFO=9, // info printed by wallet           Message: error
    S_YOUR_MWC_ADDRESS=11, // Your mwc address/Your mwcmqs address:...  Message: address
    S_MWC_ADDRESS_INDEX = 12, // Derived with index [7]     Message: index
    S_NODE_API_ERROR = 13, // NODE api returns error

    S_INIT_WANT_ENTER=16, // Press ENTER when you have done so

    S_LINE=17,   // Just a line of output data as it is

    // Listening
    S_LISTENER_ON = 22, // listener started for [XXXXXXX]       Message: address.  For MWC MQSs second param is tid
    S_LISTENER_OFF = 26, // listener [XXXXXXX] stopped.     Message address

    S_LISTENER_MQ_LOST_CONNECTION = 28, // WARNING: listener [XXXX] lost connection. it will...
    S_LISTENER_MQ_GET_CONNECTION = 29, // INFO: listener [XXXXX] reestablished connection.  Message: address
    S_LISTENER_MQ_COLLISION = 30,  // ERROR: new login detected. mwcmqs listener will stop!
    S_LISTENER_MQ_FAILED_TO_START = 31, // ERROR: Failed to start mwcmqs subscriber. Error connecting to ....

    S_LISTENER_HTTP_STARTING = 35, // Starting listener for Foreign API on [host:port]
    S_LISTENER_HTTP_FAILED   = 36, // thread 'foreign-api-gotham' panicked at XXXXXXXXX

    S_LISTENER_TOR_LOST_CONNECTION = 37, // WARNING: Tor is not responding. Will try to reconnect
    S_LISTENER_TOR_GET_CONNECTION = 38, // INFO: Tor connection reestablished

    // Recovery
    // from init wallet
    S_RECOVERY_STARTING = 45, // recovering... please wait as this could take a few minutes to complete
    S_RECOVERY_DONE     = 46,     // wallet restoration done!
    S_RECOVERY_PROGRESS = 47, // Checking 1000 outputs, up to index 13433. (Highest index: 10235)   Message:  13433|10235
    // for fresh wallet
    S_RECOVERY_MNEMONIC = 48, // Recovering from mnemonic => Mnemonic:

    S_SYNC_PROGRESS = 49, // Checking 1000 outputs, up to index 13433. (Highest index: 10235)   Message:  13433|10235

    // Accounts
    S_ACCOUNTS_INFO_SUM = 54, // Account header info. The rest of data will come in lines...

    S_SLATE_WAS_RECEIVED_FROM = 68,

    S_SET_RECEIVE = 75,

    S_TRANSACTION_LOG = 80,
    S_OUTPUT_LOG = 81,

    // Swaps
    S_SWAP_GET_OFFER = 90,
    S_SWAP_GET_MESSAGE = 91,

    // Swap Marketplace
    S_MKT_ACCEPT_OFFER = 95,
    S_MKT_FAIL_BIDDING = 96,
    S_MKT_WINNINER     = 97, // winner in the group. It is mean that offer is really taken and trade is started.
};
QString toString(WALLET_EVENTS event);

// Timeout values for the Tasks
const int TASK_STARTING_TO = 5000;
const int TASK_UNLOCK_TO = 3000;

struct WEvent {
    WALLET_EVENTS event;
    QString message;

    WEvent(WALLET_EVENTS _event) : event(_event) {}
    WEvent(WALLET_EVENTS _event, QString _message) : event(_event), message(_message) {}

    WEvent() = default;
    WEvent(const WEvent &) = default;
    WEvent & operator = (const WEvent &) = default;
};

struct taskInfo {
    int         groupId = -1;
    TASK_PRIORITY priority = TASK_PRIORITY::TASK_NORMAL;
    Mwc713Task* task = nullptr; // task
    bool        wasStarted   = false;
    int         timeout = -1; // timeout for this task

    taskInfo() = default;
    taskInfo(int _groupId, TASK_PRIORITY _priority, Mwc713Task* _task, int _timeout) : groupId(_groupId), priority(_priority), task(_task), timeout(_timeout) {}
    taskInfo(const taskInfo&) = default;
    taskInfo & operator=(const taskInfo&) = default;
};

// Aggregator for Wallet events. Expected that there are not many events are aggregating.
// That is why we are not maintaining any indexes.
class Mwc713EventManager : public QObject
{
    Q_OBJECT
public:
    Mwc713EventManager(MWC713 * mwc713wallet);
    virtual ~Mwc713EventManager() override;
    Mwc713EventManager(const Mwc713EventManager & ) = delete;
    Mwc713EventManager & operator=(const Mwc713EventManager & ) = delete;

    void connectWith(tries::Mwc713InputParser * inputParser);

    void addListener(Mwc713Task* task) { listeners.push_back(task);}

    // Add task (single wallet action) to perform.
    // This tale ownership of object
    // Note:  if timeout <= 0, task will be executed immediately
    //   idx == -1 - push_back, otherwise will insert into the index position
    // Return: true if task was added.  False - was ignored
    void addTask( TASK_PRIORITY priority, QVector< QPair<Mwc713Task*, int64_t>> tasks, int idx = -1);

    // Check if task already exist
    bool hasTask(Mwc713Task * task);

    const QVector<WEvent> & getEvents() const {return events;}

    // Cancelling all tasks except the current one. Return timeout valiue that needed to wait
    int cancelTasksInQueue();

    // clean all tasks, events and all
    void clear();
public slots:
    void slReceiveEvent( WALLET_EVENTS event, QString message); // message is optional

private:
    // timer that we are using for timeouts
    virtual void timerEvent(QTimerEvent *event) override;

    // Process next task
    void processNextTask();

    // Execute this task and start the next one
    void executeTask(taskInfo task);

private:
    // Wallet
    MWC713 * mwc713wallet = nullptr;

    // permanent tasks that always active. They will process events one by one.
    // All input will come to them.
    // Example: checking for wallet become online/offline
    QVector< Mwc713Task* > listeners; // Owner of the tasks

    static QMutex taskQMutex; // recursive
    QVector< taskInfo > taskQ; // Owner of the tasks
    int groupId = 0;

    // Events for a new task
    QVector<WEvent> events;

    volatile qint64 taskExecutionTimeLimit = 0; // Timeout value for the task

    QString lastWalletProgressCommand;
};

}

// Using in stots
Q_DECLARE_METATYPE(wallet::WALLET_EVENTS);


#endif // WALLETEVENTS_H
