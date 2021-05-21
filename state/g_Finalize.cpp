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
#include "../util/Files.h"
#include "../core/global.h"
#include "../core/Config.h"
#include <QCoreApplication>
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/g_filetransaction_b.h"
#include "../bridge/wnd/g_finalize_b.h"
#include "../bridge/wnd/e_receive_b.h"

namespace state {

Finalize::Finalize( StateContext * _context) :
    State(_context, STATE::FINALIZE)
{
    QObject::connect( context->wallet, &wallet::Wallet::onFinalizeFile,

                      this, &Finalize::onFinalizeFile, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onFinalizeSlatepack,
                      this, &Finalize::onFinalizeSlatepack, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onAllTransactions,
                      this, &Finalize::onAllTransactions, Qt::QueuedConnection );

    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Finalize::onNodeStatus, Qt::QueuedConnection);

}

Finalize::~Finalize() {}

NextStateRespond Finalize::execute() {
    atInitialPage = true;
    if (context->appContext->getActiveWndState() != STATE::FINALIZE)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    ftBack();

    return NextStateRespond(NextStateRespond::RESULT::WAIT_FOR_ACTION);
}

void Finalize::ftBack() {
    core::getWndManager()->pageFinalize();
    atInitialPage = true;
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

void Finalize::uploadFileTransaction(QString uriFileName) {
    QString fileName = uriFileName;
#ifdef WALLET_MOBILE
    // Copy to the temprary location so everybody will get an access to the data
    QString tmpFile = util::genTempFileName(".tx.response");
    if (! util::copyUriToFile(uriFileName, tmpFile) ) {
        core::getWndManager()->messageTextDlg( "File", "Unable to copy file to temporary location." );
        return;
    }
    fileName = tmpFile;
#endif
    util::FileTransactionInfo transInfo;
    QPair<bool, QString> perseResult = transInfo.parseSlateFile(fileName, util::FileTransactionType::FINALIZE );

    if (!perseResult.first) {
        core::getWndManager()->messageTextDlg("File", perseResult.second );
        return;
    }

    file2TransactionsInfo.insert(fileName, transInfo);

    atInitialPage = false;
    core::getWndManager()->pageFileTransactionFinalize(mwc::PAGE_G_FINALIZE_TRANS,
                                               fileName, transInfo, lastNodeHeight);
}

void Finalize::uploadSlatepackTransaction( QString slatepack, QString slateJson, QString sender ) {
    util::FileTransactionInfo transInfo;
    QPair<bool, QString> perseResult = transInfo.parseSlateContent(slateJson, util::FileTransactionType::FINALIZE, sender );

    if (!perseResult.first) {
        core::getWndManager()->messageTextDlg("Slatepack validation", perseResult.second );
        return;
    }

    file2TransactionsInfo.insert(transInfo.transactionId, transInfo);

    atInitialPage = false;
    core::getWndManager()->pageFileTransactionFinalize(mwc::PAGE_G_FINALIZE_TRANS,
                                               slatepack, transInfo, lastNodeHeight );
}


// Expected that user already made all possible appruvals
void Finalize::finalizeFile(QString fileName, QString resultTxFileName, bool fluff) {
    if (!file2TransactionsInfo.contains(fileName)) {
        Q_ASSERT(false);
        return;
    }

    file2TransactionsInfo[fileName].resultingFN = resultTxFileName;

    logger::logInfo("Finalize", "finalizing file " + fileName);
    context->wallet->finalizeFile(fileName, fluff);
}


// Expected that user already made all possible appruvals
void Finalize::finalizeSlatepack(QString slatepack, QString txUuid, QString resultTxFileName, bool fluff) {
    if (!file2TransactionsInfo.contains(txUuid)) {
        Q_ASSERT(false);
        return;
    }

    file2TransactionsInfo[txUuid].resultingFN = resultTxFileName;

    logger::logInfo("Finalize", "finalizing slatepack " + slatepack);
    context->wallet->finalizeSlatepack(slatepack, fluff, txUuid );
}

void Finalize::onFinalizeFile( bool success, QStringList errors, QString fileName ) {
    logger::logInfo("Finalize", "Get file finalize results. success=" + QString::number(success) + " errors=" +
                 errors.join(",") + " fileName=" + fileName );

    for (auto p : bridge::getBridgeManager()->getFileTransaction() )
        p->hideProgress();
    for (auto p : bridge::getBridgeManager()->getFinalize() )
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
            finalizeForColdWallet(trInfo);
        }

        ftBack();
    }
    else {
        core::getWndManager()->messageTextDlg("Failure", "File Transaction failed to finalize.\n" + errors.join("\n") );
    }
}

void Finalize::onFinalizeSlatepack( QString tagId, QString error, QString txUuid ) {
    logger::logInfo("Finalize", "Get slatepack finalize results. tagId=" + tagId + ", error=" + error + " txUuid=" + txUuid );

    for (auto p : bridge::getBridgeManager()->getFileTransaction() )
        p->hideProgress();
    for (auto p : bridge::getBridgeManager()->getFinalize() )
        p->hideProgress();

    if (error.isEmpty()) {
        if (!file2TransactionsInfo.contains(txUuid)) {
            // Expected because we can finalzie from send, vew slatepack page as well.
            return;
        }

        const util::FileTransactionInfo & trInfo = file2TransactionsInfo[txUuid];
        if (trInfo.resultingFN.isEmpty()) {
            // Online wallet case. The normal workflow
            core::getWndManager()->messageTextDlg("Finalize Slatepack Transaction", "Slatepack Transaction was finalized successfully.");
        }
        else {
            finalizeForColdWallet(trInfo);
        }

        ftBack();
    }
    else {
        core::getWndManager()->messageTextDlg("Failure", "Slatepack Transaction failed to finalize.\n" + error);
    }
}

void Finalize::finalizeForColdWallet(const util::FileTransactionInfo & trInfo) {
    // Cold wallet workflow, let's copy the transaction file
    QPair<bool,QString> walletPath = ioutils::getAppDataPath( context->wallet->getWalletConfig().getDataPath() );
    if (!walletPath.first) {
        core::getWndManager()->messageTextDlg("Error", walletPath.second);
        QCoreApplication::exit();
        return;
    }

    QString transactionFN = walletPath.second + "/saved_txs/" + trInfo.transactionId + ".mwctx";
    if ( !QFile(transactionFN).exists() ) {
        core::getWndManager()->messageTextDlg("Internal Error", "Transaction file for id '" + trInfo.transactionId + "' not found. mwc713 didn't create expected file.");
        return;
    }


    if ( QFile::exists(trInfo.resultingFN) )
        QFile::remove(trInfo.resultingFN);

    bool copyOk = QFile::copy(transactionFN, trInfo.resultingFN);

    if (copyOk) {
        core::getWndManager()->messageTextDlg("Finalize File Transaction", "Transaction was finalized successfully but it is not published because you are running Cold Wallet.\n"
                                                                           "Resulting transaction located at " + trInfo.resultingFN+ ". Please publish at with MWC Node, so it will be propagated to the blockchain network.");
    }
    else {
        core::getWndManager()->messageTextDlg("IO Error", "Transaction was finalized successfully but we wasn't be able to save file at the requested location. Please note, you need publish this transaction with MWC Node, so it will be propagated to the blockchain network."
                                                          "Your transaction location:\n" + transactionFN);
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

bool Finalize::mobileBack() {
    if (atInitialPage) {
        return false;
    }
    else {
        ftBack();
        return true;
    }
}


}
