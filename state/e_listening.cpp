#include "e_listening.h"
#include "../wallet/wallet.h"
#include "../windows/e_listening_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QDebug>
#include "../util/Log.h"

namespace state {

Listening::Listening(const StateContext & context) :
    State(context, STATE::LISTENING )
{
    // Let's establish connectoins at the beginning

    QObject::connect(context.wallet, &wallet::Wallet::onMwcMqListenerStatus,
                                         this, &Listening::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onKeybaseListenerStatus,
                                         this, &Listening::onKeybaseListenerStatus, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onListeningStartResults,
                                         this, &Listening::onListeningStartResults, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onListeningStopResult,
                                         this, &Listening::onListeningStopResult, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onMwcAddressWithIndex,
                                         this, &Listening::onMwcAddressWithIndex, Qt::QueuedConnection);
}

Listening::~Listening() {
    // disconnect will happen automatically
}

NextStateRespond Listening::execute() {
    if ( context.appContext->getActiveWndState() != STATE::LISTENING )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    QPair<bool,bool> lsnStatus = context.wallet->getListeningStatus();

    context.wallet->getMwcBoxAddress();
    // will get result later and will update the window

    wnd = new wnd::Listening( context.wndManager->getInWndParent(), this,
                  lsnStatus.first, lsnStatus.second,
                              context.wallet->getLastKnownMwcBoxAddress(), -1);

    context.wndManager->switchToWindow(wnd);

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Listening::triggerMwcState() {
    QPair<bool,bool> lsnStatus = context.wallet->getListeningStatus();
    if ( !lsnStatus.first ) {
        context.wallet->listeningStart(true, false);
    }
    else {
        context.wallet->listeningStop(true, false);
    }
}

void Listening::requestNextMwcMqAddress() {
    context.wallet->nextBoxAddress();
}

void Listening::requestNextMwcMqAddressForIndex(int idx) {
    context.wallet->changeMwcBoxAddress(idx);
}

void Listening::triggerKeybaseState() {
    QPair<bool,bool> lsnStatus = context.wallet->getListeningStatus();
    qDebug() << "lsnStatus: " << lsnStatus.first << " " << lsnStatus.second;
    if ( !lsnStatus.second ) {
        context.wallet->listeningStart(false, true);
    }
    else {
        context.wallet->listeningStop(false, true);
    }
}

void Listening::onMwcMqListenerStatus(bool online) {
    if (wnd) {
        wnd->updateMwcMqState(online);
    }
}
void Listening::onKeybaseListenerStatus(bool online) {
    if (wnd) {
        wnd->updateKeybaseState(online);
    }
}

// Listening, you will not be able to get a results
void Listening::onListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                               QStringList errorMessages ) // error messages, if get some
{
    Q_UNUSED(mqTry);
    Q_UNUSED(kbTry);
    if (wnd && !errorMessages.empty()) {
        QString msg;
        for (auto & s : errorMessages)
            msg += s + '\n';
        wnd->showMessage("Start listening Error", msg);
    }
}

void Listening::onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                            QStringList errorMessages ) {
    Q_UNUSED(mqTry);
    Q_UNUSED(kbTry);
    if (wnd && !errorMessages.empty()) {
        QString msg;
        for (auto & s : errorMessages)
            msg += s + "/n";
        wnd->showMessage("Start listening Error", msg);
    }
}

void Listening::onMwcAddressWithIndex(QString mwcAddress, int idx) {
    if (wnd) {
        wnd->updateMwcMqAddress(mwcAddress, idx);
    }
}



}
