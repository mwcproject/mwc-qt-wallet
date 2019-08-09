#ifndef STATE_H
#define STATE_H

#include <QString>

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
    START_WALLET, // Start backed mwc713. Check what it want and then delegate control to the next state
    STATE_INIT, // first run. Creating the password for a wallet
    INPUT_PASSWORD, // Input password from the wallet
    ACCOUNTS,       // Wallet accounts
    ACCOUNT_TRANSFER, // Transfer funds from account to account
    EVENTS,         // Wallet events (logs)
    HODL,           // Hodl program.
    SEND,           // Send coins Page
    RECIEVE_COINS,  // Recieve coins
    LISTENING,      // Listening API setting/status
    TRANSACTIONS,   // Transactions dialog
    OUTPUTS,        // Outputs for this wallet
    CONTACTS,       // Contact page. COntacts supported by wallet713
    WALLET_CONFIG,  // Wallet config
    AIRDRDOP_MAIN,  // Starting airdrop page
    SHOW_SEED,      // Show Seed

    NODE_INFO,      // Show node info

    RESYNC          // Re-sync account with a node
};

struct NextStateRespond {
    enum RESULT { NONE, WAIT_FOR_ACTION, DONE };

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
    StateContext * context;
    STATE stateId;

    //core::WalletWindow * wnd; //
    //State * prevState = nullptr;
    //State * nextState = nullptr;
public:
    State(StateContext * context, STATE stateId);
    virtual ~State();

    state::StateContext * getContext() {return context;}

    void setWindowTitle( QString title );

    // process/verify the state
    virtual NextStateRespond execute() = 0;

    // Executing another state
    virtual void exitingState() {};

    // Empty string - default document name
    virtual QString getHelpDocName() {return "";}
};

}

#endif // STATE_H
