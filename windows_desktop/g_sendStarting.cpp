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
#include "../state/g_Send.h"
#include "../state/timeoutlock.h"
#include "../control/messagebox.h"
#include "../core/Config.h"

namespace wnd {


QString generateAmountErrorMsg(int64_t mwcAmount, const wallet::AccountInfo &acc, const core::SendCoinsParams &sendParams) {
    QString msg2print = "You are trying to send " + util::nano2one(mwcAmount) + " mwc, but you only have " +
                        util::nano2one(acc.currentlySpendable) + " spendable mwc.";
    if (acc.awaitingConfirmation > 0)
        msg2print += " " + util::nano2one(acc.awaitingConfirmation) + " coins are awaiting confirmation.";

    if (acc.lockedByPrevTransaction > 0)
        msg2print += " " + util::nano2one(acc.lockedByPrevTransaction) + " coins are locked.";

    if (acc.awaitingConfirmation > 0 || acc.lockedByPrevTransaction > 0) {
        if (sendParams.inputConfirmationNumber != 1) {
            if (sendParams.inputConfirmationNumber < 0)
                msg2print += " You can modify settings to spend mwc with less than 10 confirmations (wallet default value).";
            else
                msg2print += " You can modify settings to spend mwc with less than " +
                             QString::number(sendParams.inputConfirmationNumber) + " confirmations.";
        }
    }
    return msg2print;
}


enum CHECKED_FR_ID {
    ONLINE_ID = 1, FILE_ID = 2
};

SendStarting::SendStarting(QWidget *parent, state::Send *_state) :
        core::NavWnd(parent, _state->getContext() ),
        ui(new Ui::SendStarting),
        state(_state) {
    ui->setupUi(this);

    // Waiting for account data
    ui->progress->initLoader(true);

    ui->fileChecked->setId(FILE_ID);
    ui->onlineChecked->setId(ONLINE_ID);

    connect(ui->fileChecked, &control::MwcCheckedFrame::onChecked, this, &SendStarting::onChecked,
            Qt::QueuedConnection);
    connect(ui->onlineChecked, &control::MwcCheckedFrame::onChecked, this, &SendStarting::onChecked,
            Qt::QueuedConnection);

    if (config::isColdWallet()) {
        // Hide 'online option to send'
        ui->onlineChecked->hide();
        QRect rc = ui->fileChecked->frameGeometry();
        ui->fileChecked->move( QPoint( (rc.right() - rc.width())/2, rc.top() ) );

        onChecked(FILE_ID);
    }
    else {
        onChecked(ONLINE_ID);
    }

}

void SendStarting::updateAccountBalance( QVector<wallet::AccountInfo> _accountInfo, const QString & selectedAccount ) {
    // init accounts
    accountInfo = _accountInfo;

    ui->accountComboBox->clear();

    int selectedAccIdx = 0;
    int idx = 0;
    for (auto &info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem(info.getSpendableAccountName(), QVariant(idx++));
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
    state->destroyOnlineOfflineWnd(this);
    delete ui;
}

void SendStarting::on_nextButton_clicked() {
    state::TimeoutLockObject to( state );

    auto dt = ui->accountComboBox->currentData();
    if (!dt.isValid())
        return;

    int accountIdx = dt.toInt();
    wallet::AccountInfo acc = accountInfo[accountIdx];

    if (acc.currentlySpendable == 0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Your account doesn't have any spendable MWC to send");
        ui->accountComboBox->setFocus();
        return;
    }

    QString sendAmount = ui->amountEdit->text().trimmed();

    QPair<bool, int64_t> mwcAmount;
    if (sendAmount != "All") {
        mwcAmount = util::one2nano(ui->amountEdit->text().trimmed());
        if (!mwcAmount.first || mwcAmount.second<=0) {
            control::MessageBox::messageText(this, "Incorrect Input", "Please specify the number of MWC to send");
            ui->amountEdit->setFocus();
            return;
        }
    }
    else { // All
        mwcAmount = QPair<bool, int64_t>(true, -1);
    }

    // init expected to be fixed, so no need to disable the message
    if ( mwcAmount.second > acc.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, acc, state->getSendCoinsParams() );

        control::MessageBox::messageText(this, "Incorrect Input",
                                     msg2print );
        ui->amountEdit->setFocus();
        return;
    }

    state->processSendRequest( ui->onlineChecked->isChecked(), acc, mwcAmount.second );
}

void SendStarting::on_allAmountButton_clicked() {
    ui->amountEdit->setText("All");
}

void SendStarting::on_accountComboBox_currentIndexChanged(int index)
{
    auto dt = ui->accountComboBox->currentData();
    if (!dt.isValid())
        return;

    int accountIdx = dt.toInt();
    if (accountIdx>=0 && accountIdx<accountInfo.size()) {
        wallet::AccountInfo acc = accountInfo[accountIdx];
        state->switchAccount(acc.accountName);
    }
}

}

