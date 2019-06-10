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
wallet::WalletInfo Accounts::getWalletInfo() {
    return context.wallet->getWalletBalance();
}

// resync (update) account
wallet::WalletInfo Accounts::resync() {
    context.wallet->check();
    return context.wallet->getWalletBalance();
}

// Get account list
QVector<QString> Accounts::getAccounts() {
    return context.wallet->getAccountList();
}

// Make this account current
QPair<bool, QString> Accounts::activateAccount(QString account) {
    return context.wallet->switchAccount( account );
}

// add new account
QPair<bool, QString> Accounts::addAccount(QString account) {
    return context.wallet->createAccount(account);
}

}

