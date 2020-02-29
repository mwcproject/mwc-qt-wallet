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
#include "../state/statemachine.h"
#include "../util/Log.h"
#include <QDebug>
#include "../core/global.h"

namespace state {


Outputs::Outputs(StateContext * context) :
    State(context, STATE::OUTPUTS)
{
    QObject::connect( context->wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &Outputs::onWalletBalanceUpdated, Qt::QueuedConnection );

    QObject::connect( context->wallet, &wallet::Wallet::onOutputs, this, &Outputs::onOutputs );
    QObject::connect( context->wallet, &wallet::Wallet::onOutputCount, this, &Outputs::onOutputCount );

    QObject::connect( notify::Notification::getObject2Notify(), &notify::Notification::onNewNotificationMessage,
                      this, &Outputs::onNewNotificationMessage, Qt::QueuedConnection );
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

void Outputs::requestOutputCount(bool show_spent, QString account) {
    context->wallet->getOutputCount(show_spent, account);
}

// request wallet for outputs
void Outputs::requestOutputs(QString account, int offset, int number, bool show_spent, bool enforceSync) {
    context->wallet->getOutputs(account, offset, number, show_spent, enforceSync);
    // Respond:  onOutputs(...)
    // Balance need to be updated as well to match outputs state
    context->wallet->updateWalletBalance(false,false);
}

void Outputs::onOutputCount(QString account, int count) {
    if (wnd) {
        wnd->setOutputCount(account, count);
    }
}

void Outputs::onOutputs( QString account, int64_t height, QVector<wallet::WalletOutput> outputs) {
    qDebug() << "state onOutputs call for wnd=" << wnd;
    if (wnd) {
        wnd->setOutputsData(account,height, outputs);
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



}
