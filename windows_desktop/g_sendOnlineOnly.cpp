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

#include "g_sendOnlineOnly.h"
#include "ui_g_sendOnlineOnly.h"
#include "../dialogs_desktop/sendcoinsparamsdialog.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/w_selectcontact.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/g_send_b.h"
#include "../bridge/wnd/x_walletconfig_b.h"

namespace wnd {

SendOnlineOnly::SendOnlineOnly(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::SendOnlineOnly)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);
    config = new bridge::Config(this);
    walletConfig = new bridge::WalletConfig(this);
    send = new bridge::Send(this);

    QObject::connect(wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
            this, &SendOnlineOnly::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect( send, &bridge::Send::sgnShowSendResult,
                      this, &SendOnlineOnly::onSgnShowSendResult, Qt::QueuedConnection);

    ui->progress->initLoader(true);

    wallet->requestWalletBalanceUpdate();

    ui->generatePoof->setCheckState( config->getGenerateProof() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();

    QString formatStr = "FORMATS: ";
    if (walletConfig->isFeatureMWCMQS()) {
        formatStr += "   [mwcmqs://]<mqs_address>";
    }
    if (walletConfig->isFeatureTor()) {
        formatStr += "   [http://]<tor_address>[.onion]";
    }
    formatStr += "   http(s)://<host>:<port>";
    ui->formatsLable->setText(formatStr);
}

SendOnlineOnly::~SendOnlineOnly()
{
    delete ui;
}

void SendOnlineOnly::onSgnWalletBalanceUpdated() {
    // init accounts
    ui->accountComboBox->clear();

    QString account = wallet->getCurrentAccountName();
    QVector<QString> accountInfo = wallet->getWalletBalance(true, true,  false);

    int selectedAccIdx = 0;
    int idx = 0;

    for (int i=1; i<accountInfo.size(); i+=2) {
        if ( accountInfo[i-1] == "integrity")
            continue;

        if (accountInfo[i-1] == account)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( accountInfo[i], QVariant(accountInfo[i-1]));
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);

    ui->progress->hide();
}

void SendOnlineOnly::on_allAmountButton_clicked() {
    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;
    else {
        QString amount = send->getSpendAllAmount(account);
        ui->amountEdit->setText(amount);
    }
}

void SendOnlineOnly::on_accountComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;

    wallet->switchAccount(account);
}

static bool showGenProofWarning = false;

void SendOnlineOnly::on_generatePoof_clicked(bool checked)
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

void SendOnlineOnly::on_contactsButton_clicked()
{
    util::TimeoutLockObject to("SendOnlineOnly");

    // Get the contacts

    dlg::SelectContact dlg(this, walletConfig->isFeatureTor(), walletConfig->isFeatureMWCMQS(), true);
    if (dlg.exec() == QDialog::Accepted) {
        core::ContactRecord selectedContact = dlg.getSelectedContact();
        ui->sendEdit->setText( selectedContact.address );
        ui->contactNameLable->setText("     Contact: " + selectedContact.name );
        ui->contactNameLable->show();
        ui->formatsLable->hide();
    }
}

void SendOnlineOnly::on_sendEdit_textEdited(const QString &)
{
    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();
    ui->formatsLable->show();
}

void SendOnlineOnly::on_settingsBtn_clicked()
{
    util::TimeoutLockObject to("SendOnlineOnly");

    SendCoinsParamsDialog dlg(this, config->getInputConfirmationNumber(),
                config->getChangeOutputs());
    if (dlg.exec() == QDialog::Accepted) {
        config->updateSendCoinsParams( dlg.getInputConfirmationNumber(), dlg.getChangeOutputs() );
    }
}

void SendOnlineOnly::on_sendButton_clicked()
{
    util::TimeoutLockObject to("SendOnlineOnly");

    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;

    QString sendAmount = ui->amountEdit->text().trimmed();

    config->setSendMethod(bridge::SEND_SELECTED_METHOD::ONLINE_ID);

    // Note, we don't go to the next page
    int res = send->initialSendSelection( bridge::SEND_SELECTED_METHOD::ONLINE_ID, account, sendAmount, false );
    if (res==1) {
        ui->accountComboBox->setFocus();
        return;
    }
    else if (res==2) {
        ui->amountEdit->setFocus();
        return;
    }

    uint64_t amount = send->getTmpAmount();
    account = send->getTmpAccountName();

    if ( !send->isNodeHealthy() ) {
        control::MessageBox::messageText(this, "Unable to send", "Your MWC Node, that wallet connected to, is not ready.\n"
                                                                     "MWC Node needs to be connected to a few peers and finish block synchronization process");
        return;
    }

    QString sendTo = ui->sendEdit->text().trimmed();

    {
        QString valRes = util->validateMwc713Str(sendTo);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->sendEdit->setFocus();
            return;
        }
    }

    if (sendTo.size() == 0 ) {
        control::MessageBox::messageText(this, "Incorrect Input",
                                     "Please specify a valid address." );
        ui->sendEdit->setFocus();
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

    ui->progress->show();
    ui->sendButton->setEnabled(false);

    if (!send->sendMwcOnline( account, QString::number(amount), sendTo, "", description)) {
        ui->progress->hide();
        ui->sendButton->setEnabled(true);
    }
}

void SendOnlineOnly::onSgnShowSendResult( bool success, QString message ) {
    util::TimeoutLockObject to("SendOnlineOnly");

    ui->progress->hide();
    ui->sendButton->setEnabled(true);

    if (success) {
        control::MessageBox::messageText(this, "Success", "Your MWC was successfully sent to recipient");
        ui->sendEdit->setText("");
        ui->descriptionEdit->setText("");
        return;
    }

    control::MessageBox::messageText( this, "Send request has failed", message );
}


}


