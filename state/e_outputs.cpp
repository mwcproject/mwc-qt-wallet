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

#include "e_outputs.h"
#include "../windows/e_outputs_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../core/Config.h"
#include "../state/statemachine.h"
#include "../util/crypto.h"
#include "../util/Log.h"
#include <QDebug>
#include "../core/global.h"

namespace state {

void CachedOutputInfo::resetCache(QString account, bool show_spent) {
    currentAccount = account;
    showSpent = show_spent;
    height = -1;
    unspentOutputs.clear();
}

void CachedOutputInfo::setCache(QString account, int64_t height, QVector<wallet::WalletOutput> outputs) {
    Q_UNUSED(account);
    this->height = height;
    allAccountOutputs = outputs;
    // Check if the cache has been reset. If so, we need to regenerate the unspentOutputs.
    if (showSpent == false && unspentOutputs.isEmpty()) {
        // create list of any output that isn't spent
        for (int i=0; i<allAccountOutputs.size(); ++i) {
            if (allAccountOutputs[i].status != OUTPUT_SPENT_STATUS) {
                unspentOutputs.append(allAccountOutputs[i]);
            }
        }
    }
}


Outputs::Outputs(StateContext * context) :
    State(context, STATE::OUTPUTS)
{
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Outputs::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onOutputs, this, &Outputs::onOutputs );

    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                      this, &Outputs::onNewNotificationMessage, Qt::QueuedConnection );

    QObject::connect( context->wallet , &wallet::Wallet::onLoginResult, this, &Outputs::onLoginResult, Qt::QueuedConnection );

    QObject::connect( context->wallet , &wallet::Wallet::onRootPublicKey, this, &Outputs::onRootPublicKey, Qt::QueuedConnection );
}

Outputs::~Outputs() {}

NextStateRespond Outputs::execute() {
    if (context->appContext->getActiveWndState() != STATE::OUTPUTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd = (wnd::Outputs*) context->wndManager->switchToWindowEx( mwc::PAGE_E_OUTPUTS,
                new wnd::Outputs( context->wndManager->getInWndParent(), this) );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// request wallet for outputs
void Outputs::requestOutputs(QString account, bool show_spent, bool enforceSync) {
    if (cachedOutputs.currentAccount != account) {
        enforceSync = true;
    }

    if (enforceSync) {
        // request all outputs, including spent, so we can cache them
        // Respond:  onOutputs(...)
        context->wallet->getOutputs(account, true, enforceSync);
        // Balance needs to be updated as well to match outputs state
        context->wallet->updateWalletBalance(false,false);
        cachedOutputs.resetCache(account, show_spent);
    }
    else if (wnd) {
        wnd->setOutputsData(account, cachedOutputs.height, show_spent ? cachedOutputs.allAccountOutputs : cachedOutputs.unspentOutputs);
    }
}

void Outputs::onOutputs( QString account, int64_t height, QVector<wallet::WalletOutput> outputs) {
    qDebug() << "state onOutputs call for wnd=" << wnd;
    cachedOutputs.setCache(account, height, outputs);
    context->appContext->initOutputNotes(account, cachedOutputs.allAccountOutputs);
    if (wnd) {
        QVector<wallet::WalletOutput>& wndOutputs = cachedOutputs.showSpent ? cachedOutputs.allAccountOutputs : cachedOutputs.unspentOutputs;
        wnd->setOutputCount(account, wndOutputs.size());
        wnd->setOutputsData(account, height, wndOutputs);
    }
}

void Outputs::switchCurrentAccount(const wallet::AccountInfo & account) {
    // Switching without expected feedback.   Possible error will be cought by requestTransactions.
    context->wallet->switchAccount( account.accountName );
}

QVector<wallet::AccountInfo> Outputs::getWalletBalance() {
    return context->wallet->getWalletBalance();
}

QString Outputs::getCurrentAccountName() const {
    return context->wallet->getCurrentAccountName();
}

// IO for columns widhts
QVector<int> Outputs::getColumnsWidhts() const {
    return context->appContext->getIntVectorFor("OutputsTblColWidth");
}

void Outputs::updateColumnsWidhts(const QVector<int> & widths) {
    context->appContext->updateIntVectorFor("OutputsTblColWidth", widths);
}

void Outputs::onWalletBalanceUpdated() {
    if (wnd) {
        wnd->updateWalletBalance();
    }
}

void Outputs::onNewNotificationMessage(notify::MESSAGE_LEVEL  level, QString message) {
    Q_UNUSED(level)

    if (wnd && message.contains("Changing status for output")) {
        wnd->triggerRefresh();
    }
}

void Outputs::onLoginResult(bool ok) {
    Q_UNUSED(ok)

    if (config::isOnlineWallet()) {
        Q_ASSERT(config::isOnlineWallet() || config::isColdWallet());
        context->wallet->getRootPublicKey("");
    }
}

void Outputs::onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature ) {
    Q_UNUSED(errMsg)
    Q_UNUSED(message)
    Q_UNUSED(signature)

    if (success) {
        QString rootPubKeyHash;
        QString rpkey = rootPubKey;
        QByteArray keyHex = rpkey.toUtf8();
        if (!keyHex.isEmpty()) {
            rootPubKeyHash = crypto::hex2str( crypto::HSA256( keyHex ) );
        }
        else {
            rootPubKeyHash = "";
        }
        context->appContext->setOutputNotesWalletId(rootPubKeyHash);
    }
}



}
