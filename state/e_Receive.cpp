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
#include "../windows/e_receive_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../util/Json.h"
#include "../dialogs/fileslateinfodlg.h"
#include "timeoutlock.h"
#include "../core/global.h"
#include "../control/messagebox.h"

namespace state {

Receive::Receive( StateContext * context ) :
        State(context, STATE::RECEIVE_COINS) {

    QObject::connect(context->wallet, &wallet::Wallet::onMwcMqListenerStatus,
                     this, &Receive::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onKeybaseListenerStatus,
                     this, &Receive::onKeybaseListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onMwcAddressWithIndex,
                     this, &Receive::onMwcAddressWithIndex, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onReceiveFile,
                                   this, &Receive::respReceiveFile, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Receive::onNodeStatus, Qt::QueuedConnection);

    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Receive::onWalletBalanceUpdated, Qt::QueuedConnection );

}

Receive::~Receive() {}

NextStateRespond Receive::execute() {
    if ( context->appContext->getActiveWndState() != STATE::RECEIVE_COINS  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        context->wallet->getMwcBoxAddress();
        ftBack();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


QString Receive::getFileGenerationPath() {
    return context->appContext->getPathFor("fileGen");
}

void Receive::updateFileGenerationPath(QString path) {
    context->appContext->updatePathFor("fileGen", path);
}

void Receive::signTransaction( QString fileName ) {
    Q_ASSERT(wnd);

    // Let's parse transaction first
    util::FileTransactionInfo flTrInfo;
    if (!flTrInfo.parseTransaction(fileName)) {
        if (wnd) {
            wnd->onTransactionActionIsFinished( false, "Unable to parse mwc transaction data at file: " + fileName );
        }
        return;
    }


    wallet::WalletTransaction transaction;
    fileTransWnd = (wnd::FileTransaction*) context->wndManager->switchToWindowEx( mwc::PAGE_G_RECEIVE_TRANS,
                                                                                  new wnd::FileTransaction( context->wndManager->getInWndParent(), this, fileName, flTrInfo, transaction, lastNodeHeight,
                                                                                                            "Receive File Transaction", "Generate Response") );
}

void Receive::ftBack() {
    QPair<bool,bool> lsnStatus = context->wallet->getListenerStatus();
    wnd = (wnd::Receive*) context->wndManager->switchToWindowEx( mwc::PAGE_E_RECEIVE,
                                                                 new wnd::Receive( context->wndManager->getInWndParent(), this,
                                                                                   lsnStatus.first, lsnStatus.second,
                                                                                   context->wallet->getLastKnownMwcBoxAddress(),
                                                                                   context->wallet->getWalletConfig() ) );
}

void Receive::ftContinue(QString fileName) {
    logger::logInfo("Receive", "Receive file " + fileName);
    context->wallet->receiveFile( fileName );
}

state::StateContext * Receive::getContext() {
    return context;
}


void Receive::respReceiveFile( bool success, QStringList errors, QString inFileName ) {
    if (fileTransWnd)
        fileTransWnd->hideProgress();

    if (success) {
        control::MessageBox::message(nullptr, "Receive File Transaction", "Transaction file was successfully signed. Resulting transaction located at " + inFileName + ".response" );
        ftBack();
    }
    else {
        control::MessageBox::message(nullptr, "Failure", "Unable to sign file transaction.\n" + util::formatErrorMessages(errors) );
    }
}

void Receive::onMwcMqListenerStatus(bool online) {
    if (wnd) {
        wnd->updateMwcMqState(online);
    }
}
void Receive::onKeybaseListenerStatus(bool online) {
    if (wnd) {
        wnd->updateKeybaseState(online);
    }
}
void Receive::onMwcAddressWithIndex(QString mwcAddress, int idx) {
    Q_UNUSED(idx);
    if (wnd) {
        wnd->updateMwcMqAddress(mwcAddress);
    }
}

QString  Receive::getReceiveAccount() {
    return context->appContext->getReceiveAccount();
}

void  Receive::setReceiveAccount(QString accountName ) {
    context->appContext->setReceiveAccount(accountName);
    context->wallet->setReceiveAccount(accountName);
    // feedback will be ignored. Errors will go to the events naturally
}

QVector<wallet::AccountInfo>  Receive::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

void Receive::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg);
    Q_UNUSED(peerHeight);
    Q_UNUSED(totalDifficulty);
    Q_UNUSED(connections);

    if (online)
        lastNodeHeight = nodeHeight;
}


void Receive::onWalletBalanceUpdated() {
    if (wnd) {
        wnd->updateWalletBalance();
    }
}


}
