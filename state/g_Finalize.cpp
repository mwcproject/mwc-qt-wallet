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

#include <QFile>
#include "g_Finalize.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include <QCoreApplication>
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/g_filetransaction_b.h"

namespace state {

Finalize::Finalize( StateContext * _context) :
    State(_context, STATE::FINALIZE)
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
    core::getWndManager()->pageFinalize();
}


bool Finalize::needResultTxFileName() {
    return config::isColdWallet();
}

QString Finalize::getResultTxPath() {
    return context->appContext->getPathFor("resultTx");
}
void Finalize::updateResultTxPath(QString path) {
    context->appContext->updatePathFor("resultTx", path);
}


void Finalize::uploadFileTransaction(QString fileName) {

    util::FileTransactionInfo transInfo;

    QPair<bool, QString> perseResult = transInfo.parseTransaction(fileName, util::FileTransactionType::FINALIZE );

    if (!perseResult.first) {
        core::getWndManager()->messageTextDlg("Slate File", perseResult.second );
        return;
    }

    file2TransactionsInfo.insert(fileName, transInfo);

    core::getWndManager()->pageFileTransaction(mwc::PAGE_G_FINALIZE_TRANS, FINALIZE_CALLER_ID,
                                               fileName, transInfo, lastNodeHeight,
                                               "Finalize Transaction", "Finalize");
}


// Expected that user already made all possible appruvals
void Finalize::ftContinue(QString fileName, QString resultTxFileName, bool fluff) {
    if (!file2TransactionsInfo.contains(fileName)) {
        Q_ASSERT(false);
        return;
    }

    file2TransactionsInfo[fileName].resultingFN = resultTxFileName;

    logger::logInfo("Finalize", "finalizing file " + fileName);
    context->wallet->finalizeFile(fileName, fluff);
}

void Finalize::onFinalizeFile( bool success, QStringList errors, QString fileName ) {
    logger::logInfo("Finalize", "Get file finalize results. success=" + QString::number(success) + " errors=" +
                 errors.join(",") + " fileName=" + fileName );

    for (auto p : bridge::getBridgeManager()->getFileTransaction() )
        p->hideProgress();

    if (success) {
        if (!file2TransactionsInfo.contains(fileName)) {
            Q_ASSERT(false);
            return;
        }

        const util::FileTransactionInfo & trInfo = file2TransactionsInfo[fileName];
        if (trInfo.resultingFN.isEmpty()) {
            // Online wallet case. The normal workflow
            core::getWndManager()->messageTextDlg("Finalize File Transaction", "File Transaction was finalized successfully.");
        }
        else {
            // Cold wallet workflow, let's copy the transaction file
            QPair<bool,QString> walletPath = ioutils::getAppDataPath( context->wallet->getWalletConfig().getDataPath() );
            if (!walletPath.first) {
                core::getWndManager()->messageTextDlg("Error", walletPath.second);
                QCoreApplication::exit();
                return;
            }

            QString transactionFN = walletPath.second + "/saved_txs/" + trInfo.transactionId + ".grintx";
            if ( !QFile(transactionFN).exists() ) {
                core::getWndManager()->messageTextDlg("Internal Error", "Transaction file for id '" + trInfo.transactionId + "' not found. wmc713 didn't create expected file.");
                return;
            }


            if ( QFile::exists(trInfo.resultingFN) )
                QFile::remove(trInfo.resultingFN);

            bool copyOk = QFile::copy(transactionFN, trInfo.resultingFN);

            if (copyOk) {
                core::getWndManager()->messageTextDlg("Finalize File Transaction", "File Transaction was finalized successfully but it is not published because you are running Cold Wallet.\n"
                                             "Resulting transaction located at " + trInfo.resultingFN+ ". Please publish at with mwc node, so it will be propagated to the blockchain network.");
            }
            else {
                core::getWndManager()->messageTextDlg("IO Error", "File Transaction was finalized successfully but we wasn't be able to save file at the requested location. Please note, you need publish this transaction with mwc node, so it will be propagated to the blockchain network."
                                             "Your transaction location:\n" + transactionFN);
            }
        }

        ftBack();
    }
    else {
        core::getWndManager()->messageTextDlg("Failure", "File Transaction failed to finalize.\n" + errors.join("\n") );
    }
}

void Finalize::onAllTransactions( QVector<wallet::WalletTransaction> transactions) {
    allTransactions = transactions;
}

void Finalize::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)

    nodeIsHealthy = online && ( (config::isColdWallet() || connections>0) && totalDifficulty>0 && nodeHeight>peerHeight-5); // 5 blocks difference will be tolerated

    if (online)
        lastNodeHeight = nodeHeight;
}


}
