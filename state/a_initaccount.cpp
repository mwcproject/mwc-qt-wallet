#include "a_initaccount.h"
#include "../wallet/wallet.h"
#include "../windows/a_initaccount_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

InitAccount::InitAccount(const StateContext & context) :
        State(context, STATE::STATE_INIT)
{
}

InitAccount::~InitAccount() {

}

NextStateRespond InitAccount::execute() {
    if ( context.wallet->getWalletStatus() == wallet::InitWalletStatus::NEED_INIT &&
        context.appContext->getCookie<QString>(COOKIE_PASSWORD).length()==0 ) {
        // Show window to input password

        context.wndManager->switchToWindowEx(
                    new wnd::InitAccount( context.wndManager->getInWndParent(), this ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void InitAccount::setPassword(const QString & password) {
    context.appContext->pushCookie<QString>(COOKIE_PASSWORD, password);
//    context.appContext->setPassHash(password);

    context.stateMachine->executeFrom(STATE::STATE_INIT);
}


}
