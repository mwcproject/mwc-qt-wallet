#include "listening.h"
#include "../wallet/wallet.h"
#include "../windows/listening_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

Listening::Listening(const StateContext & context) :
    State(context, STATE::LISTENING )
{
}

Listening::~Listening() {}

NextStateRespond Listening::execute() {
    if ( context.appContext->getActiveWndState() != STATE::LISTENING )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::Listening( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


QPair<bool, QString> Listening::getBoxListeningStatus() {
    uint status = context.wallet->getListeningStatus();
    return QPair<bool, QString>( (status & wallet::Wallet::LISTEN_MWCBOX) != 0, "");
}

QPair<bool, QString> Listening::getKeystoneListeningStatus() {
    uint status = context.wallet->getListeningStatus();
    return QPair<bool, QString>( (status & wallet::Wallet::LISTEN_KEYSTONE) != 0, "");
}

QPair<bool, QString> Listening::startBoxListeningStatus() {
    return context.wallet->startListening( wallet::Wallet::LISTEN_MWCBOX );
}

QPair<bool, QString> Listening::startKeystoneListeningStatus() {
    return context.wallet->startListening( wallet::Wallet::LISTEN_KEYSTONE );
}

QPair<bool, QString> Listening::stopBoxListeningStatus() {
    return context.wallet->stopListening(wallet::Wallet::LISTEN_MWCBOX);
}

QPair<bool, QString> Listening::stopKeystoneListeningStatus() {
    return context.wallet->stopListening(wallet::Wallet::LISTEN_KEYSTONE);
}

QPair<QString,int> Listening::getBoxAddress() {
    return context.wallet->getMwcBoxAddress();
}

void Listening::changeBoxAddress(int idx) {
    context.wallet->changeMwcBoxAddress(idx);
}

void Listening::nextBoxAddress() {
    context.wallet->nextBoxAddress();
}



}
