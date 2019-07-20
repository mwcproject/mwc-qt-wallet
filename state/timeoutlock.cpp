#include "timeoutlock.h"
#include "statemachine.h"
#include "state.h"


namespace state {

TimeoutLockObject::TimeoutLockObject( State * state ) {
    Q_ASSERT(state);
    sm = state->getContext()->stateMachine;
    sm->blockLogout();
}

TimeoutLockObject::TimeoutLockObject( StateMachine * _sm ) : sm(_sm) {
    Q_ASSERT(sm);
    sm->blockLogout();
}

TimeoutLockObject::~TimeoutLockObject() {
    sm->unblockLogout();
}


}