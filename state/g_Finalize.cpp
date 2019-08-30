// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <control/messagebox.h>
#include "g_Finalize.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../core/windowmanager.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../windows/g_finalizeupload_w.h"
#include "../windows/g_filetransaction_w.h"
#include "../core/global.h"

namespace state {

Finalize::Finalize( StateContext * context) :
    State(context, STATE::FINALIZE)
{
    QObject::connect( context->wallet, &wallet::Wallet::onFinalizeFile,
                          this, &Finalize::onFinalizeFile, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onAllTransactions,
                      this, &Finalize::onAllTransactions, Qt::QueuedConnection );

    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Finalize::onNodeStatus, Qt::QueuedConnection);

}

Finalize::~Finalize() {}

NextStateRespond Finalize::execute() {

    if (context->appContext->getActiveWndState() != STATE::FINALIZE)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    // !!!! Note.  Currently we don't need any transaction info. But code works. Just uncomment it and data will be pushed.
    // will get respond at onAllTransactions
    // This task consider backgroud and we don't care if it is not finished.
    // That is why no progress will be printed.
    //context->wallet->getAllTransactions();

    ftBack();

    return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
}

void Finalize::ftBack() {
    uploadWnd = ( wnd::FinalizeUpload* ) context->wndManager->switchToWindowEx( mwc::PAGE_G_FINALIZE_UPLOAD,
                new wnd::FinalizeUpload( context->wndManager->getInWndParent(), this ) );
}


QString Finalize::getFileGenerationPath() {
    return context->appContext->getPathFor("fileGen");
}

void Finalize::updateFileGenerationPath(QString path) {
    context->appContext->updatePathFor("fileGen", path);
}

// Process to the next Step, show transaction details
void Finalize::fileTransactionUploaded( const QString & fileName, const util::FileTransactionInfo & transInfo ) {

    wallet::WalletTransaction transaction;

    /*  No need to get any transaction info
    if (!transInfo.transactionId.isEmpty()) {
        for (auto &tr : allTransactions) {
            if (!tr.txid.isEmpty() && transInfo.transactionId.startsWith(tr.txid) ) {
                transaction = tr;
                break;
            }
        }
    }*/

    // Let's try to find the transaction that match that file.

    fileTransWnd = (wnd::FileTransaction*) context->wndManager->switchToWindowEx( mwc::PAGE_G_FINALIZE_TRANS,
               new wnd::FileTransaction( context->wndManager->getInWndParent(), this, fileName, transInfo, transaction, lastNodeHeight,
                                         "Finalize Transaction", "Finalize") );
}


// Expected that user already made all possible appruvals
void Finalize::ftContinue(QString fileName) {
    logger::logInfo("Finalize", "finalizing file " + fileName);
    context->wallet->finalizeFile(fileName);
}

void Finalize::onFinalizeFile( bool success, QStringList errors, QString fileName ) {
    logger::logInfo("Finalize", "Get file finalize results. success=" + QString::number(success) + " errors=" +
                 errors.join(",") + " fileName=" + fileName );

    if (fileTransWnd!=nullptr)
        fileTransWnd->hideProgress();

    if (success) {
        control::MessageBox::message(nullptr, "Finalize File Transaction", "File Transaction was finalized successfully.");
        ftBack();
    }
    else {
        control::MessageBox::message(nullptr, "Failure", "File Transaction failed to finalize.\n" + errors.join("\n") );
    }
}

state::StateContext * Finalize::getContext() {
    return context;
}


void Finalize::onAllTransactions( QVector<wallet::WalletTransaction> transactions) {
    allTransactions = transactions;
}

void Finalize::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg);
    Q_UNUSED(peerHeight);
    Q_UNUSED(totalDifficulty);
    Q_UNUSED(connections);

    if (online)
        lastNodeHeight = nodeHeight;
}


}
