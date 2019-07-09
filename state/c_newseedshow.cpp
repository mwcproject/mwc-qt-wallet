#include "c_newseedshow.h"
#include "../wallet/wallet.h"
#include "../windows/c_newseed_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

NewSeedShow::NewSeedShow(const StateContext & context) :
    State(context, STATE::SHOW_NEW_SEED)
{

}

NewSeedShow::~NewSeedShow() {}

NextStateRespond NewSeedShow::execute() {
    auto seed = context.appContext->getCookie< QVector<QString> >("seed2verify");

    if (seed.size()==0)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindowEx(
                new wnd::NewSeed( context.wndManager->getInWndParent(), this, getStateMachine(), context.appContext, seed ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );

}

void NewSeedShow::submit() {
    // go to the next page
    context.stateMachine->executeFrom(STATE::TEST_NEW_SEED);
}

}
