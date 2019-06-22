#include "accounts.h"
#include "../wallet/wallet.h"
#include "../windows/accounts_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {


Accounts::Accounts(const StateContext & context) :
    State(context, STATE::ACCOUNTS)
{
}

Accounts::~Accounts() {}

NextStateRespond Accounts::execute() {
    if (context.appContext->getActiveWndState() != STATE::ACCOUNTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::Accounts( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


// get balance for current account
QVector<wallet::AccountInfo> Accounts::getWalletInfo() {
    return context.wallet->getWalletBalance();
}

// resync (update) account
void Accounts::resync() {
    // TODO
}

// Make this account current
void Accounts::switchAccount(QString account) {
    // TODO
}

// add new account
void Accounts::createAccount(QString account) {
    // TODO
}

}

