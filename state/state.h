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

#ifndef STATE_H
#define STATE_H

#include <QString>

namespace core {
    class WindowManager;
    class MainWindow;
    class AppContext;
    class HodlStatus;
}

namespace wallet {
    class Wallet;
}

namespace node {
class MwcNode;
}

namespace state {

class State;
class StateMachine;

enum STATE {
    NONE = 0,
    START_WALLET = 1,           // Start backed mwc713. Check what it want and then delegate control to the next state
    STATE_INIT = 2,             // first run. Creating the password for a wallet
    INPUT_PASSWORD = 3,         // Input password from the wallet
    ACCOUNTS = 4,               // Wallet accounts.              Note!!!  Keep accounts first Action page.
    MIGRATION = 5,              // Data migration between wallet versions. Note!! Needs to be before Transaction page.
    ACCOUNT_TRANSFER= 6,        // Transfer funds from account to account
    EVENTS = 7,                 // Wallet events (logs)
    HODL = 8,                   // Hodl program.
    SEND = 9,                   // Send coins Page
    RECEIVE_COINS = 10,         // Receive coins
    LISTENING = 11,             // Listening API setting/status
    TRANSACTIONS = 12,          // Transactions dialog
    OUTPUTS = 13,               // Outputs for this wallet
    CONTACTS = 14,              // Contact page. COntacts supported by wallet713
    WALLET_CONFIG = 15,         // Wallet config
    AIRDRDOP_MAIN = 16,         // Starting airdrop page
    SHOW_SEED = 17,             // Show Seed
    NODE_INFO = 18,             // Show node info
    RESYNC = 19,                // Re-sync account with a node
    FINALIZE = 20,              // Finalize transaction. Windowless state
    WALLET_RUNNING_MODE = 21,   // Running mode as a node, wallet or cold wallet

    // Mobile Specific Pages
    WALLET_HOME = 22,           // Wallet home page
    ACCOUNT_OPTIONS = 23,       // Account options page
    WALLET_SETTINGS = 24        // Settings page
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
    node::MwcNode       * const mwcNode;
    StateMachine        * stateMachine = nullptr;
    core::HodlStatus    * hodlStatus = nullptr;

    StateContext(core::AppContext * _appContext, wallet::Wallet * _wallet,
                 node::MwcNode * _mwcNode) :
        appContext(_appContext), wallet(_wallet), mwcNode(_mwcNode), stateMachine(nullptr) {}

    void setStateMachine(StateMachine * sm) {stateMachine=sm;}
    void setHodlStatus(core::HodlStatus* hs) {hodlStatus=hs;}
};

void setStateContext(StateContext * context);
StateContext * getStateContext();
// Shortcut to state if state_machine. Callir suppose to know the class name
State * getState(STATE state);

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

    // process/verify the state
    virtual NextStateRespond execute() = 0;

    // State can block the stare change. Wallet config is the first usage.
    virtual bool canExitState(STATE nextWindowState) { Q_UNUSED(nextWindowState) return true;}

    // Executing another state
    virtual void exitingState() {};

    // Empty string - default document name
    virtual QString getHelpDocName() {return "";}
};

}

#endif // STATE_H
