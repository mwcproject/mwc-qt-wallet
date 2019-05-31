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
    auto status = context.wallet->getWalletStatus();
    if ( status == wallet::Wallet::InitWalletStatus::NEED_PASSWORD ||
            status == wallet::Wallet::InitWalletStatus::WRONG_PASSWORD )
    {
        wnd = new wnd::InputPassword( context.wndManager->getInWndParent(), this );
        context.wndManager->switchToWindow(wnd);

        slotConn = QObject::connect( context.wallet, &wallet::Wallet::onInitWalletStatus, this, &InputPassword::onInitWalletStatus );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void InputPassword::submitPassword(const QString & password) {
    Q_ASSERT(wnd != nullptr);
    if (wnd) {
        wnd->startWaiting();
    }

    context.wallet->loginWithPassword(password, context.appContext->getCurrentAccount());
}

void InputPassword::onInitWalletStatus( wallet::Wallet::InitWalletStatus  status ) {
    if (status == wallet::Wallet::InitWalletStatus::WRONG_PASSWORD ) {
        Q_ASSERT(wnd != nullptr);
        if (wnd) {
            wnd->stopWaiting();
            wnd->reportWrongPassword();
        }
    } else if (status == wallet::Wallet::InitWalletStatus::READY ) {
        // Great, login is done. Now we can use the wallet
        Q_ASSERT(context.wallet->getWalletStatus() == wallet::Wallet::InitWalletStatus::READY);
        QObject::disconnect(slotConn);
        context.stateMachine->executeFrom(STATE::INPUT_PASSWORD);
    }
    else {
        // seems like fatal error. Unknown state
        context.wallet->reportFatalError("Internal error. Wallet Password verification invalid state.");
    }
}

}
