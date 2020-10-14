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

#include "g_sendStarting.h"
#include "ui_g_sendStarting.h"
#include "../util_desktop/timeoutlock.h"
#include "../control_desktop/messagebox.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/g_send_b.h"

namespace wnd {

enum CHECKED_FR_ID {
    ONLINE_ID = 1, FILE_ID = 2
};

SendStarting::SendStarting(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::SendStarting) {
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    send = new bridge::Send(this);

    connect(wallet, &bridge::Wallet::sgnWalletBalanceUpdated, this, &SendStarting::onSgnWalletBalanceUpdated,
            Qt::QueuedConnection);

    // Waiting for account data
    ui->progress->initLoader(true);

    wallet->requestWalletBalanceUpdate();

    ui->fileChecked->setId(FILE_ID);
    ui->onlineChecked->setId(ONLINE_ID);

    connect(ui->fileChecked, &control::MwcCheckedFrame::onChecked, this, &SendStarting::onChecked,
            Qt::QueuedConnection);
    connect(ui->onlineChecked, &control::MwcCheckedFrame::onChecked, this, &SendStarting::onChecked,
            Qt::QueuedConnection);

    if (config->isColdWallet()) {
        // Hide 'online option to send'
        ui->onlineChecked->hide();
        QRect rc = ui->fileChecked->frameGeometry();
        ui->fileChecked->move( QPoint( (rc.right() - rc.width())/2, rc.top() ) );

        onChecked(FILE_ID);
    }
    else {
        onChecked(ONLINE_ID);
    }

    ui->generatePoof->setCheckState( config->getGenerateProof() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
}

void SendStarting::onSgnWalletBalanceUpdated() {
    // init accounts
    ui->accountComboBox->clear();

    QString account = wallet->getCurrentAccountName();
    QVector<QString> accountInfo = wallet->getWalletBalance(true, true,  false);

    int selectedAccIdx = 0;
    int idx = 0;

    for (int i=1; i<accountInfo.size(); i+=2) {
        if (accountInfo[i-1] == account)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( accountInfo[i], QVariant(accountInfo[i-1]));
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);

    ui->progress->hide();
}

void SendStarting::onChecked(int id) {
    if (id == ONLINE_ID) {
        ui->onlineChecked->setChecked(true);
        ui->fileChecked->setChecked(false);
        ui->fileLabel->hide();
        ui->onlineLabel->show();
    } else {
        ui->onlineChecked->setChecked(false);
        ui->fileChecked->setChecked(true);
        ui->fileLabel->show();
        ui->onlineLabel->hide();
    }
}


SendStarting::~SendStarting() {
    delete ui;
}

void SendStarting::on_nextButton_clicked() {
    util::TimeoutLockObject to( "SendStarting" );

    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;

    QString sendAmount = ui->amountEdit->text().trimmed();

    int res = send->initialSendSelection( ui->onlineChecked->isChecked(), account, sendAmount );
    if (res==1)
        ui->accountComboBox->setFocus();
    else if (res==2)
        ui->amountEdit->setFocus();
}

void SendStarting::on_allAmountButton_clicked() {
    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;
    else {
        QString amount = send->getSpendAllAmount(account);
        ui->amountEdit->setText(amount);
    }
}

void SendStarting::on_accountComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;

    wallet->switchAccount(account);
}

static bool showGenProofWarning = false;

void wnd::SendStarting::on_generatePoof_clicked(bool checked)
{
    if ( checked && !showGenProofWarning ) {
        if ( core::WndManager::RETURN_CODE::BTN2 !=  control::MessageBox::questionText(this, "Warning", "Transaction proof generation requires receiver wallet version 1.0.23 or higher.\n\n"
                                          "Do you want to generate proofs for all your send transactions?",
                                          "Cancel", "Generate proofs",
                                          "No, I don't want to generate proofs for my send transaction", "Yes, I am sure that receiver wallet is upgraded and I can generate proofs",
                                          false, true))
        {
            ui->generatePoof->setCheckState(Qt::CheckState::Unchecked);
            return;
        }

        showGenProofWarning  = true;
    }
    config->setGenerateProof(checked);
}

}


