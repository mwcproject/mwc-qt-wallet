#include "e_Recieve.h"
#include "../wallet/wallet.h"
#include "../windows/e_recieve_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"


namespace state {

Recieve::Recieve(const StateContext &context) :
        State(context, STATE::RECIEVE_COINS) {

    QObject::connect(context.wallet, &wallet::Wallet::onMwcMqListenerStatus,
                     this, &Recieve::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onKeybaseListenerStatus,
                     this, &Recieve::onKeybaseListenerStatus, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onMwcAddressWithIndex,
                     this, &Recieve::onMwcAddressWithIndex, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onReceiveFile,
                                   this, &Recieve::respReceiveFile, Qt::QueuedConnection);

}

Recieve::~Recieve() {}

NextStateRespond Recieve::execute() {
    if ( context.appContext->getActiveWndState() != STATE::RECIEVE_COINS  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    QPair<bool,bool> lsnStatus = context.wallet->getListeningStatus();
    context.wallet->getMwcBoxAddress();

    wnd = new wnd::Recieve( context.wndManager->getInWndParent(), this,
                            lsnStatus.first, lsnStatus.second,
                            context.wallet->getLastKnownMwcBoxAddress() );

    context.wndManager->switchToWindow( wnd );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


QString Recieve::getFileGenerationPath() {
    return context.appContext->getPathFor("fileGen");
}

void Recieve::updateFileGenerationPath(QString path) {
    context.appContext->updatePathFor("fileGen", path);
}

void Recieve::signTransaction( QString fileName ) {
    context.wallet->receiveFile( fileName );
}

void Recieve::respReceiveFile( bool success, QStringList errors, QString inFileName ) {
    if (wnd) {
        QString message;
        if (success)
            message = "Transaction file was successfully signed. Resulting transaction located at " + inFileName + ".response";
        else
            message = "Unable to sign file transaction.\n" + util::formatErrorMessages(errors);

        wnd->onTransactionActionIsFinished( success, message );
    }
}

void Recieve::onMwcMqListenerStatus(bool online) {
    if (wnd) {
        wnd->updateMwcMqState(online);
    }
}
void Recieve::onKeybaseListenerStatus(bool online) {
    if (wnd) {
        wnd->updateKeybaseState(online);
    }
}
void Recieve::onMwcAddressWithIndex(QString mwcAddress, int idx) {
    Q_UNUSED(idx);
    if (wnd) {
        wnd->updateMwcMqAddress(mwcAddress);
    }
}



}