#include "x_events.h"
#include "../wallet/wallet.h"
#include "../windows/x_events_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"



namespace state {

Events::Events(StateContext * context):
    State(context, STATE::EVENTS)
{
    QObject::connect( context->wallet, &wallet::Wallet::onNewNotificationMessage,
            this, &Events::onNewNotificationMessage, Qt::QueuedConnection );
}

Events::~Events() {}

NextStateRespond Events::execute() {
    if (context->appContext->getActiveWndState() != STATE::EVENTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context->wndManager->switchToWindowEx(
                new wnd::Events( context->wndManager->getInWndParent(), this ) );

    emit updateNonShownWarnings(false);

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Events::deleteEventsWnd(wnd::Events * w)
{
    if (w==wnd) {
        wnd=nullptr;
        messageWaterMark = QDateTime::currentMSecsSinceEpoch();
    }
}

QVector<int> Events::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("NotifTblColWidth");
}

void Events::updateColumnsWidhts(const QVector<int> & widths) {
    context->appContext->updateIntVectorFor("NotifTblColWidth", widths);

}

QVector<wallet::WalletNotificationMessages> Events::getWalletNotificationMessages() {
    return context->wallet->getWalletNotificationMessages();
}

void Events::onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message) {
    Q_UNUSED(message);

    if (wnd!= nullptr) {
        wnd->updateShowMessages();
    }
    else {
        if ( wallet::WalletNotificationMessages::isCritical(level) )
            emit updateNonShownWarnings(true);
    }
}

// Check if some error/warnings need to be shown
bool Events::hasNonShownWarnings() const {
    QVector<wallet::WalletNotificationMessages> msgs = context->wallet->getWalletNotificationMessages();

    for ( int i = msgs.size()-1; i>=0; i-- ) {
        if (msgs[i].time.currentMSecsSinceEpoch() <= messageWaterMark )
            return false;

        if ( msgs[i].isCritical() )
            return true;
    }

    return false;
}


}
