#include "send3_Offline.h"
#include "../wallet/wallet.h"
#include "../windows/send3_offlinesettings_w.h"
#include "../windows/send4_offlinefiles_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"


namespace state {

SendOffline::SendOffline(const StateContext & context) :
        State(context, STATE::SEND_OFFLINE ) {

    QObject::connect(context.wallet, &wallet::Wallet::onSendFile,
                                   this, &SendOffline::respSendFile, Qt::QueuedConnection);

    QObject::connect(context.wallet, &wallet::Wallet::onFinalizeFile,
                                   this, &SendOffline::respFinalizeFile, Qt::QueuedConnection);

}

SendOffline::~SendOffline() {}

NextStateRespond SendOffline::execute() {
    if ( context.appContext->getActiveWndState() != STATE::SEND_OFFLINE  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    settingsWnd = new wnd::SendOfflineSettings( context.wndManager->getInWndParent(), this );
    context.wndManager->switchToWindow( settingsWnd );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


QVector<wallet::AccountInfo> SendOffline::getWalletBalance() {
    return context.wallet->getWalletBalance();
}

core::SendCoinsParams SendOffline::getSendCoinsParams() {
    return context.appContext->getSendCoinsParams();
}

void SendOffline::updateSendCoinsParams(const core::SendCoinsParams &params) {
    context.appContext->setSendCoinsParams(params);
}

// Go to the next window
void SendOffline::prepareSendMwcOffline( const wallet::AccountInfo & account, QString message ) {
    // Switching the account async, we don't really need the response
    context.wallet->switchAccount( account.accountName );

    filesWnd = new wnd::SendOfflineFiles( context.wndManager->getInWndParent(), account, this );
    context.wndManager->switchToWindow( filesWnd );
}

QString SendOffline::getFileGenerationPath() {
    return context.appContext->getPathFor("fileGen");
}

void SendOffline::updateFileGenerationPath(QString path) {
    context.appContext->updatePathFor("fileGen", path);
}

void SendOffline::sendToFile(int64_t nanoCoins, QString fileName) {

    context.wallet->sendFile( nanoCoins, fileName );
}

/*void SendOffline::signTransaction( QString fileName ) {
    logger::logConnect("SendOffline", "onReceiveFile");
    sendConnect = QObject::connect(context.wallet, &wallet::Wallet::onReceiveFile,
                                   this, &SendOffline::respReceiveFile, Qt::QueuedConnection);
    context.wallet->receiveFile( fileName );
}*/

void SendOffline::publishTransaction( QString fileName ) {
    context.wallet->finalizeFile( fileName );
}


void SendOffline::respSendFile( bool success, QStringList errors, QString fileName ) {
    if (filesWnd) {
        QString message;
        if (success)
            message = "Transaction file was successfully generated at " + fileName;
        else
            message = "Unable to generate transaction file.\n" + util::formatErrorMessages(errors);

        filesWnd->onTransactionActionIsFinished( success, message );

        if (success)
            context.stateMachine->setActionWindow(STATE::SEND_ONLINE_OFFLINE);
    }
}

/*void SendOffline::respReceiveFile( bool success, QStringList errors, QString inFileName ) {
    logger::logDisconnect("SendOffline", "onReceiveFile");
    QObject::disconnect(sendConnect);

    if (sendFilesWnd) {
        QString message;
        if (success)
            message = "Transaction file was successfully signed. Resulting transaction located at " + inFileName + ".response";
        else
            message = "Unable to sign file transaction.\n" + util::formatErrorMessages(errors);

        sendFilesWnd->onTransactionActionIsFinished( success, message );

        if (success)
            context.stateMachine->setActionWindow(STATE::SEND_ONLINE_OFFLINE);
    }
}*/

void SendOffline::respFinalizeFile( bool success, QStringList errors, QString fileName ) {
    if (filesWnd) {
        QString message;
        if (success)
            message = "Transaction file was successfully published";
        else
            message = "Unable to publish file transaction.\n" + util::formatErrorMessages(errors);

        filesWnd->onTransactionActionIsFinished( success, message );

        if (success)
            context.stateMachine->setActionWindow(STATE::SEND_ONLINE_OFFLINE);
    }
}






}

