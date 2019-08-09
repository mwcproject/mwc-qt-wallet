#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QMap>
#include "state.h"
#include <QObject>

namespace state {

// State machine that describes wallet application
class StateMachine : public QObject
{
    Q_OBJECT
public:
    StateMachine(StateContext * context);
    ~StateMachine();

    void start();

    void executeFrom( STATE nextState );

    // set current action winodow if possible
    bool setActionWindow( STATE actionWindowState, bool enforce = false );
    // get current action window
    STATE getActionWindow() const;

    // return true if action window will applicable
    bool isActionWindowMode() const;

    // Reset logout time.
    void resetLogoutLimit();


    // Logout must be blocked for modal dialogs
    void blockLogout();
    void unblockLogout();

    // Please use carefully, don't abuse this interface since no type control can be done
    State* getState(STATE state) const;

    State* getCurrentState() const;

private:
    // routine to process state into the loop
    bool processState(State* st);

    virtual void timerEvent(QTimerEvent *event) override;

private:
    StateContext * context = nullptr;

    // Map is orders by Ids. It naturally define the priority of
    // all states
    QMap< STATE, State* > states;
    STATE currentState = STATE::NONE;

    int64_t logoutTime = 0; // 0 mean never logout...
    int     blockLogoutCounter = 0;
};


}

#endif // STATEMACHINE_H
