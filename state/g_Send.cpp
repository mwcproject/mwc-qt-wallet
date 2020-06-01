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
#include "g_Send.h"
#include "../wallet/wallet.h"
#include "windows/g_sendStarting.h"
#include "windows/g_sendOnline.h"
#include "windows/g_sendOffline.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../core/global.h"
#include "../core/Config.h"

namespace state {

void SendEventInfo::setData(QString _address, int64_t _txid,  QString _slate, bool _send, bool _respond) {
    address = _address;
    txid    = _txid;
    slate   = _slate;
    send = _send;
    respond = _respond;
    timestamp = QDateTime::currentMSecsSinceEpoch();
}

Send::Send(StateContext * context) :
        State(context, STATE::SEND) {

    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated,
                      this, &Send::onWalletBalanceUpdated, Qt::QueuedConnection );
    QObject::connect(context->wallet, &wallet::Wallet::onSend,
                     this, &Send::sendRespond, Qt::QueuedConnection);
    QObject::connect(context->wallet, &wallet::Wallet::onSendFile,
                     this, &Send::respSendFile, Qt::QueuedConnection);

    // Need to update mwc node status because send can fail if node is not healthy.
    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &Send::onNodeStatus, Qt::QueuedConnection);
}

Send::~Send() {}

NextStateRespond Send::execute() {
    if ( context->appContext->getActiveWndState() != STATE::SEND )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    switchToStartingWindow();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void Send::switchToStartingWindow() {
    onlineOfflineWnd = (wnd::SendStarting*)context->wndManager->switchToWindowEx( mwc::PAGE_G_SEND,
            new wnd::SendStarting( context->wndManager->getInWndParent(), this ) );
    context->wallet->updateWalletBalance(true,true); // request update, respond at onWalletBalanceUpdated
}


core::SendCoinsParams Send::getSendCoinsParams() {
    return context->appContext->getSendCoinsParams();
}

void Send::updateSendCoinsParams( const core::SendCoinsParams  & params ) {
    context->appContext->setSendCoinsParams(params);
}

QString Send::getFileGenerationPath() {
    return context->appContext->getPathFor("fileGen");
}

void Send::updateFileGenerationPath(QString path) {
    context->appContext->updatePathFor("fileGen", path);
}

QString Send::getWalletPasswordHash() {
    return context->wallet->getPasswordHash();
}

void Send::switchAccount(const QString & accountName) {
    if (accountName == context->wallet->getCurrentAccountName())
        return;

    context->wallet->switchAccount(accountName);
}

// onlineOffline => Next step
void Send::processSendRequest( bool isOnline, const wallet::AccountInfo & selectedAccount, int64_t amount ) {
    if (isOnline) {
        onlineWnd = (wnd::SendOnline*)context->wndManager->switchToWindowEx( mwc::PAGE_G_SEND_ONLINE,
                new wnd::SendOnline( context->wndManager->getInWndParent(), selectedAccount, amount, this,
                                     (state::Contacts *)context->stateMachine->getState(STATE::CONTACTS) ) );
    }
    else {
        offlineWnd = (wnd::SendOffline*)context->wndManager->switchToWindowEx( mwc::PAGE_G_SEND_FILE,
                new wnd::SendOffline( context->wndManager->getInWndParent(), selectedAccount, amount, this ) );
    }
}

// Request for MWC to send
void Send::sendMwcOnline(const wallet::AccountInfo &account, util::ADDRESS_TYPE type, QString address, int64_t mwcNano,
        QString message, QString apiSecret, const QStringList & outputs, int changeOutputs, bool fluff ) {
    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();
    context->wallet->sendTo( account, mwcNano, util::fullFormalAddress( type, address), apiSecret, message,
            prms.inputConfirmationNumber, changeOutputs, outputs, fluff );
}

void Send::sendRespond( bool success, QStringList errors, QString address, int64_t txid, QString slate, QString mwc ) {
    Q_UNUSED(address)
    Q_UNUSED(txid)
    Q_UNUSED(slate)
    Q_UNUSED(mwc)

    if (onlineWnd) {
        onlineWnd->sendRespond(success, errors);

        if (success)
            switchToStartingWindow();
    }
}

void Send::sendMwcOffline(  const wallet::AccountInfo & account, int64_t amount, QString message, QString fileName,
        const QStringList & outputs, int changeOutputs ) {
    core::SendCoinsParams prms = context->appContext->getSendCoinsParams();
    context->wallet->sendFile( account, amount, message, fileName,prms.inputConfirmationNumber, changeOutputs, outputs );
}

void Send::respSendFile( bool success, QStringList errors, QString fileName ) {
    if (offlineWnd) {
        QString message;
        if (success)
            message = "Transaction file was successfully generated at " + fileName;
        else
            message = "Unable to generate transaction file.\n" + util::formatErrorMessages(errors);

        offlineWnd->showSendMwcOfflineResult( success, message );

        if (success) {
            switchToStartingWindow();
        }
    }
}

// Account info is updated
void Send::onWalletBalanceUpdated() {
    if (onlineOfflineWnd)
        onlineOfflineWnd->updateAccountBalance( context->wallet->getWalletBalance(true),
                context->wallet->getCurrentAccountName() );
}

void Send::onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections ) {
    Q_UNUSED(errMsg)
    nodeIsHealthy = online &&
                    ((config::isColdWallet() || connections > 0) && totalDifficulty > 0 && nodeHeight > peerHeight - 5);
}


}

