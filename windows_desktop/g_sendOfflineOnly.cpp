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

#include "g_sendOfflineOnly.h"
#include "ui_g_sendOfflineOnly.h"
#include "../dialogs_desktop/sendcoinsparamsdialog.h"
#include "../dialogs_desktop/w_selectcontact.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/g_send_b.h"
#include "zz_utils.h"

namespace wnd {

SendOfflineOnly::SendOfflineOnly(QWidget *parent) :
    NavWnd(parent),
    ui(new Ui::SendOfflineOnly)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);
    config = new bridge::Config(this);
    send = new bridge::Send(this);

    connect(wallet, &bridge::Wallet::sgnWalletBalanceUpdated, this, &SendOfflineOnly::onSgnWalletBalanceUpdated,
            Qt::QueuedConnection);
    connect( send, &bridge::Send::sgnShowSendResult,
                      this, &SendOfflineOnly::onSgnShowSendResult, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();

    updateAccountsData(wallet, ui->accountComboBox, true, false);

    ui->generatePoof->setCheckState( config->getGenerateProof() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
}

SendOfflineOnly::~SendOfflineOnly()
{
    delete ui;
}

void SendOfflineOnly::on_allAmountButton_clicked() {
    QString accountPath = accountComboData2AccountPath(ui->accountComboBox->currentData().toString()).second;
    if (accountPath.isEmpty())
        return;
    else {
        QString amount = send->getSpendAllAmount(accountPath);
        ui->amountEdit->setText(amount);
    }
}

void SendOfflineOnly::onSgnWalletBalanceUpdated() {
    updateAccountsData(wallet, ui->accountComboBox, true, false);
}

void SendOfflineOnly::on_accountComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString accountPath = accountComboData2AccountPath(ui->accountComboBox->currentData().toString()).second;
    if (accountPath.isEmpty())
        return;

    wallet->switchAccountById(accountPath);
}

static bool showGenProofWarning = false;

void SendOfflineOnly::on_generatePoof_clicked(bool checked)
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

void SendOfflineOnly::on_settingsBtn_clicked()
{
    util::TimeoutLockObject to("SendOfflineOnly");

    SendCoinsParamsDialog dlg(this, config->getInputConfirmationNumber(), config->getChangeOutputs());
    if (dlg.exec() == QDialog::Accepted) {
        config->updateSendCoinsParams(dlg.getInputConfirmationNumber(), dlg.getChangeOutputs());
    }
}

void SendOfflineOnly::on_sendButton_clicked()
{
    util::TimeoutLockObject to("SendOfflineOnly");

    auto accData = accountComboData2AccountPath(ui->accountComboBox->currentData().toString());
    QString accountName = accData.first;
    QString accountPath = accData.second;
    if (accountPath.isEmpty())
        return;

    QString sendAmount = ui->amountEdit->text().trimmed();

    config->setSendMethod(bridge::SEND_SELECTED_METHOD::SLATEPACK_ID);

    // Note, we don't go to the next page
    int res = send->initialSendSelection( bridge::SEND_SELECTED_METHOD::ONLINE_ID, accountPath, sendAmount, false );
    if (res==1) {
        ui->accountComboBox->setFocus();
        return;
    }
    else if (res==2) {
        ui->amountEdit->setFocus();
        return;
    }

    quint64 amount = send->getTmpAmount();
    accountPath = send->getTmpAccountPath();

    QString recipientWallet;
        recipientWallet = ui->recipientAddress->text();
        if ( !recipientWallet.isEmpty() && util->verifyAddress(recipientWallet) != "SP/Tor") {
            control::MessageBox::messageText(this, "Unable to send", "Please specify valid recipient wallet Slatepack address");
            ui->recipientAddress->setFocus();
            return;
        }

    if ( !send->isNodeHealthy() ) {
        control::MessageBox::messageText(this, "Unable to send", "Your MWC Node, that wallet is connected to, is not ready.\n"
                                                                     "MWC Node needs to be connected to a few peers and finish block synchronization process");
        return;
    }

    QString description = ui->descriptionEdit->text().trimmed().replace('\n', ' ');

    {
        QString valRes = util->validateMwc713Str(description);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    if ( send->sendMwcOffline( accountName, accountPath, QString::number(amount), description, config->getSendLockOutput(), recipientWallet) )
        ui->progress->show();
}

void SendOfflineOnly::onSgnShowSendResult( bool success, QString message ) {
    ui->progress->hide();
    control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
}

void SendOfflineOnly::on_contactsButton_clicked()
{
    util::TimeoutLockObject to("SendOfflineOnly");

    dlg::SelectContact dlg(this, true, false, false);
    if (dlg.exec() == QDialog::Accepted) {
        QString address = dlg.getSelectedContact().pub_key;
        ui->recipientAddress->setText(address);
        ui->contactNameLable->setText("     Contact: " + dlg.getSelectedContact().name );
        ui->contactNameLable->show();
    }
}

void SendOfflineOnly::on_recipientAddress_textEdited(const QString &)
{
    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();
}



}


