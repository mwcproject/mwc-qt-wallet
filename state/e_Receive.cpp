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
#include "../util/Files.h"
//#include "../util_desktop/timeoutlock.h"
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/e_receive_b.h"
#include "../core/WndManager.h"
#include "node/node_client.h"
#include "util/message_mapper.h"
#ifdef WALLET_MOBILE
#include "../core_mobile/qtandroidservice.h"
#endif
#include <QFile>
#include <QUrl>

namespace state {

Receive::Receive( StateContext * _context ) :
        State(_context, STATE::RECEIVE_COINS) {
}

Receive::~Receive() {}

NextStateRespond Receive::execute() {
    atInitialPage = true;
    if ( context->appContext->getActiveWndState() != STATE::RECEIVE_COINS  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    ftBack();
    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Receive::signSlatepackTransaction(QString slatepack, QString slateJson, QString slateSenderAddress) {
    logger::logInfo(logger::STATE, "Call Receive::signSlatepackTransaction <slatepack> <slateJson> <slateSenderAddress>");
    util::FileTransactionInfo flTrInfo;
    QPair<bool, QString> perseResult = flTrInfo.parseSlateContent( slateJson, util::FileTransactionType::RECEIVE, slateSenderAddress, context->wallet );

    if (!perseResult.first) {
        for (auto p : bridge::getBridgeManager()->getReceive() )
            p->onTransactionActionIsFinished( false, perseResult.second );
        return;
    }

    // We don't want to intercept the action from other windows like AirDrop...
    if ( isActive() ) {
        signingFile = false;
        core::getWndManager()->pageFileTransactionReceive(mwc::PAGE_G_RECEIVE_TRANS,
                    slatepack, flTrInfo,
                    context->nodeClient->getLastNodeHeight());
        atInitialPage = false;
    }
}


void Receive::ftBack() {
    logger::logInfo(logger::STATE, "Call Receive::ftBack");
    core::getWndManager()->pageRecieve();
    atInitialPage = true;
}

void Receive::receiveSlatepack(QString slatepack, QString description) {
    logger::logInfo(logger::STATE, "Call Receive::receiveSlatepack with <slatepack> <description>");
    // It can be filename or slate
    Q_ASSERT(!signingFile);
    QPair<wallet::ResReceive,QString> sp_err = context->wallet->receiveSlatepack(slatepack, description);

    if (!sp_err.first.tx_UUID.isEmpty() && !sp_err.first.slatepack.isEmpty())
        context->appContext->addReceiveSlatepack(sp_err.first.tx_UUID, sp_err.first.slatepack);

    if (sp_err.second.isEmpty()) {
        Q_ASSERT(!sp_err.first.slatepack.isEmpty());
        atInitialPage = false;
        core::getWndManager()->pageShowSlatepack(sp_err.first.slatepack, sp_err.first.tx_UUID, STATE::RECEIVE_COINS, ".response", false );
    } else {
        core::getWndManager()->messageTextDlg("Failure",
                                              "Unable to receive slatepack transaction.\n\n" + util::mapMessage(sp_err.second));
    }
}


bool Receive::isActive() const {
    return state::getStateMachine()->getCurrentStateId() == STATE::RECEIVE_COINS;
}

bool Receive::mobileBack() {
    if (atInitialPage) {
        return false;
    }
    else {
        ftBack();
        return true;
    }
}

QString Receive::getHelpDocName() {
    if (context->appContext->isFeatureMWCMQS() || context->appContext->isFeatureTor()) {
        return "receive_online.html";
    }
    else {
        return "receive_sp_only.html";
    }
}

bool Receive::needResultTxFileName() {
    logger::logInfo(logger::STATE, "Call Receive::needResultTxFileName");
    return false;
}

QString Receive::getResultTxPath() {
    logger::logInfo(logger::STATE, "Call Receive::getResultTxPath");
    return "";
}

void Receive::updateResultTxPath(QString path) {
    logger::logInfo(logger::STATE, "Call Receive::updateResultTxPath with path=" + path);
    Q_UNUSED(path)
}

bool Receive::isNodeHealthy() const {
    logger::logInfo(logger::STATE, "Call Receive::isNodeHealthy");
    return true;
}

}
