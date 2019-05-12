#include "filetransactions.h"
#include "../wallet/wallet.h"
#include "../windows/filetransactions_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"


namespace state {


FileTransactions::FileTransactions(const StateContext & context) :
         State(context, STATE::FILE_TRANSACTIONS)
{
}

FileTransactions::~FileTransactions() {}

NextStateRespond FileTransactions::execute() {
    if ( context.appContext->getActiveWndState() != STATE::FILE_TRANSACTIONS )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::FileTransactions( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// Generate a file with send coins transaction
QPair<bool, QString> FileTransactions::generateSendCoinsFile( long nanoCoins, QString filename ) {
    return context.wallet->sendFile( nanoCoins, filename );
}

// Sign transaction, process response file
QPair<bool, QString> FileTransactions::signTransaction( QString inFileName, QString resultFileName ) {
    return context.wallet->receiveFile(inFileName, resultFileName);
}

// Processing response file. Close transaction and publish it.
QPair<bool, QString> FileTransactions::processResponseFile( QString responseFileName ) {
    return context.wallet->finalizeFile( responseFileName );
}

QString FileTransactions::getFileGenerationPath() {
    return context.appContext->getPathFor("fileGen");
}

void FileTransactions::updateFileGenerationPath(QString path) {
    context.appContext->updatePathFor("fileGen", path);
}


}


