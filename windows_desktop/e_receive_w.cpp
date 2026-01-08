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

#include "e_receive_w.h"
#include "ui_e_receive.h"
#include <QFileInfo>
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/util_b.h"
#include "../bridge/heartbeat_b.h"
#include "../bridge/wnd/e_receive_b.h"
#include "../core/global.h"
#include "../dialogs_desktop/g_inputslatepackdlg.h"
#include "zz_utils.h"
#include "core/Config.h"

namespace wnd {

Receive::Receive(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::Receive)
{
    ui->setupUi(this);

    config  = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);
    wallet  = new bridge::Wallet(this);
    receive = new bridge::Receive(this);
    util    = new bridge::Util(this);
    heartBeat = new bridge::HeartBeat(this);

    QObject::connect( receive, &bridge::Receive::sgnTransactionActionIsFinished,
                      this, &Receive::onSgnTransactionActionIsFinished, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Receive::onSgnWalletBalanceUpdated, Qt::QueuedConnection);

    QObject::connect( heartBeat, &bridge::HeartBeat::sgnUpdateListenerStatus,
                      this, &Receive::onSgnUpdateListenerStatus, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    updateAccountsData(wallet, ui->accountComboBox, true, true);

    if (!walletConfig->isFeatureMWCMQS()) {
        ui->frameMqs->hide();
        ui->mqsTorHorizontalLayout->layout()->removeItem( ui->spacer_mqs_tor );
        ui->label_mqs->hide();
        ui->mwcmqAddress->hide();

        ui->frameReceive->layout()->removeItem( ui->verticalSpacer_10 );
    }

    bool hasTor = walletConfig->isFeatureTor();
    if (!hasTor) {
        // hiding indicator
        ui->frameTor->hide();
        ui->mqsTorHorizontalLayout->layout()->removeItem( ui->spacer_mqs_tor );
    }
    bool hasSp = walletConfig->isFeatureSlatepack();
    if (!hasSp) {
        ui->recieveSlatepackButton->hide();
    }

    if (hasTor && hasSp) {
        // all good
    }
    else if (hasTor) {
        // Tor only
        ui->label_sp_tor->setText("Tor address");
    }
    else if (hasSp) {
        ui->label_sp_tor->setText("Slatepack address");
    }
    else {
        // no Tor and no SP
        ui->label_sp_tor->hide();
        ui->torAddress->hide();
    }

    if ( !config->isOnlineWallet() || config->isFaucetRequested()) {
        ui->requestFaucetMWCButton->hide();
    }

    ui->torAddress->setText( "http://" + wallet->getTorSlatepackAddress() + ".onion");
    QString address = wallet->getMqsAddress();
    mwcAddress = "mwcmqs://" + address;
    ui->mwcmqAddress->setText( mwcAddress );
    updateListenerStatus();
}

Receive::~Receive() {
    delete ui;
}

void Receive::updateListenerStatus() {
    // Recieve is a simple small page, we can update all without problems
    QVector<bool> listenerStatuses = wallet->getListenerStatus();
    Q_ASSERT(listenerStatuses.size()==4);
    updateListenerStatusWith(listenerStatuses[1], listenerStatuses[3]);
}

void Receive::updateListenerStatusWith(bool mqsOnline, bool torOnline) {
    ui->mwcmqStatusImg->setPixmap( QPixmap(mqsOnline ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->mwcmqStatusLabel->setText( mqsOnline ? "Online" : "Offline" );

    ui->torStatusImg->setPixmap( QPixmap(torOnline ? ":/img/StatusOk@2x.svg" : ":/img/StatusEmpty@2x.svg") );
    ui->torStatusLabel->setText( torOnline ? "Online" : "Offline" );
}

void Receive::on_recieveSlatepackButton_clicked()
{
    util::TimeoutLockObject to( "Receive" );

    dlg::InputSlatepackDlg inputSlateDlg("SendInitial", "Initial Send Slate",
                           util::FileTransactionType::RECEIVE,  this);

    if (inputSlateDlg.exec()  == QDialog::Accepted ) {
        receive->signSlatepackTransaction(inputSlateDlg.getSlatepack(), inputSlateDlg.getSlateJson(), inputSlateDlg.getSenderAddress() );
    }
}


void Receive::onSgnTransactionActionIsFinished( bool success, QString message ) {
    util::TimeoutLockObject to( "Receive" );

    ui->progress->hide();
    control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
}

void Receive::on_accountComboBox_activated(int index)
{
    Q_UNUSED(index);
    auto account = ui->accountComboBox->currentData(); // QVariant
    if (account.isValid())
        wallet->setReceiveAccountById( accountComboData2AccountPath(account.toString()).second );
}

void Receive::onSgnWalletBalanceUpdated() {
    updateAccountsData(wallet, ui->accountComboBox, true, true);
}

void Receive::onSgnUpdateListenerStatus(bool mqsOnline, bool torOnline) {
    Q_UNUSED(mqsOnline)
    Q_UNUSED(torOnline)
    updateListenerStatusWith(mqsOnline, torOnline);
}

void Receive::on_requestFaucetMWCButton_clicked()
{
    // even it is sync call, it is a long operation and wallet will continue to handle the events.
    // Not great design, but OK for single call at floonet
    ui->progress->show();
    if (wallet->requestFaucetMWC()) {
        config->faucetRequested();
        ui->requestFaucetMWCButton->hide();
    }
    ui->progress->hide();
}


}

