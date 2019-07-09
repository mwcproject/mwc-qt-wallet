#include "nodestatus.h"

#include "../wallet/wallet.h"
#include "../windows/nodestatus_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

NodeStatus::NodeStatus(const StateContext & context) :
    State(context, STATE::NODE_STATUS)
{
}

NodeStatus::~NodeStatus() {}

wallet::WalletConfig NodeStatus::getWalletConfig() {
    return context.wallet->getWalletConfig();
}

wallet::NodeStatus NodeStatus::getNodeStatus() {
    return context.wallet->getNodeStatus();
}

NextStateRespond NodeStatus::execute() {
    if (context.appContext->getActiveWndState() != STATE::NODE_STATUS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindowEx(
                new wnd::NodeStatus( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void NodeStatus::requestToChangeTheNode() {
    context.appContext->pushCookie<QString>("ChangeNode", "yes");
    context.stateMachine->executeFrom(STATE::CONNECT_2_NODE);
}


}


