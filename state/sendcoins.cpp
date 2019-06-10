#include "sendcoins.h"
#include "../wallet/wallet.h"
#include "../windows/sendcoins_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"

namespace state {

void SendCoinsParams::saveData(QDataStream & out) const {
    out << int(0x348A4);
    out << inputConfirmationNumber;
    out << changeOutputs;
}

bool SendCoinsParams::loadData(QDataStream & in) {
    int id = -1;
    in >> id;
    if ( id!=0x348A4 )
        return false;

    in >> inputConfirmationNumber;
    in >> changeOutputs;
    return true;
}


SendCoins::SendCoins(const StateContext & context) :
    State(context, STATE::SEND_COINS)
{
}

SendCoins::~SendCoins() {}

NextStateRespond SendCoins::execute() {
    if (context.appContext->getActiveWndState() != STATE::SEND_COINS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::SendCoins( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

SendCoinsParams SendCoins::getSendCoinsParams() {
    return context.appContext->getSendCoinsParams();
}

void SendCoins::updateSendCoinsParams( const SendCoinsParams  & params ) {
    context.appContext->setSendCoinsParams(params);
}

QVector<wallet::WalletContact> SendCoins::getContacts() {
    return context.wallet->getContacts();
}

QVector<wallet::WalletTransaction> SendCoins::getTransactions(int numOfTransactions) {
    return context.wallet->getTransactions(numOfTransactions);
}


QPair<bool, QString> SendCoins::sendCoins( long nanoCoins, QString address, QString message ) {
    SendCoinsParams params = context.appContext->getSendCoinsParams();
    return context.wallet->sendTo(nanoCoins, address, message, params.inputConfirmationNumber, params.changeOutputs );
}

QVector<int> SendCoins::getColumnsWidhts() const {
    return context.appContext->getIntVectorFor("SendTblColWidth");
}

void SendCoins::updateColumnsWidhts(const QVector<int> & widths) {
    context.appContext->updateIntVectorFor("SendTblColWidth", widths);
}


}
