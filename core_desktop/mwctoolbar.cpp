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

#include "../features.h"
#include "mwctoolbar.h"
#include "ui_mwctoolbar.h"
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/statemachine_b.h"
#include "../bridge/corewindow_b.h"
#include "../core/global.h"
#include "../control_desktop/messagebox.h"
#include "core/WalletApp.h"

using namespace bridge;

namespace core {

MwcToolbar::MwcToolbar(QWidget *parent) :
    control::MwcWidget(parent),
    ui(new Ui::MwcToolbar) {
    ui->setupUi(this);

    config = new Config(this);
    wallet = new Wallet(this);
    walletConfig = new WalletConfig(this);
    stateMachine = new StateMachine(this);
    coreWindow = new CoreWindow(this);

    bool showSwap = false;
    bool showMktplace = false;

    if (!config->isColdWallet() && mwc::isSwapActive()) {
#ifdef FEATURE_SWAP
        showSwap = true;
#endif
#ifdef FEATURE_MKTPLACE
        showMktplace = true;
#endif
    }

    if (!showSwap) {
        ui->swapToolButton->hide();
    }

    if (!showMktplace) {
        ui->swapMarketplaceToolButton->hide();
    }

    QObject::connect( wallet, &Wallet::sgnWalletBalanceUpdated,
                      this, &MwcToolbar::onWalletBalanceUpdated, Qt::QueuedConnection );
    QObject::connect( wallet, &Wallet::sgnLogin,
                      this, &MwcToolbar::onLogin, Qt::QueuedConnection );
    QObject::connect( wallet, &Wallet::sgnLogout,
                      this, &MwcToolbar::onLogout, Qt::QueuedConnection );
    QObject::connect( coreWindow, &CoreWindow::sgnUpdateActionStates,
                      this, &MwcToolbar::onUpdateButtonsState, Qt::QueuedConnection );
    QObject::connect( walletConfig, &WalletConfig::sgnWalletFeaturesChanged,
                      this, &MwcToolbar::onWalletFeaturesChanged, Qt::QueuedConnection );

    ui->mwcUnconfFrame->hide();

    onWalletFeaturesChanged();

    startTimer(300);
}

MwcToolbar::~MwcToolbar()
{
    delete ui;
}

static void checkButton(QToolButton *sendToolButton, bool checked) {
    if (checked) {
        sendToolButton->setChecked(true);
    }
    else {
        sendToolButton->setChecked(false);
        sendToolButton->clearFocus();
    }
}

// state::STATE state
void MwcToolbar::onUpdateButtonsState( int state ) {
    checkButton( ui->sendToolButton, state==state::SEND );
    checkButton( ui->receiveToolButton, state==state::RECEIVE_COINS );
    checkButton( ui->transactionToolButton, state==state::TRANSACTIONS );
#ifdef FEATURE_SWAP
    checkButton( ui->swapToolButton, state==state::SWAP );
#endif
#ifdef FEATURE_MKTPLACE
    checkButton( ui->swapMarketplaceToolButton, state==state::SWAP_MKT );
#endif
    checkButton( ui->finalizeToolButton, state==state::FINALIZE );
}

void MwcToolbar::paintEvent(QPaintEvent *)
{
     QStyleOption opt;
     opt.initFrom(this);
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

    QVector<QString> amounts = wallet->getTotalAmount();

    QString totalHtml = QString(R"(
            <span style="font-size:20px;">%1</span>
            <span style="font-size:15px;">%2</span>)")
    .arg(amounts[0], "." + amounts[1]);

    ui->totalMwc->setTextFormat(Qt::RichText);
    ui->totalMwc->setText(totalHtml);

    ui->mwc->setText("MWC");

    QString unconfirmed = amounts[2];
    if (unconfirmed.isEmpty()) {
        ui->mwcUnconfFrame->hide();
    }
    else {
        ui->mwcUnconfFrame->show();
        ui->nonConfMwcValue->setText(unconfirmed + " MWC");
    }
}

void MwcToolbar::onLogin() {
    ui->totalMwc->setText("");
    ui->mwc->setText("");
}
void MwcToolbar::onLogout() {
    ui->totalMwc->setText("");
    ui->mwc->setText("");
}

void MwcToolbar::on_swapToolButton_clicked()
{
    if (mwc::isSwapActive()) {
#ifdef FEATURE_SWAP
        stateMachine->setActionWindow(state::STATE::SWAP);
#endif
    }
}

void MwcToolbar::on_swapMarketplaceToolButton_clicked()
{
    if (mwc::isSwapActive()) {
#ifdef FEATURE_MKTPLACE
        stateMachine->setActionWindow(state::STATE::SWAP_MKT);
#endif
    }
}

void MwcToolbar::timerEvent(QTimerEvent *event)
{
    if (core::WalletApp::isExiting())
        return;

    static int counter = 0;
    static int64_t lastNoGasErrorEvent = 0;

    Q_UNUSED(event);
    if (mwc::hasSwapErrors(30000)) {
        ui->swapToolButton->setIcon( QIcon( QPixmap( (counter++ % 2)==0 ? ":/img/swap@2x.svg" : ":/img/swap_yellow@2x.svg" )));
    }
    else {
        ui->swapToolButton->setIcon( QIcon( QPixmap( ":/img/swap@2x.svg" )));
    }

    if (mwc::hasNoGasError()) {
        if (lastNoGasErrorEvent < QDateTime::currentMSecsSinceEpoch() - 15*60*1000) {
            control::MessageBox::messageText(this, "No Ethers", "Please deposit some Ethers for funds or gas. Otherwise, it's possible to lose money!!!");
            lastNoGasErrorEvent = QDateTime::currentMSecsSinceEpoch();
        }
    }
}

void MwcToolbar::onWalletFeaturesChanged() {
    if (walletConfig->isFeatureSlatepack()) {
        // Finalize make sense for Slatepack only.
        ui->finalizeToolButton->show();
    }
    else {
        ui->finalizeToolButton->hide();
    }


}

}
