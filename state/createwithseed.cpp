#include "createwithseed.h"
#include "../wallet/wallet.h"
#include "../windows/enterseed.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QMessageBox>

namespace state {


CreateWithSeed::CreateWithSeed(const StateContext & context) :
    State(context, STATE::CREATE_WITH_SEED) {
}

CreateWithSeed::~CreateWithSeed() {
}

NextStateRespond CreateWithSeed::execute() {
    QString withSeed = context.appContext->pullCookie<QString>("withSeed");
    if (withSeed.length()==0)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::EnterSeed( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QPair<bool, QString> CreateWithSeed::createWalletWithSeed( QVector<QString> seed ) {
    QPair<bool, QString> result = context.wallet->recover(seed);

    if (result.first) {
        QMessageBox::information(nullptr, "Wallet", "Your wallet was successfully recovered");
        context.stateMachine->executeFrom(STATE::CREATE_WITH_SEED);
    }
    else {
        QMessageBox::critical(nullptr, "Wallet",
             "We wasn't be able to recover your wallet. Error: " + result.second);
    }

    return result;
}


}
