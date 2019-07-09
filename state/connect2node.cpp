#include "connect2node.h"
#include "../wallet/wallet.h"
#include "../windows/connect2server.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

Connect2Node::Connect2Node(const StateContext & context) :
    State(context, STATE::CONNECT_2_NODE)
{
}

Connect2Node::~Connect2Node() {}

NextStateRespond Connect2Node::execute() {

    QString cookie = context.appContext->pullCookie<QString>("ChangeNode");

    if ( cookie.length() == 0 )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindowEx(
                new wnd::ConnectToServer( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );

}

void Connect2Node::applyChoice(CONNECT_CHOICE connect) {
    switch(connect) {
    case NODE_LOCAL: {
        wallet::WalletConfig config = context.wallet->getWalletConfig();
//        config.mwcNodeURI = "127.0.0.1:13413";
        context.wallet->setWalletConfig(config);
        context.stateMachine->setActionWindow( STATE::NODE_STATUS, true );
        break;
    }
    case NODE_POOL: {
        wallet::WalletConfig config = context.wallet->getWalletConfig();
//        config.mwcNodeURI = "pool.mwc.mw:13413";
        context.wallet->setWalletConfig(config);
        context.stateMachine->setActionWindow( STATE::NODE_STATUS, true );
        break;
    }
    case NODE_MANUAL:
        context.appContext->pushCookie<QString>("nodeManualy", "yes");
        context.stateMachine->executeFrom(STATE::NODE_MANUALY );
        break;
    default:
        Q_ASSERT(false);
    }
}

void Connect2Node::cancel() {
    context.stateMachine->setActionWindow( STATE::NODE_STATUS, true );
}

}
