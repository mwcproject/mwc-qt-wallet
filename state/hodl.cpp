#include "hodl.h"
#include "../wallet/wallet.h"
#include "../windows/hodl_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

Hodl::Hodl(const StateContext & context) :
        State(context, STATE::HODL)
{
}

Hodl::~Hodl() {}

NextStateRespond Hodl::execute() {
    if (context.appContext->getActiveWndState() != STATE::HODL)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::Hodl( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QVector<wallet::WalletTransaction> Hodl::getTransactions() {
    return QVector<wallet::WalletTransaction>();
    //return context.wallet->getTransactions();
}

void Hodl::submitForHodl( const QVector<QString> & transactions ) {
    // starting  a int64_t proces of submitting transactions that we will define later.
    Q_UNUSED(transactions);
}



}
