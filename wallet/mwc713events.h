#ifndef WALLETEVENTS_H
#define WALLETEVENTS_H

#include <QString>
#include <QVector>
#include <QObject>

namespace tries {
    class Mwc713InputParser;
}

namespace wallet {

class Mwc713Task;
class MWC713;

// Wallet event codes
enum WALLET_EVENTS {
    S_READY=1, // Wallet ready and waiting for intoput
    S_WELCOME=2, // Welcome string.                       Message: version
    S_INIT=3,    // Init choice request. Wallet need seed
    S_PASSWORD_ERROR=4, // Incorrect password error
    S_NEED2UNLOCK=5, // Need to unlock with password and optional account
    S_BOX_LISTENER=6, // BOX listenet is up               Message: address
    S_GENERIC_ERROR=7, // error printed by wallet         Message: error
    S_YOUR_MWC_ADDRESS=8, // Respond to Unlock with box address   Message: address
};

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
    void addTask( Mwc713Task * task, long timeout );

//    void addEvent(WALLET_EVENTS event) { events.push_back(WEvent(event)); }
  //  void addEvent(WALLET_EVENTS event, QString message) { events.push_back(WEvent(event, message)); }
//    void reset() { events.clear(); }

    const QVector<WEvent> & getEvents() const {return events;}

public slots:
    void slRecieveEvent( WALLET_EVENTS event, QString message); // message is optional

private:
    // timer that we are using for timeouts
    virtual void timerEvent(QTimerEvent *event) override;

    // Process next task
    void processNextTask();

private:
    // Wallet
    MWC713 * mwc713wallet = nullptr;

    // permanent tasks that allways active. They will process events one by one.
    // All input will come to them.
    // Example: checking for wallet become online/offline
    QVector< Mwc713Task* > listeners; // Owner of the tasks

    QVector< taskInfo > taskQ; // Owner of the tasks

    // Events for a new task
    QVector<WEvent> events;

    bool readyState = false; // Can start a new task only from ready state
    qint64 taskExecutionTimeLimit = 0; // Timeout valur for the task
};

}

// Using in stots
Q_DECLARE_METATYPE(wallet::WALLET_EVENTS);


#endif // WALLETEVENTS_H
