#include "nodemanually.h"
#include "../wallet/wallet.h"
#include "../windows/nodemanually_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

NodeManually::NodeManually(const StateContext & context) :
    State(context, STATE::NODE_MANUALY)
{
}

NodeManually::~NodeManually() {
}

NextStateRespond NodeManually::execute() {
    QString cookie = context.appContext->pullCookie<QString>("nodeManualy");
    if (cookie.length()==0)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindowEx(
                new wnd::NodeManually( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QPair<bool,QString> NodeManually::testMwcNode(QString host, int port) {
    Q_UNUSED(host);
    Q_UNUSED(port);
    return QPair<bool,QString>(true,"OK");
}

void NodeManually::submitMwcNode(QString host, int port) {
    wallet::WalletConfig config = context.wallet->getWalletConfig();
//    config.mwcNodeURI = host + ":" + QString::number(port);
    context.wallet->setWalletConfig(config);

    context.stateMachine->setActionWindow( STATE::NODE_STATUS, true );
}

void NodeManually::cancel() {
    context.stateMachine->setActionWindow( STATE::NODE_STATUS, true );
}


}

