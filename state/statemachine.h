#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <QMap>
#include "state.h"

namespace state {

// State machine that describes wallet application
class StateMachine
{
public:
    StateMachine(StateContext & context);
    ~StateMachine();

    void start();

    void executeFrom( STATE nextState );

    // set current action winodow if possible
    bool setActionWindow( STATE actionWindowState );
    // get current action window
    STATE getActionWindow() const;

    // return true if action window will applicable
    bool isActionWindowMode() const;
private:
    // routine to process state into the loop
    bool processState(State* st);

private:
    core::AppContext * const appContext;
    core::MainWindow * const mainWindow;

    // Map is orders by Ids. It naturally define the priority of
    // all states
    QMap< STATE, State* > states;
    STATE currentState = STATE::NONE;
};


}

#endif // STATEMACHINE_H
