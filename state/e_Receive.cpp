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
#include "../bridge/wnd/g_filetransaction_b.h"
#include "../core/WndManager.h"
#ifdef WALLET_MOBILE
#include "../core_mobile/qtandroidservice.h"
#endif
#include <QFile>

namespace state {

Receive::Receive( StateContext * _context ) :
        State(_context, STATE::RECEIVE_COINS) {

    QObject::connect(context->wallet, &wallet::Wallet::onReceiveFile,
                                   this, &Receive::onReceiveFile, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onReceiveSlatepack,
                     this, &Receive::onReceiveSlatepack, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Receive::onNodeStatus, Qt::QueuedConnection);

#ifdef WALLET_MOBILE
    androidDevice = new QtAndroidService(this);
    QObject::connect(androidDevice, &QtAndroidService::sgnOnFileReady,
                     this, &Receive::sgnOnFileReady, Qt::QueuedConnection);
#endif
}

Receive::~Receive() {}

NextStateRespond Receive::execute() {
    atInitialPage = true;
    if ( context->appContext->getActiveWndState() != STATE::RECEIVE_COINS  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if ( state::getStateMachine()->getCurrentStateId() != STATE::RECEIVE_COINS ) {
        ftBack();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Receive::signSlatepackTransaction(QString slatepack, QString slateJson, QString slateSenderAddress) {
    util::FileTransactionInfo flTrInfo;
    QPair<bool, QString> perseResult = flTrInfo.parseSlateContent( slateJson, util::FileTransactionType::RECEIVE, slateSenderAddress );

    if (!perseResult.first) {
        for (auto p : bridge::getBridgeManager()->getReceive() )
            p->onTransactionActionIsFinished( false, perseResult.second );
        return;
    }

    // We don't want to intercept the action from other windows like AirDrop...
    if ( isActive() ) {
        signingFile = false;
        core::getWndManager()->pageFileTransactionReceive(mwc::PAGE_G_RECEIVE_TRANS, slatepack, flTrInfo, lastNodeHeight);
        atInitialPage = false;
    }
}


void Receive::signTransaction( QString fileName ) {
    // Let's parse transaction first
    util::FileTransactionInfo flTrInfo;
    QPair<bool, QString> perseResult = flTrInfo.parseSlateFile( fileName, util::FileTransactionType::RECEIVE );

    if (!perseResult.first) {
        for (auto p : bridge::getBridgeManager()->getReceive() )
            p->onTransactionActionIsFinished( false, perseResult.second );

        return;
    }

    // We don't want to intercept the action from other windows like AirDrop...
    if ( isActive() ) {
        signingFile = true;
        core::getWndManager()->pageFileTransactionReceive(mwc::PAGE_G_RECEIVE_TRANS, fileName, flTrInfo, lastNodeHeight);
        atInitialPage = false;
    }
}

void Receive::ftBack() {
    core::getWndManager()->pageRecieve();
    atInitialPage = true;
}

void Receive::receiveFile(QString fileName, QString description) {
    logger::logInfo("Receive", "receiveFile " + fileName);
#ifdef WALLET_MOBILE
    QString tmpFile = util::genTempFileName(".tx");
    QFile::copy(fileName, tmpFile);
    fileName = tmpFile;
#endif

    // It can be filename or slate
    Q_ASSERT(signingFile);
    context->wallet->receiveFile(fileName, description);
}

void Receive::receiveSlatepack(QString slatepack, QString description) {
    logger::logInfo("Receive", "Receive Slatepack " + slatepack);
    // It can be filename or slate
    Q_ASSERT(!signingFile);
    context->wallet->receiveSlatepack(slatepack, description, "");
}


void Receive::onReceiveFile( bool success, QStringList errors, QString inFileName ) {
    // Checking if this state is really active on UI level
    if (isActive()) {
        for (auto p: bridge::getBridgeManager()->getFileTransaction())
            p->hideProgress();
        for (auto p: bridge::getBridgeManager()->getReceive())
            p->hideProgress();

        if (success) {
#ifdef WALLET_MOBILE
            // We got temp file, now we need to save the result. To save we need ask user for location
            QString pickerInitialUri = context->appContext->getPathFor("fileGen");
            scrFileName = inFileName + ".response";
            QString dstFile = scrFileName.mid( scrFileName.lastIndexOf('/') );
            androidDevice->createFile( pickerInitialUri, "*/*", dstFile, 300 );
#else
            core::getWndManager()->messageTextDlg("Receive File Transaction",
                                         "Transaction file was successfully signed. Resulting transaction located at " +
                                         inFileName + ".response");
            ftBack();
#endif
        } else {
            core::getWndManager()->messageTextDlg("Failure",
                                         "Unable to receive file transaction.\n\n" + util::formatErrorMessages(errors));
        }
    }
}

#ifdef WALLET_MOBILE
void Receive::sgnOnFileReady( int eventCode, QString fileUri ) {
    qDebug() << "Receive::sgnOnFileReady get " << eventCode << " " <<  fileUri;
    if (eventCode == 300 && !fileUri.isEmpty() && !scrFileName.isEmpty()) {
        context->appContext->updatePathFor("fileGen", fileUri);
        bool ok = util::copyFiles(scrFileName, fileUri);
        scrFileName = "";
        if (ok) {
            ftBack();
        }
    }
}
#endif

void Receive::onReceiveSlatepack( QString tagId, QString error, QString slatepack ) {
    Q_UNUSED(tagId)
    if (isActive()) {
        for (auto p: bridge::getBridgeManager()->getFileTransaction())
            p->hideProgress();
        for (auto p: bridge::getBridgeManager()->getReceive())
            p->hideProgress();

        if (error.isEmpty()) {
            Q_ASSERT(!slatepack.isEmpty());
            atInitialPage = false;
            core::getWndManager()->pageShowSlatepack(slatepack, STATE::RECEIVE_COINS, ".response", false );
        } else {
            core::getWndManager()->messageTextDlg("Failure",
                                                  "Unable to receive slatepack transaction.\n\n" + error);
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


}
