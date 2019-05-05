#include "newwallet.h"
#include "../wallet/wallet.h"
#include "../windows/newwallet_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>

namespace state {

NewWallet::NewWallet(const StateContext & context) :
    State(context, STATE::NEW_WALLET)
{
}


NewWallet::~NewWallet() {
}

void NewWallet::submitCreateChoice(NEW_WALLET_CHOICE newWalletChoice) {
    switch (newWalletChoice) {
    case CREATE_NEW:
        context.appContext->pushCookie<QString>("newSeed", "Yes");
        context.stateMachine->executeFrom(STATE::GENERATE_NEW_SEED);
        break;
    case CREATE_WITH_SEED:
        context.appContext->pushCookie<QString>("withSeed", "Yes");
        context.stateMachine->executeFrom(STATE::CREATE_WITH_SEED);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

NextStateRespond NewWallet::execute() {
    QString pass = context.appContext->getCookie<QString>(COOKIE_PASSWORD);
    Q_ASSERT(pass.length()>0);

    using namespace wallet;

    Wallet::InitWalletStatus status = context.wallet->open( context.appContext->getNetwork(), pass);
    if (status == Wallet::InitWalletStatus::OK)
        return NextStateRespond(NextStateRespond::RESULT::DONE);
    else if (status == Wallet::InitWalletStatus::WRONG_PASSWORD) {
        QMessageBox::critical( nullptr, "Wrong password", "Your password was rejected by the wallet. If you restored you files, please use the password that match your restored data");
        context.appContext->pullCookie<QString>(COOKIE_PASSWORD);
        context.appContext->setPassHash(""); // reset hash

        return NextStateRespond(NextStateRespond::RESULT::NEXT_STATE, STATE::INPUT_PASSWORD);
    }
    else if (status == Wallet::InitWalletStatus::NEED_INIT ) {
        context.wndManager->switchToWindow(
                    new wnd::NewWallet( context.wndManager->getInWndParent(), this ) );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    Q_ASSERT(false);
    return NextStateRespond(NextStateRespond::RESULT::DONE);
}

}
