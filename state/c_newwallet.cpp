#include "c_newwallet.h"
#include "../wallet/wallet.h"
#include "../windows/c_newwallet_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

NewWallet::NewWallet( StateContext * context) :
    State(context, STATE::NEW_WALLET)
{
}


NewWallet::~NewWallet() {
}

void NewWallet::submitCreateChoice(NEW_WALLET_CHOICE newWalletChoice) {
    switch (newWalletChoice) {
    case CREATE_NEW:
        context->appContext->pushCookie<QString>("newSeed", "Yes");
        context->stateMachine->executeFrom(STATE::GENERATE_NEW_SEED);
        break;
    case CREATE_WITH_SEED:
        context->appContext->pushCookie<QString>("withSeed", "Yes");
        context->stateMachine->executeFrom(STATE::CREATE_WITH_SEED);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

NextStateRespond NewWallet::execute() {
    if ( context->wallet->getWalletStatus() == wallet::InitWalletStatus::NEED_INIT ) {
        // Password expected to be entered
        QString pass = context->appContext->getCookie<QString>(COOKIE_PASSWORD);
        Q_ASSERT(pass.length()>0);


        context->wndManager->switchToWindowEx(
                    new wnd::NewWallet( context->wndManager->getInWndParent(), this ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    return NextStateRespond(NextStateRespond::RESULT::DONE);
}

}
