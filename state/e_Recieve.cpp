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

#include "e_Recieve.h"
#include "../wallet/wallet.h"
#include "../windows/e_recieve_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "../util/Json.h"
#include "../dialogs/fileslateinfodlg.h"
#include "timeoutlock.h"

namespace state {

Recieve::Recieve( StateContext * context ) :
        State(context, STATE::RECIEVE_COINS) {

    QObject::connect(context->wallet, &wallet::Wallet::onMwcMqListenerStatus,
                     this, &Recieve::onMwcMqListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onKeybaseListenerStatus,
                     this, &Recieve::onKeybaseListenerStatus, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onMwcAddressWithIndex,
                     this, &Recieve::onMwcAddressWithIndex, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onReceiveFile,
                                   this, &Recieve::respReceiveFile, Qt::QueuedConnection);

}

Recieve::~Recieve() {}

NextStateRespond Recieve::execute() {
    if ( context->appContext->getActiveWndState() != STATE::RECIEVE_COINS  )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        QPair<bool,bool> lsnStatus = context->wallet->getListeningStatus();
        context->wallet->getMwcBoxAddress();

        wnd = (wnd::Recieve*) context->wndManager->switchToWindowEx( new wnd::Recieve( context->wndManager->getInWndParent(), this,
                                                          lsnStatus.first, lsnStatus.second,
                                                          context->wallet->getLastKnownMwcBoxAddress() ) );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


QString Recieve::getFileGenerationPath() {
    return context->appContext->getPathFor("fileGen");
}

void Recieve::updateFileGenerationPath(QString path) {
    context->appContext->updatePathFor("fileGen", path);
}

void Recieve::signTransaction( QString fileName ) {
    Q_ASSERT(wnd);

    // Let's parse transaction first
    util::FileTransactionInfo flTrInfo;
    if (!flTrInfo.parseTransaction(fileName)) {
        if (wnd) {
            wnd->onTransactionActionIsFinished( false, "Unable to parse mwc transaction data at file: " + fileName );
        }
        return;
    }

    TimeoutLockObject to( this );

    // Ask for acceptanse...
    dlg::FileSlateInfoDlg acceptDlg( wnd, "Recieve File Transaction", flTrInfo );
    if ( acceptDlg.exec() != QDialog::Accepted ) {
        if (wnd) {
            wnd->stopWaiting();
        }
        return;
    }

    context->wallet->receiveFile( fileName );
}

void Recieve::respReceiveFile( bool success, QStringList errors, QString inFileName ) {
    if (wnd) {
        QString message;
        if (success)
            message = "Transaction file was successfully signed. Resulting transaction located at " + inFileName + ".response";
        else
            message = "Unable to sign file transaction.\n" + util::formatErrorMessages(errors);

        wnd->onTransactionActionIsFinished( success, message );
    }
}

void Recieve::onMwcMqListenerStatus(bool online) {
    if (wnd) {
        wnd->updateMwcMqState(online);
    }
}
void Recieve::onKeybaseListenerStatus(bool online) {
    if (wnd) {
        wnd->updateKeybaseState(online);
    }
}
void Recieve::onMwcAddressWithIndex(QString mwcAddress, int idx) {
    Q_UNUSED(idx);
    if (wnd) {
        wnd->updateMwcMqAddress(mwcAddress);
    }
}


QString  Recieve::getReceiveAccount() {
    return context->appContext->getReceiveAccount();
}

void  Recieve::setReceiveAccount(QString accountName ) {
    context->appContext->setReceiveAccount(accountName);
    context->wallet->setReceiveAccount(accountName);
    // feedback will be ignored. Errors will go to the events naturally
}

QVector<wallet::AccountInfo>  Recieve::getWalletBalance() {
    return context->wallet->getWalletBalance();
}



}
