#include "state/a_inputpassword.h"
#include "windows/a_inputpassword_w.h"
#include "../wallet/wallet.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"

namespace state {

InputPassword::InputPassword( StateContext * context) :
    State(context, STATE::INPUT_PASSWORD)
{
    QObject::connect( context->wallet, &wallet::Wallet::onInitWalletStatus, this, &InputPassword::onInitWalletStatus, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &InputPassword::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect(context->wallet, &wallet::Wallet::onMwcMqListenerStatus,
                                         this, &InputPassword::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onKeybaseListenerStatus,
                                         this, &InputPassword::onKeybaseListenerStatus, Qt::QueuedConnection);

}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    auto status = context->wallet->getWalletStatus();

    QString lockStr = context->appContext->pullCookie<QString>("LockWallet");

    if ( status == wallet::InitWalletStatus::NEED_PASSWORD ||
            status == wallet::InitWalletStatus::WRONG_PASSWORD )
    {
        wnd = (wnd::InputPassword*)context->wndManager->switchToWindowEx(new wnd::InputPassword( context->wndManager->getInWndParent(), this, false ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    if (!lockStr.isEmpty()) {
        // wallet locking mode
        wnd = (wnd::InputPassword*)context->wndManager->switchToWindowEx(new wnd::InputPassword( context->wndManager->getInWndParent(), this, true ) );
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

    // Check if we need to logout first. It is very valid case if we in lock mode
    auto status = context->wallet->getWalletStatus();
    if (!(status == wallet::InitWalletStatus::NEED_PASSWORD ||
          status == wallet::InitWalletStatus::WRONG_PASSWORD)) {

        context->wallet->logout();
        context->wallet->start();
    }

    context->wallet->loginWithPassword( password );
}

QPair<bool,bool> InputPassword::getWalletListeningStatus() {
    return context->wallet->getListeningStatus();
}


void InputPassword::onInitWalletStatus( wallet::InitWalletStatus  status ) {

    if (status == wallet::InitWalletStatus::WRONG_PASSWORD ) {
        //Q_ASSERT(wnd != nullptr);
        if (wnd) {
            wnd->stopWaiting();
            wnd->reportWrongPassword();
        }
    } else if (status == wallet::InitWalletStatus::READY ) {
        // Great, login is done. Now we can use the wallet
        Q_ASSERT(context->wallet->getWalletStatus() == wallet::InitWalletStatus::READY);

        // Start listening, no feedback interested
        context->wallet->listeningStart(true, false);
        context->wallet->listeningStart(false, true);

        // Set current receive account
        context->wallet->setReceiveAccount( context->appContext->getReceiveAccount() );

        // Updating the wallet balance
        context->wallet->updateWalletBalance();
    }
}

// Account info is updated
void InputPassword::onWalletBalanceUpdated() {

    if (context->wallet->getWalletStatus() == wallet::InitWalletStatus::NONE)
        return; // in restart mode

    // Using wnd as a flag that we are active
    if (wnd) {
        context->stateMachine->executeFrom(STATE::INPUT_PASSWORD);
    }
}

void InputPassword::onMwcMqListenerStatus(bool online) {
    if (wnd) {
        wnd->updateMwcMqState(online);
    }
}
void InputPassword::onKeybaseListenerStatus(bool online) {
    if (wnd) {
        wnd->updateKeybaseState(online);
    }
}


}
