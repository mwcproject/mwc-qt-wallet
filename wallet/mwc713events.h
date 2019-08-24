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

// Wallet event codes
enum WALLET_EVENTS {
    S_READY=1, // Wallet ready and waiting for intoput
    S_WELCOME=2, // Welcome string.                         Message: version
    S_INIT=3,    // Init choice request. Wallet need seed
    S_PASSWORD_ERROR=4, // Incorrect password error
    S_NEED2UNLOCK=5, // Need to unlock with password and optional account
    S_BOX_LISTENER=6, // BOX listener is up                 Message: address
    S_GENERIC_ERROR=7, // error printed by wallet           Message: error
    S_GENERIC_WARNING=8, // error printed by wallet           Message: error
    S_GENERIC_INFO=9, // error printed by wallet           Message: error
    S_ERROR         = 10, // error: keybase not fo...        Message: error
    S_YOUR_MWC_ADDRESS=11, // Your mwc address/Your mwcmq address:...  Message: address
    S_MWC_ADDRESS_INDEX = 12, // Derived with index [7]     Message: index

    S_INIT_WANT_ENTER=16, // Press ENTER when you have done so

    S_LINE=17,   // Just a line of output data as it is

    // Listening
    S_LISTENER_MQ_STARTING = 20, // starting mwcmq listener...
    S_LISTENER_KB_STARTING = 21, // starting keybase listener...
    S_LISTENER_ON = 22, // listener started for [XXXXXXX]       Message: address  or 'keybase'
    S_LISTENER_MQ_STOPPING = 24, // stopping mwcmq listener...
    S_LISTENER_KB_STOPPING = 25, // stopping keybase listener...
    S_LISTENER_OFF = 26, // listener [XXXXXXX] stopped.     Message address or keybase

    S_LISTENER_MQ_LOST_CONNECTION = 28, // WARNING: listener [XXXX] lost connection. it will...
    S_LISTENER_MQ_GET_CONNECTION = 29, // INFO: listener [XXXXX] reestablished connection.  Message: address

    // Recovery
    // from init wallet
    S_RECOVERY_STARTING = 30, // recovering... please wait as this could take a few minutes to complete
    S_RECOVERY_DONE = 31,     // wallet restoration done!
    S_RECOVERY_PROGRESS = 33, // Checking 1000 outputs, up to index 13433. (Highest index: 10235)   Message:  13433|10235
    // for fresh wallet
    S_RECOVERY_MNEMONIC = 34, // Recovering from mnemonic => Mnemonic:

    // Accounts
    S_ACCOUNTS_INFO_SUM = 41, // Account header info. The rest of data will come in lines...

    // Send
    S_SLATE_WAS_SENT = 55,
    S_SLATE_WAS_RECEIVED = 56,
    S_SLATE_WAS_FINALIZED = 57,

    S_SET_RECEIVE = 58,

    S_TRANSACTION_LOG = 60,
    S_OUTPUT_LOG = 61,


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
    Mwc713Task* task = nullptr; // task
    bool        wasProcessed = false;
    int         timeout = -1; // timeout for this task

    taskInfo() = default;
    taskInfo(Mwc713Task* _task, int _timeout) : task(_task), timeout(_timeout) {}
    taskInfo(const taskInfo&) = default;
    taskInfo & operator=(const taskInfo&) = default;
};

// Aggregator for Wallet events. Expected that there are not many events are aggregating.
// That is whay we are not maintaining any indexes.
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
    void addTask( Mwc713Task * task, int64_t timeout );

//    void addEvent(WALLET_EVENTS event) { events.push_back(WEvent(event)); }
  //  void addEvent(WALLET_EVENTS event, QString message) { events.push_back(WEvent(event, message)); }
//    void reset() { events.clear(); }

    const QVector<WEvent> & getEvents() const {return events;}

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

    // permanent tasks that allways active. They will process events one by one.
    // All input will come to them.
    // Example: checking for wallet become online/offline
    QVector< Mwc713Task* > listeners; // Owner of the tasks

    QMutex taskQMutex; // recursive
    QVector< taskInfo > taskQ; // Owner of the tasks

    // Events for a new task
    QVector<WEvent> events;

    volatile qint64 taskExecutionTimeLimit = 0; // Timeout value for the task
};

}

// Using in stots
Q_DECLARE_METATYPE(wallet::WALLET_EVENTS);


#endif // WALLETEVENTS_H
