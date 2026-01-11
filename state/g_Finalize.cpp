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
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"
#include <QCoreApplication>

#include "statemachine.h"
#include "../core/WndManager.h"
#include "core/Notification.h"
#include "node/node_client.h"
#include "util/ioutils.h"
#include "util/message_mapper.h"

namespace state {

Finalize::Finalize( StateContext * _context) :
    State(_context, STATE::FINALIZE)
{
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
    logger::logInfo(logger::STATE, "Call Finalize::ftBack");
    core::getWndManager()->pageFinalize();
    atInitialPage = true;
}


bool Finalize::needResultTxFileName() {
    logger::logInfo(logger::STATE, "Call Finalize::needResultTxFileName");
    return config::isColdWallet();
}

void Finalize::uploadSlatepackTransaction( QString slatepack, QString slateJson, QString sender, bool switch2nextPage ) {
    logger::logInfo(logger::STATE, "Call Finalize::uploadSlatepackTransaction with <slatepack> <slateJson> <sender>");
    util::FileTransactionInfo transInfo;
    QPair<bool, QString> perseResult = transInfo.parseSlateContent(slateJson, util::FileTransactionType::FINALIZE, sender, context->wallet );

    if (!perseResult.first) {
        core::getWndManager()->messageTextDlg("Slatepack validation", perseResult.second );
        return;
    }

    file2TransactionsInfo.insert(transInfo.transactionId, transInfo);

    qint64 lastNodeHeight = context->nodeClient->getLastNodeHeight();

    atInitialPage = false;
    if (switch2nextPage) {
        core::getWndManager()->pageFileTransactionFinalize(mwc::PAGE_G_FINALIZE_TRANS,
                                                   slatepack, transInfo, lastNodeHeight );
    }
}


// Expected that user already made all possible approvals
void Finalize::finalizeSlatepack(QString slatepack, QString txUuid, QString resultTxFileName, bool fluff, int backStateId) {
    logger::logInfo(logger::STATE, "Call Finalize::finalizeSlatepack with <slatepack> txUuid=" + txUuid + " resultTxFileName=" + resultTxFileName + " fluff=" + QString(fluff ? "true" : "false"));
    if (!file2TransactionsInfo.contains(txUuid)) {
        Q_ASSERT(false);
        return;
    }

    file2TransactionsInfo[txUuid].resultingFN = resultTxFileName;

    logger::logInfo( logger::QT_WALLET, "finalizing slatepack " + slatepack);
    // resultTxFileName non empty for the cold wallet. No reasons to post
    QString error = context->wallet->finalizeSlatepack(slatepack, fluff, !resultTxFileName.isEmpty() );

    logger::logInfo(logger::QT_WALLET, "Get slatepack finalize results. error=" + error + " txUuid=" + txUuid );

    if (error.isEmpty()) {
        // Slatepack we don't want to keep any more. Doesn't make sense after finalize
        context->appContext->deleteSendSlatepack(txUuid);

        if (!file2TransactionsInfo.contains(txUuid)) {
            // Expected because we can finalize from send, veiw slatepack page as well.
            return;
        }

        const util::FileTransactionInfo & trInfo = file2TransactionsInfo[txUuid];

        qint64 amount = 0;
        if (trInfo.amount_fee_not_defined) {
            // Requesting transaction info form the wallet.
            // It is a normal case, compact slate doesn;t have all info
            // Note, account can be any account, looking for send transaction type
            wallet::WalletTransaction tx = context->wallet->getTransactionByUUID( txUuid );
            amount = std::abs(tx.coinNano);
        }
        else {
            amount = trInfo.amount;
        }

        notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "You successfully finalized slatepack " + txUuid +
            (amount<=0 ? "" : " with " + util::nano2one(amount) + " MWC") +
            (trInfo.fromAddress.isEmpty() ? "" : " to " + trInfo.fromAddress) );

        if (trInfo.resultingFN.isEmpty()) {
            // Online wallet case. The normal workflow
            core::getWndManager()->messageTextDlg("Finalize Slatepack Transaction",
                "Slatepack Transaction was finalized successfully.");
        }
        else {
            finalizeForColdWallet(trInfo);
        }

        if (backStateId<0) {
            ftBack();
        }
        else {
            context->stateMachine->setActionWindow(state::STATE(backStateId), false);
        }
    }
    else {
        core::getWndManager()->messageTextDlg("Failure",
            "Slatepack transaction failed to finalize.\n" + util::mapMessage(error));
    }
}

void Finalize::finalizeForColdWallet(const util::FileTransactionInfo & trInfo) {
    logger::logInfo(logger::STATE, "Call Finalize::finalizeForColdWallet with transactionId=" + trInfo.transactionId + " resultingFN=" + trInfo.resultingFN);
    // Cold wallet workflow, let's copy the transaction file
    QPair<bool,QString> walletPath = ioutils::getAppDataPath( context->wallet->getWalletConfig().getDataPath() );
    if (!walletPath.first) {
        core::getWndManager()->messageTextDlg("Error", walletPath.second);
        QCoreApplication::exit();
        return;
    }

    QString transactionFN = walletPath.second + "/saved_txs/" + trInfo.transactionId + ".mwctx";
    if ( !QFile(transactionFN).exists() ) {
        core::getWndManager()->messageTextDlg("Internal Error", "Transaction file for id '" + trInfo.transactionId + "' not found. The wallet didn't create the expected file.");
        return;
    }

    if ( QFile::exists(trInfo.resultingFN) )
        QFile::remove(trInfo.resultingFN);

    bool copyOk = QFile::copy(transactionFN, trInfo.resultingFN);

    if (copyOk) {
        core::getWndManager()->messageTextDlg("Finalize File Transaction", "Transaction was finalized successfully but it is not published because you are running Cold Wallet.\n"
                                                                           "The resulting transaction is located at " + trInfo.resultingFN + ". Please publish it with an MWC Node so it will be propagated to the blockchain network.");
    }
    else {
        core::getWndManager()->messageTextDlg("IO Error", "Transaction was finalized successfully but we were not able to save the file at the requested location. Please note that you need to publish this transaction with an MWC Node so it will be propagated to the blockchain network."
                                                          "Your transaction location:\n" + transactionFN);
    }
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
