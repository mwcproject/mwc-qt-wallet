#include "a_StartWallet.h"
#include "../core/appcontext.h"
#include "../core/windowmanager.h"
#include "../state/statemachine.h"
#include "../windows/a_waitingwnd.h"
#include "../util/Log.h"

namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
StartWallet::StartWallet(StateContext * context) :
        State(context, STATE::START_WALLET)
{
}

StartWallet::~StartWallet() {
}

NextStateRespond StartWallet::execute() {
    if ( !context->wallet->isRunning() ) {
        // Just update the wallet with a status. Then continue
        context->appContext->pushCookie<QString>("checkWalletInitialized",
                context->wallet->checkWalletInitialized() ? "OK" : "FAILED" );
    }

    return NextStateRespond(NextStateRespond::RESULT::DONE);
}




}
