#ifndef STATE_H
#define STATE_H

namespace core {
    class WalletWindow;
    class WindowManager;
    class MainWindow;
    class AppContext;
}

namespace wallet {
    class Wallet;
}

namespace state {

class State;
class StateMachine;

enum STATE {
    NONE,
    STATE_INIT, // first run. Creating the password for a wallet
    INPUT_PASSWORD, // Input password from the wallet
    NEW_WALLET,     // Select how to init the wallet (first run)
    GENERATE_NEW_SEED, // Init a new wallet, generate a new seed
    SHOW_NEW_SEED,  // Show the new seed
    TEST_NEW_SEED,  // Test a word from the seed
    CREATE_WITH_SEED,// Init wallet with exist seed
    ACCOUNTS,       // Wallet accounts
    EVENTS,         // Wallet events (logs)
    HODL,           // Hodl program.
    SEND_COINS,     // Send coins
    NODE_STATUS,    // Cgeck node status
    CONNECT_2_NODE, // connection to the MWC nodes
    NODE_MANUALY,   // input URI to the custom node
    FILE_TRANSACTIONS, // Transactions for files
    LISTENING,      // Listening API setting/status
    TRANSACTIONS    // Transactions dialog
};

struct NextStateRespond {
    enum RESULT { NONE, NEXT_STATE, WAIT_FOR_ACTION, DONE };

    NextStateRespond( RESULT res ) : result(res) {}
    NextStateRespond( RESULT res, STATE nextSt  ) : result(res), nextState(nextSt) {}

    RESULT result = RESULT::NONE;
    STATE nextState = STATE::NONE;
};


struct StateContext {
    core::AppContext    * const appContext;
    wallet::Wallet      * const wallet; //wallet caller interface
    core::WindowManager * const wndManager;
    core::MainWindow    * const mainWnd;
    StateMachine        * stateMachine;

    StateContext(core::AppContext * _appContext, wallet::Wallet * _wallet,
                 core::WindowManager * _wndManager, core::MainWindow * _mainWnd) :
        appContext(_appContext), wallet(_wallet), wndManager(_wndManager),
        mainWnd(_mainWnd), stateMachine(nullptr) {}

    void setStateMachine(StateMachine * sm) {stateMachine=sm;}
};

// Single state of the app that is described with Dialog
class State
{
protected:
    StateContext context;
    STATE stateId;

    //core::WalletWindow * wnd; //
    //State * prevState = nullptr;
    //State * nextState = nullptr;
public:
    State(const StateContext & context, STATE stateId);
    virtual ~State();

    // process/verify the state
    virtual NextStateRespond execute() = 0;
};

}

#endif // STATE_H
