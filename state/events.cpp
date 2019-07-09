#include "events.h"
#include "../wallet/wallet.h"
#include "../windows/events_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"



namespace state {

Events::Events(const StateContext & context):
    State(context, STATE::EVENTS)
{
}

Events::~Events() {}

NextStateRespond Events::execute() {
    if (context.appContext->getActiveWndState() != STATE::EVENTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindowEx(
                new wnd::Events( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

}
