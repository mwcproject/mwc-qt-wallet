#include "statemachine.h"
#include "initaccount.h"
#include "inputpassword.h"
#include "newwallet.h"
#include "newseed.h"
#include "newseedtest.h"
#include "newseedshow.h"
#include "createwithseed.h"
#include "../core/appcontext.h"
#include "../core/mainwindow.h"
#include "accounts.h"
#include "events.h"
#include "hodl.h"
#include "sendcoins.h"
#include "nodestatus.h"
#include "connect2node.h"
#include "nodemanually.h"
#include "filetransactions.h"

namespace state {

StateMachine::StateMachine(StateContext & context) :
    appContext(context.appContext),
    mainWindow(context.mainWnd)
{
    context.setStateMachine(this);
    Q_ASSERT(appContext);

    states[ STATE::STATE_INIT ]     = new InitAccount(context);
    states[ STATE::INPUT_PASSWORD ] = new InputPassword(context);
    states[ STATE::NEW_WALLET ]     = new NewWallet(context);
    states[ STATE::GENERATE_NEW_SEED ] = new NewSeed( context );
    states[ STATE::SHOW_NEW_SEED ]     = new NewSeedShow(context);
    states[ STATE::TEST_NEW_SEED ]     = new NewSeedTest(context);
    states[ STATE::CREATE_WITH_SEED ]  = new CreateWithSeed(context);

    states[ STATE::ACCOUNTS ]       = new Accounts(context);
    states[ STATE::EVENTS ]         = new Events(context);
    states[ STATE::HODL ]           = new Hodl(context);
    states[ STATE::SEND_COINS ]     = new SendCoins(context);
    states[ STATE::NODE_STATUS]     = new NodeStatus(context);
    states[ STATE::CONNECT_2_NODE ] = new Connect2Node(context);
    states[ STATE::NODE_MANUALY]    = new NodeManually(context);
    states[ STATE::FILE_TRANSACTIONS] = new FileTransactions(context);
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
        mainWindow->updateActionStates();
        break;
    }
}

void StateMachine::executeFrom( STATE nextState ) {
    if (nextState == STATE::NONE)
        nextState = states.firstKey();

    Q_ASSERT( states.contains(nextState) );

    currentState = STATE::NONE;
    for ( auto it = states.find(nextState); it!=states.end(); it++)
    {
        if ( processState( it.value() ) )
            continue;

        currentState = it.key();
        mainWindow->updateActionStates();
        break;
    }
}

bool StateMachine::setActionWindow( STATE actionWindowState, bool enforce ) {
    if (!enforce && !isActionWindowMode() )
            return false;

    appContext->setActiveWndState(actionWindowState);
    executeFrom(STATE::NONE);
    return currentState==actionWindowState;
}

STATE StateMachine::getActionWindow() const {
    return appContext->getActiveWndState();
}

// return true if action window will applicable
bool StateMachine::isActionWindowMode() const {
    return appContext->getActiveWndState() == currentState;
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



}
