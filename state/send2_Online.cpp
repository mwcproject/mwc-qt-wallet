#include "send2_Online.h"
#include "../wallet/wallet.h"
#include "../windows/send2_online_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"

namespace state {

SendOnline::SendOnline( StateContext * context ):
        State(context, STATE::SEND_ONLINE ) {
    QObject::connect(context->wallet, &wallet::Wallet::onSend,
                                   this, &SendOnline::sendRespond, Qt::QueuedConnection);
}

SendOnline::~SendOnline() {}

NextStateRespond SendOnline::execute() {
    if ( context->appContext->getActiveWndState() != STATE::SEND_ONLINE )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    wnd = (wnd::SendOnline*)context->wndManager->switchToWindowEx( new wnd::SendOnline( context->wndManager->getInWndParent(), this,
                                                                                        (state::Contacts *) context->stateMachine->getState( STATE::CONTACTS ) )
            );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QVector<wallet::AccountInfo> SendOnline::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

QString SendOnline::getCurrentAccountName() {
    return context->wallet->getCurrentAccountName();
}

core::SendCoinsParams SendOnline::getSendCoinsParams() {
    return context->appContext->getSendCoinsParams();
}

void SendOnline::updateSendCoinsParams(const core::SendCoinsParams &params) {
    context->appContext->setSendCoinsParams(params);
}

// Request for MWC to send
void SendOnline::sendMwc(const wallet::AccountInfo &account, QString address, int64_t mwcNano, QString message) {

    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();
    context->wallet->sendTo( account, mwcNano, address, message, prms.inputConfirmationNumber, prms.changeOutputs );
}

void SendOnline::sendRespond( bool success, QStringList errors ) {

    if (wnd) {
        wnd->sendRespond(success, errors);

        if (success)
            context->stateMachine->setActionWindow(STATE::SEND_ONLINE_OFFLINE);
    }
}


}
