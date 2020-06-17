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

#include "statemachine.h"
#include "a_initaccount.h"
#include "a_inputpassword.h"
#include "../core/appcontext.h"
#include "k_accounts.h"
#include "k_AccountTransfer.h"
#include "x_events.h"
#include "h_hodl.h"
#include "e_Receive.h"
#include "e_listening.h"
#include "e_transactions.h"
#include "e_outputs.h"
#include "w_contacts.h"
#include "x_walletconfig.h"
#include "m_airdrop.h"
#include "a_StartWallet.h"
#include "g_Send.h"
#include "x_ShowSeed.h"
#include "x_Resync.h"
#include "../core/Config.h"
#include "u_nodeinfo.h"
#include "g_Finalize.h"
#include "y_selectmode.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/corewindow_b.h"
#include "../core/WndManager.h"
#include "x_migration.h"
#include "z_wallethome.h"
#include "z_walletsettings.h"
#include "z_accountoptions.h"


namespace state {

static StateMachine * stateMachine = nullptr;

void StateMachine::initStateMachine() {
    Q_ASSERT(stateMachine== nullptr);
    stateMachine = new StateMachine();
}
void StateMachine::destroyStateMachine() {
    Q_ASSERT(stateMachine);
    delete stateMachine;
    stateMachine = nullptr;
}
StateMachine * getStateMachine() {
    return stateMachine;
}


StateMachine::StateMachine()
{
    StateContext * context = getStateContext();
    Q_ASSERT(context);
    context->setStateMachine(this);

    // Those states are mandatory because that manage wallet lifecycle
    states[ STATE::START_WALLET ]   = new StartWallet(context);
    states[ STATE::STATE_INIT ]     = new InitAccount(context);
    states[ STATE::INPUT_PASSWORD ] = new InputPassword(context);

    // Pages

    if (config::isOnlineWallet() || config::isColdWallet()) {
        states[ STATE::ACCOUNTS ]       = new Accounts(context);
        states[ STATE::ACCOUNT_TRANSFER ] = new AccountTransfer(context);
        states[ STATE::SEND ]           = new Send(context);
        states[ STATE::RECEIVE_COINS ]  = new Receive(context);
        states[ STATE::TRANSACTIONS ]   = new Transactions(context);
        states[ STATE::OUTPUTS ]        = new Outputs(context);
        states[ STATE::CONTACTS ]       = new Contacts(context);
        states[ STATE::SHOW_SEED ]      = new ShowSeed(context);
        states[ STATE::RESYNC ]         = new Resync(context);
        states[ STATE::FINALIZE ]       = new Finalize(context);
    }
    if (config::isOnlineWallet() ) {
        states[ STATE::LISTENING ]      = new Listening(context);
        states[ STATE::AIRDRDOP_MAIN ]  = new Airdrop(context);
    }

    states[ STATE::HODL ]           = new Hodl(context);
    states[ STATE::EVENTS ]         = new Events(context);
    states[ STATE::WALLET_CONFIG ]  = new WalletConfig(context);
    states[ STATE::NODE_INFO ]      = new NodeInfo(context);

    states[ WALLET_RUNNING_MODE ]   = new SelectMode(context);

    // Mobile specfic states
    states[ STATE::WALLET_HOME ] = new WalletHome(context);
    states[ STATE::WALLET_SETTINGS ] = new WalletSettings(context);
    states[ STATE::ACCOUNT_OPTIONS ] = new AccountOptions(context);

    // State for handling any data migration between wallet
    // versions that might need to be done
    states[ STATE::MIGRATION ] = new Migration(context);

    startTimer(1000);
}

StateMachine::~StateMachine() {
    for (auto st : states)
        delete st;

    states.clear();
}

// Please use carefully, don't abuse this interface since no type control can be done
State * StateMachine::getState(STATE state) const {
    return states.value(state, nullptr);
}

State* StateMachine::getCurrentStateObj() const {
    return getState(currentState);
}


void StateMachine::start() {

    // Init the app
    for ( auto it = states.begin(); it!=states.end(); it++)
    {
        if (processState(it.value()))
            continue;

        currentState = it.key();
        for (auto b : bridge::getBridgeManager()->getCoreWindow())
            b->updateActionStates(currentState);

        return;
    }

    executeFrom( STATE::NONE );
}

// Check if current state agree to switch the state
bool StateMachine::canSwitchState(STATE nextWindowState) {
    State* prevState = states.value(currentState, nullptr);

    if (prevState) {
        return prevState->canExitState(nextWindowState);
    }
    return true;
}

// Try to chnage the state.
// return: true - if was changes
//         false - operation was cancelled and can't be done...
void StateMachine::executeFrom( STATE nextState ) {

    // notify current state
    State* prevState = states.value(currentState, nullptr);
    if (prevState)
        prevState->exitingState();

    if (nextState == STATE::NONE)
        nextState = states.firstKey();

    if ( isLogoutOff(nextState ) )
        logoutTime = 0;

    Q_ASSERT( states.contains(nextState) );

    {
        STATE newState = STATE::NONE;
        for ( auto it = states.find(nextState); it!=states.end(); it++)
        {
            if ( processState( it.value() ) )
                continue;

            newState = it.key();
            for (auto b : bridge::getBridgeManager()->getCoreWindow())
                b->updateActionStates(newState);
            break;
        }
        currentState = newState;
    }

    // Resync is blocking logout. We need to respect that.
    if ( !isLogoutOff(currentState) )
        resetLogoutLimit(true);

    if (currentState == STATE::NONE) {
        // Selecting the send page if nothing found
        getStateContext()->appContext->setActiveWndState( STATE::NODE_INFO );
        executeFrom(STATE::NONE);
    }
}

bool StateMachine::setActionWindow( STATE actionWindowState, bool enforce ) {
    if (!enforce ) {
        if (!isActionWindowMode())
            return false;
    }

    if (!canSwitchState(actionWindowState))
        return false;

    getStateContext()->appContext->setActiveWndState(actionWindowState);
    executeFrom(actionWindowState);
    return currentState==actionWindowState;
}

STATE StateMachine::getActionWindow() const {
    return getStateContext()->appContext->getActiveWndState();
}

// return true if action window will applicable
bool StateMachine::isActionWindowMode() const {
    return getStateContext()->appContext->getActiveWndState() == currentState;
}

// Reset logout time.
void StateMachine::resetLogoutLimit(bool resetBlockLogoutCounter ) {
    if (config::getLogoutTimeMs() < 0)
        logoutTime = 0;
    else
        logoutTime = QDateTime::currentMSecsSinceEpoch() + config::getLogoutTimeMs();

    if (resetBlockLogoutCounter)
        blockLogoutStack.clear();
}

// Logout must be blocked for modal dialogs
void StateMachine::blockLogout(const QString & id) {
    blockLogoutStack.push_back(id);
    logoutTime = 0;
}
void StateMachine::unblockLogout(const QString & id) {
    if (id.isEmpty()) {
        blockLogoutStack.clear();
    }
    else {
        int idx = blockLogoutStack.size()-1;
        for (; idx>=0; idx--) {
            if (blockLogoutStack[idx] == id)
                break;
        }
        if (idx>=0)
            blockLogoutStack.resize(idx);
    }
    if (blockLogoutStack.isEmpty() && logoutTime==0)
        resetLogoutLimit(true);
}

//////////////////////////////////////////////////////////////////////////

bool StateMachine::processState(State* st) {
    NextStateRespond resp = st->execute();
    if (resp.result == NextStateRespond::RESULT::DONE)
        return true;

    if (resp.result == NextStateRespond::RESULT::WAIT_FOR_ACTION)
        return false;

    Q_ASSERT(false); // NONE state is a bug

    return false;
}

void StateMachine::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event)

    // No locking make sense for the node.
    if (config::isOnlineNode())
        return;

    // no password - no locks.
    if(!getStateContext()->wallet->hasPassword())
        return;

    // Check if timer expired and we need to logout...
    if (logoutTime == 0)
        return;

    if (QDateTime::currentMSecsSinceEpoch() > logoutTime ) {
        // logout
        logoutTime = 0;
        logout();
    }
}

// logout now
void StateMachine::logout() {
    if(!getStateContext()->wallet->hasPassword()) {
        core::getWndManager()->messageTextDlg("Logout", "Your wallet doesn't protected with a password. Because of that you can't do a logout.");
        return;
    }

    getStateContext()->appContext->pushCookie<QString>("LockWallet", "lock");
    executeFrom(STATE::NONE);
}

}
