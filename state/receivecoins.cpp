#include "receivecoins.h"
#include "../wallet/wallet.h"
#include "../windows/receivecoins_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "e_transactions.h"
#include "e_listening.h"

namespace state {

ReceiveCoins::ReceiveCoins(const StateContext & context) :
    State(context, STATE::RECIEVE_COINS)
{
    transactionsState = new Transactions(context);
    listeningState = new Listening(context);
    //fileTransactionsState = new FileTransactions(context);
}

ReceiveCoins::~ReceiveCoins() {
    delete transactionsState;
    delete listeningState;
    //delete fileTransactionsState;
}

NextStateRespond ReceiveCoins::execute() {
    if (context.appContext->getActiveWndState() != STATE::RECIEVE_COINS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::ReceiveCoins( context.wndManager->getInWndParent(), listeningState, transactionsState ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


}
