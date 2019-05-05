#include "state/inputpassword.h"
#include "windows/inputpassword_w.h"
#include "../wallet/wallet.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"


namespace state {

InputPassword::InputPassword(const StateContext & context) :
    State(context, STATE::INPUT_PASSWORD)
{
}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    if ( context.appContext->getCookie<QString>(COOKIE_PASSWORD).length()==0) {

        context.wndManager->switchToWindow(
                    new wnd::InputPassword( context.wndManager->getInWndParent(), this ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

bool InputPassword::checkPassword(const QString & password) {
    return context.appContext->checkPassHash(password);
}

void InputPassword::submitPassword(const QString & password) {
    // no needs ti update the hash
    context.appContext->pushCookie<QString>(COOKIE_PASSWORD, password);

    context.stateMachine->executeFrom(STATE::INPUT_PASSWORD);
}


}
