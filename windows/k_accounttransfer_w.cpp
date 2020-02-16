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

#include "windows/k_accounttransfer_w.h"
#include "ui_k_accounttransfer_w.h"
#include "../control/messagebox.h"
#include "../state/k_AccountTransfer.h"
#include "../dialogs/sendcoinsparamsdialog.h"
#include "g_sendStarting.h"
#include "../state/timeoutlock.h"

namespace wnd {

AccountTransfer::AccountTransfer(QWidget *parent, state::AccountTransfer * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::AccountTransfer),
    state(_state)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);

    updateAccounts();
}

AccountTransfer::~AccountTransfer()
{
    state->wndDeleted(this);
    delete ui;
}


void AccountTransfer::updateAccounts() {
    accountInfo = state->getWalletBalance();

    int fromI = getAccountSelectionComboBoxCurrentIndex( ui->accountFromCB, false );
    int toI   = getAccountSelectionComboBoxCurrentIndex( ui->accountToCB, false );

    ui->accountFromCB->clear();
    ui->accountToCB->clear();

    int idx = 0;
    for (auto & info : accountInfo) {
        QString accountStr = info.getLongAccountName();
        ui->accountFromCB->addItem(accountStr, QVariant(idx) );
        ui->accountToCB->addItem(accountStr, QVariant(idx) );

        idx++;
    }
    ui->accountFromCB->setCurrentIndex(fromI);
    ui->accountToCB->setCurrentIndex(toI);
}


void AccountTransfer::showTransferResults(bool ok, QString errMsg) {

    ui->progress->hide();

    state::TimeoutLockObject to( state );

    if (ok) {
        control::MessageBox::messageText(this, "Success", "Your funds were successfully transferred");
        // reset state
        ui->amountEdit->setText("");
        updateAccounts();
    }
    else {
        control::MessageBox::messageText(this, "Transfer failure", "Funds transfer request has failed.\n" + errMsg);
    }
}


void AccountTransfer::on_allAmountButton_clicked()
{
    ui->amountEdit->setText("All");
}

void AccountTransfer::on_settingsBtn_clicked()
{
    core::SendCoinsParams  params = state->getSendCoinsParams();

    state::TimeoutLockObject to( state );

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }
}

// return -1 if not seleted or not valid
int AccountTransfer::getAccountSelectionComboBoxCurrentIndex( control::MwcComboBox * combo, bool showInputErrMessage ) {
    auto dt = combo->currentData();
    if ( !dt.isValid() ) {
        if (showInputErrMessage)
            control::MessageBox::messageText(this, "Incorrect Input", "Please select pair of accounts to transfer coins.");
        return -1;
    }

    int idxVal = dt.toInt();

    if (idxVal<0 || idxVal>accountInfo.size() ) {
        if (showInputErrMessage)
            control::MessageBox::messageText(this, "Incorrect Input", "Please select pair of different accounts to transfer coins.");
        return -1;
    }

    return idxVal;
}


void AccountTransfer::on_transferButton_clicked()
{
    state::TimeoutLockObject to( state );

    int fromI = getAccountSelectionComboBoxCurrentIndex( ui->accountFromCB, true );
    int toI   = getAccountSelectionComboBoxCurrentIndex( ui->accountToCB, true );

    if (fromI == toI) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please select pair of different accounts to transfer coins.");
        return;
    }

    QString sendAmount = ui->amountEdit->text().trimmed();

    QPair<bool, int64_t> mwcAmount;
    if (sendAmount != "All") {
        mwcAmount = util::one2nano(ui->amountEdit->text().trimmed());
        if (!mwcAmount.first) {
            control::MessageBox::messageText(this, "Incorrect Input", "Please specify correct number of MWC to send");
            ui->amountEdit->setFocus();
            return;
        }
    }
    else { // All
        mwcAmount = QPair<bool, int64_t>(true, -1);
    }

    auto & acc = accountInfo[fromI];
    if ( mwcAmount.second > acc.currentlySpendable ) {

        QString msg2print = generateAmountErrorMsg( mwcAmount.second, acc, state->getSendCoinsParams() );

        control::MessageBox::messageText(this, "Incorrect Input",
                                     msg2print );
        ui->amountEdit->setFocus();
        return;
    }


    ui->progress->show();
    state->transferFunds( accountInfo[fromI], accountInfo[toI] , mwcAmount.second );
}

void wnd::AccountTransfer::on_backButton_clicked()
{
    state->goBack();
}

}
