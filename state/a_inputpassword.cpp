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
    // Result of the login
    QObject::connect( context->wallet, &wallet::Wallet::onLoginResult, this, &InputPassword::onLoginResult, Qt::QueuedConnection );


    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &InputPassword::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect(context->wallet, &wallet::Wallet::onMwcMqListenerStatus,
                                         this, &InputPassword::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onKeybaseListenerStatus,
                                         this, &InputPassword::onKeybaseListenerStatus, Qt::QueuedConnection);

}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    bool running = context->wallet->isRunning();
    QString lockStr = context->appContext->pullCookie<QString>("LockWallet");
    inLockMode = false;

    // Allways try to start the wallet. State before is responsible for the first init
    if ( !running ) {
        // We are at the right place. Let's start the wallet

        // Starting the wallet normally. The password is needed and it will be provided.
        // It is a first run, just need to login
        context->wallet->start(false);

        wnd = (wnd::InputPassword*)context->wndManager->switchToWindowEx(new wnd::InputPassword( context->wndManager->getInWndParent(), this,
                (state::WalletConfig *) context->stateMachine->getState(STATE::WALLET_CONFIG),
                false ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    if (!lockStr.isEmpty()) {
        inLockMode = true;
                // wallet locking mode
        wnd = (wnd::InputPassword*)context->wndManager->switchToWindowEx(new wnd::InputPassword( context->wndManager->getInWndParent(), this,
                      (state::WalletConfig *) context->stateMachine->getState(STATE::WALLET_CONFIG),
                      true ) );
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
    if ( !inLockMode ) {
        context->wallet->logout(true);
        context->wallet->start(false);
        inLockMode = false;
    }

    context->wallet->loginWithPassword( password );
}

QPair<bool,bool> InputPassword::getWalletListeningStatus() {
    return context->wallet->getListeningStatus();
}

void InputPassword::onLoginResult(bool ok) {
    if (!ok) {
        if (wnd) {
            wnd->stopWaiting();
            wnd->reportWrongPassword();
        }
    }
    else {
        // Going forward by initializing the wallet
        if ( context->wallet->getStartedMode() == wallet::Wallet::STARTED_MODE::NORMAL ) { // Normall start of the wallet. Problem that now we have many cases how wallet started

            // Start listening, no feedback interested
            context->wallet->listeningStart(true, false);
            context->wallet->listeningStart(false, true);

            // Set current receive account
            context->wallet->setReceiveAccount(context->appContext->getReceiveAccount());

            // Updating the wallet balance
            context->wallet->updateWalletBalance();
        }

    }
}


// Account info is updated
void InputPassword::onWalletBalanceUpdated() {

    if (context->wallet->getWalletBalance().isEmpty() )
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
