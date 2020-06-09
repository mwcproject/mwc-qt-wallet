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

#include "k_accounttransfer_w.h"
#include "ui_k_accounttransfer_w.h"
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/sendcoinsparamsdialog.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/k_accounttransfer_b.h"

namespace wnd {

AccountTransfer::AccountTransfer(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::AccountTransfer)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    accountTransfer = new bridge::AccountTransfer(this);

    QObject::connect( accountTransfer, &bridge::AccountTransfer::sgnShowTransferResults,
                      this, &AccountTransfer::onSgnShowTransferResults, Qt::QueuedConnection);
    QObject::connect( accountTransfer, &bridge::AccountTransfer::sgnUpdateAccounts,
                      this, &AccountTransfer::onSgnUpdateAccounts, Qt::QueuedConnection);
    QObject::connect( accountTransfer, &bridge::AccountTransfer::sgnHideProgress,
                      this, &AccountTransfer::onSgnHideProgress, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    onSgnUpdateAccounts();
}

AccountTransfer::~AccountTransfer()
{
    delete ui;
}


void AccountTransfer::onSgnUpdateAccounts() {
    QVector<QString> accountInfo = wallet->getWalletBalance(true, false, true);

    QString fromAcc = getSelectedAccount( ui->accountFromCB, false );
    QString toAcc   = getSelectedAccount( ui->accountToCB, false );

    ui->accountFromCB->clear();
    ui->accountToCB->clear();

    int fromI = -1;
    int toI = -1;

    int idx = 0;
    for ( int t=1; t<accountInfo.size(); t+=2 ) {
        ui->accountFromCB->addItem(accountInfo[t], QVariant(accountInfo[t-1]) );
        ui->accountToCB->addItem(accountInfo[t], QVariant(accountInfo[t-1]) );
        if (accountInfo[t]==fromAcc)
            fromI = idx;
        if (accountInfo[t]==toAcc)
            toI = idx;
        idx++;
    }
    ui->accountFromCB->setCurrentIndex(fromI);
    ui->accountToCB->setCurrentIndex(toI);
}


void AccountTransfer::onSgnShowTransferResults(bool ok, QString errMsg) {

    ui->progress->hide();

    util::TimeoutLockObject to( "AccountTransfer" );

    if (ok) {
        control::MessageBox::messageText(this, "Success", "Your funds were successfully transferred");
        // reset state
        ui->amountEdit->setText("");
        onSgnUpdateAccounts();
    }
    else {
        control::MessageBox::messageText(this, "Transfer failure", "Funds transfer request has failed.\n" + errMsg);
    }
}

void AccountTransfer::onSgnHideProgress() {
    ui->progress->hide();
}

void AccountTransfer::on_allAmountButton_clicked()
{
    ui->amountEdit->setText("All");
}

void AccountTransfer::on_settingsBtn_clicked()
{
    util::TimeoutLockObject to( "AccountTransfer" );

    SendCoinsParamsDialog dlg(this, config->getInputConfirmationNumber(), config->getChangeOutputs());
    if (dlg.exec() == QDialog::Accepted) {
        config->updateSendCoinsParams( dlg.getInputConfirmationNumber(), dlg.getChangeOutputs() );
    }
}

// return -1 if not seleted or not valid
QString AccountTransfer::getSelectedAccount( control::MwcComboBox * combo, bool showInputErrMessage ) {
    auto acc = combo->currentData().toString();
    if ( acc.isEmpty() ) {
        if (showInputErrMessage) {
            control::MessageBox::messageText(this, "Incorrect Input",
                                             "Please select pair of accounts to transfer coins.");
            combo->setFocus();
        }
    }
    return acc;
}


void AccountTransfer::on_transferButton_clicked()
{
    util::TimeoutLockObject to( "AccountTransfer" );

    QString fromAcc = getSelectedAccount( ui->accountFromCB, true );
    QString toAcc   = getSelectedAccount( ui->accountToCB, true );

    if (fromAcc.isEmpty() || toAcc.isEmpty() )
        return; // erro message wwas shown to the user. Just exiting...

    if (fromAcc == toAcc) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please select pair of different accounts to transfer coins.");
        return;
    }

    if (accountTransfer->transferFunds(fromAcc, toAcc, ui->amountEdit->text().trimmed())) {
        ui->progress->show();
    }
}

void wnd::AccountTransfer::on_backButton_clicked()
{
    accountTransfer->goBack();
}

}
