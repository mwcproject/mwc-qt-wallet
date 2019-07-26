#include "statemachine.h"
#include "a_initaccount.h"
#include "a_inputpassword.h"
#include "../core/appcontext.h"
#include "../core/mainwindow.h"
#include "k_accounts.h"
#include "k_AccountTransfer.h"
#include "events.h"
#include "hodl.h"
#include "e_Recieve.h"
#include "e_listening.h"
#include "e_transactions.h"
#include "e_outputs.h"
#include "contacts.h"
#include "x_walletconfig.h"
#include "m_airdrop.h"
#include "a_StartWallet.h"
#include "send1_OnlineOffline.h"
#include "send2_Online.h"
#include "send3_Offline.h"
#include "x_ShowSeed.h"
#include "x_Resync.h"
#include "../core/Config.h"

namespace state {

StateMachine::StateMachine(StateContext * _context) :
        context(_context)
{
    context->setStateMachine(this);

    states[ STATE::START_WALLET ]   = new StartWallet(context);
    states[ STATE::STATE_INIT ]     = new InitAccount(context);
    states[ STATE::INPUT_PASSWORD ] = new InputPassword(context);
    states[ STATE::ACCOUNTS ]       = new Accounts(context);

    states[ STATE::ACCOUNT_TRANSFER ] = new AccountTransfer(context);

    states[ STATE::EVENTS ]         = new Events(context);
    states[ STATE::HODL ]           = new Hodl(context);

    states[ STATE::SEND_ONLINE_OFFLINE ] = new SendOnlineOffline(context);
    states[ STATE::SEND_ONLINE ]    = new SendOnline(context);
    states[ STATE::SEND_OFFLINE ]   = new SendOffline(context);

    states[ STATE::RECIEVE_COINS ]  = new Recieve(context);
    states[ STATE::LISTENING ]      = new Listening(context);
    states[ STATE::TRANSACTIONS ]   = new Transactions(context);
    states[ STATE::OUTPUTS ]        = new Outputs(context);
    states[ STATE::CONTACTS ]       = new Contacts(context);
    states[ STATE::WALLET_CONFIG ]  = new WalletConfig(context);
    states[ STATE::AIRDRDOP_MAIN ]  = new Airdrop(context);
    states[ STATE::SHOW_SEED ]      = new ShowSeed(context);
    states[ STATE::RESYNC ]         = new Resync(context);

    startTimer(1000);
}

StateMachine::~StateMachine() {
    for (auto st : states)
        delete st;

    states.clear();
}

void StateMachine::start() {

    // Init the app
    for ( auto it = states.begin(); it!=states.end(); it++)
    {
        if (processState(it.value()))
            continue;

        currentState = it.key();
        context->mainWnd->updateActionStates(currentState);
        break;
    }
}

void StateMachine::executeFrom( STATE nextState ) {
    if (nextState == STATE::NONE)
        nextState = states.firstKey();

    if ( nextState < STATE::ACCOUNTS )
        logoutTime = 0;

    Q_ASSERT( states.contains(nextState) );

    currentState = STATE::NONE;
    for ( auto it = states.find(nextState); it!=states.end(); it++)
    {
        if ( processState( it.value() ) )
            continue;

        currentState = it.key();
        context->mainWnd->updateActionStates(currentState);
        break;
    }

    if ( currentState >= STATE::ACCOUNTS )
        resetLogoutLimit();

    if (currentState == STATE::NONE) {
        // Selecting the send page if nothing found
        context->appContext->setActiveWndState( STATE::SEND_ONLINE_OFFLINE );
        executeFrom(STATE::NONE);
    }

}

bool StateMachine::setActionWindow( STATE actionWindowState, bool enforce ) {
    if (!enforce && !isActionWindowMode() )
            return false;

    context->appContext->setActiveWndState(actionWindowState);
    executeFrom(actionWindowState);
    return currentState==actionWindowState;
}

STATE StateMachine::getActionWindow() const {
    return context->appContext->getActiveWndState();
}

// return true if action window will applicable
bool StateMachine::isActionWindowMode() const {
    return context->appContext->getActiveWndState() == currentState;
}

// Reset logout time.
void StateMachine::resetLogoutLimit() {
    logoutTime = QDateTime::currentMSecsSinceEpoch() + config::getLogoutTimeMs();
    blockLogoutCounter = 0;
}

// Logout must be blocked for modal dialogs
void StateMachine::blockLogout() {
    blockLogoutCounter++;
    logoutTime = 0;
}
void StateMachine::unblockLogout() {
    blockLogoutCounter--;
    if (blockLogoutCounter<=0 && logoutTime==0)
        resetLogoutLimit();
}

//////////////////////////////////////////////////////////////////////////

bool StateMachine::processState(State* st) {
    NextStateRespond resp = st->execute();
    if (resp.result == NextStateRespond::RESULT::DONE)
        return true;

    if (resp.result == NextStateRespond::RESULT::WAIT_FOR_ACTION)
        return false;

    if (resp.result == NextStateRespond::RESULT::NEXT_STATE) {
        executeFrom( resp.nextState );
        return false;
    }

    Q_ASSERT(false); // NONE state is a bug

    return false;
}

void StateMachine::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    // Check if timer expired and we need to logout...
    if (logoutTime==0)
        return;

    if (QDateTime::currentMSecsSinceEpoch() > logoutTime ) {
        // logout
        logoutTime = 0;
        context->appContext->pushCookie<QString>("LockWallet", "lock");
        context->stateMachine->executeFrom(STATE::NONE);
    }
}


}
