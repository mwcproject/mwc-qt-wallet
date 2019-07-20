#ifndef MWC_QT_WALLET_TIMEOUTLOCK_H
#define MWC_QT_WALLET_TIMEOUTLOCK_H

namespace state {

class StateMachine;
class State;

// Expected to be created on the stack and prevent locking by timeout
class TimeoutLockObject {
public:
    TimeoutLockObject( State * state );
    TimeoutLockObject( StateMachine * sm );
    ~TimeoutLockObject();
private:
    StateMachine * sm = nullptr;
};


}



#endif //MWC_QT_WALLET_TIMEOUTLOCK_H
