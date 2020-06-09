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

#include "e_Receive.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../util/Json.h"
//#include "../util_desktop/timeoutlock.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/e_receive_b.h"
#include "../bridge/wnd/g_filetransaction_b.h"
#include "../core/WndManager.h"

namespace state {

Receive::Receive( StateContext * _context ) :
        State(_context, STATE::RECEIVE_COINS) {

    QObject::connect(context->wallet, &wallet::Wallet::onReceiveFile,
                                   this, &Receive::respReceiveFile, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Receive::onNodeStatus, Qt::QueuedConnection);

}

Receive::~Receive() {}

NextStateRespond Receive::execute() {
    if ( context->appContext->getActiveWndState() != STATE::RECEIVE_COINS  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (bridge::getBridgeManager()->getReceive().isEmpty()) {
        ftBack();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


void Receive::signTransaction( QString fileName ) {

    // Let's parse transaction first
    util::FileTransactionInfo flTrInfo;
    QPair<bool, QString> perseResult = flTrInfo.parseTransaction( fileName, util::FileTransactionType::RECEIVE );

    if (!perseResult.first) {
        for (auto p : bridge::getBridgeManager()->getReceive() )
            p->onTransactionActionIsFinished( false, perseResult.second );

        return;
    }

    // We don't want to intercept the action from other windows like AirDrop...
    if ( isActive() ) {
        core::getWndManager()->pageFileTransaction(mwc::PAGE_G_RECEIVE_TRANS, RECEIVE_CALLER_ID, fileName, flTrInfo, lastNodeHeight,
                    "Receive File Transaction", "Generate Response");
    }
}

void Receive::ftBack() {
    core::getWndManager()->pageRecieve();
}

void Receive::ftContinue(QString fileName, QString resultTxFileName, bool fluff) {
    Q_UNUSED(resultTxFileName)
    Q_UNUSED(fluff)
    logger::logInfo("Receive", "Receive file " + fileName);
    context->wallet->receiveFile( fileName );
}

void Receive::respReceiveFile( bool success, QStringList errors, QString inFileName ) {
    // Checking if this state is really active on UI level
    if (isActive()) {
        for (auto p: bridge::getBridgeManager()->getFileTransaction())
            p->hideProgress();

        if (success) {
            core::getWndManager()->messageTextDlg("Receive File Transaction",
                                         "Transaction file was successfully signed. Resulting transaction located at " +
                                         inFileName + ".response");
            ftBack();
        } else {
            core::getWndManager()->messageTextDlg("Failure",
                                         "Unable to sign file transaction.\n" + util::formatErrorMessages(errors));
        }
    }
}

void Receive::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    Q_UNUSED(peerHeight)
    Q_UNUSED(totalDifficulty)
    Q_UNUSED(connections)

    if (online)
        lastNodeHeight = nodeHeight;
}

bool Receive::isActive() const {
    if (!bridge::getBridgeManager()->getReceive().isEmpty())
        return true;

    for (auto p : bridge::getBridgeManager()->getFileTransaction()) {
        if (p->getCallerId() == RECEIVE_CALLER_ID)
            return true;
    }
    return false;
}


}
