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

#include "mwctoolbar.h"
#include "ui_mwctoolbar.h"
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/statemachine_b.h"
#include "../bridge/corewindow_b.h"

using namespace bridge;

namespace core {

MwcToolbar::MwcToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MwcToolbar)
{
    ui->setupUi(this);

    config = new Config(this);
    wallet = new Wallet(this);
    stateMachine = new StateMachine(this);
    coreWindow = new CoreWindow(this);

    if (config->isColdWallet()) {
        ui->swapToolButton->hide();
    }

    QObject::connect( wallet, &Wallet::sgnWalletBalanceUpdated,
                      this, &MwcToolbar::onWalletBalanceUpdated, Qt::QueuedConnection );
    QObject::connect( wallet, &Wallet::sgnLoginResult,
                      this, &MwcToolbar::onLoginResult, Qt::QueuedConnection );
    QObject::connect( wallet, &Wallet::sgnLogout,
                      this, &MwcToolbar::onLogout, Qt::QueuedConnection );

    QObject::connect( coreWindow, &CoreWindow::sgnUpdateActionStates,
                      this, &MwcToolbar::onUpdateButtonsState, Qt::QueuedConnection );
}

MwcToolbar::~MwcToolbar()
{
    delete ui;
}

// state::STATE state
void MwcToolbar::onUpdateButtonsState( int state ) {
    ui->sendToolButton->setChecked( state==state::SEND );
    ui->receiveToolButton->setChecked( state==state::RECEIVE_COINS );
    ui->transactionToolButton->setChecked( state==state::TRANSACTIONS );
    ui->swapToolButton->setChecked( state==state::SWAP );
    ui->finalizeToolButton->setChecked( state==state::FINALIZE );
}

void MwcToolbar::paintEvent(QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MwcToolbar::on_sendToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::SEND );
}

void MwcToolbar::on_receiveToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::RECEIVE_COINS );
}

void core::MwcToolbar::on_finalizeToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::FINALIZE );
}

void MwcToolbar::on_transactionToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::TRANSACTIONS );
}

// Account info is updated
void MwcToolbar::onWalletBalanceUpdated() {
    qDebug() << "get onWalletBalanceUpdated. Updating the balance";
    ui->totalMwc->setText( wallet->getTotalMwcAmount() + " MWC" );
}

void MwcToolbar::onLoginResult(bool ok) {
    Q_UNUSED(ok)
    ui->totalMwc->setText("");
}
void MwcToolbar::onLogout() {
    ui->totalMwc->setText("");
}

void core::MwcToolbar::on_swapToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::SWAP );
}

}
