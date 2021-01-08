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

#include "g_sendOffline.h"
#include "ui_g_sendOffline.h"
#include "../dialogs_desktop/sendcoinsparamsdialog.h"
#include "../dialogs_desktop/w_selectcontact.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/g_send_b.h"

namespace wnd {

SendOffline::SendOffline(QWidget *parent, QString _selectedAccount, int64_t _amount, bool _slatepacks) :
    core::NavWnd(parent),
    ui(new Ui::SendOffline),
    selectedAccount(_selectedAccount),
    amount(_amount),
    slatepacks(_slatepacks)
{
    ui->setupUi(this);

    util = new bridge::Util(this);
    config = new bridge::Config(this);
    send = new bridge::Send(this);

    QObject::connect( send, &bridge::Send::sgnShowSendResult,
                      this, &SendOffline::onSgnShowSendResult, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->fromAccount->setText("From account: " + selectedAccount );
    ui->amount2send->setText( "Amount to send: " + (amount<0 ? "All" : util->nano2one(QString::number(amount))) + " MWC" );

    if (!slatepacks) {
        ui->recipientAddress->hide();
        ui->contactsButton->hide();

        ui->sendTitleLabel->setText("Send to File");
    }
    else {
        ui->sendTitleLabel->setText("Send to Slatepack");
    }
}

SendOffline::~SendOffline()
{
    delete ui;
}

void SendOffline::on_settingsBtn_clicked()
{
    util::TimeoutLockObject to("SendOffline");

    SendCoinsParamsDialog dlg(this, config->getInputConfirmationNumber(), config->getChangeOutputs());
    if (dlg.exec() == QDialog::Accepted) {
        config->updateSendCoinsParams(dlg.getInputConfirmationNumber(), dlg.getChangeOutputs());
    }
}

void SendOffline::on_sendButton_clicked()
{
    util::TimeoutLockObject to("SendOffline");

    QString recipientWallet;
    if (slatepacks) {
        recipientWallet = ui->recipientAddress->text();
        if ( !recipientWallet.isEmpty() && util->verifyAddress(recipientWallet) != "tor") {
            control::MessageBox::messageText(this, "Unable to send", "Please specify valid recipient wallet address");
            ui->recipientAddress->setFocus();
            return;
        }
    }

    if ( !send->isNodeHealthy() ) {
        control::MessageBox::messageText(this, "Unable to send", "Your MWC Node, that wallet is connected to, is not ready.\n"
                                                                     "MWC Node needs to be connected to a few peers and finish block synchronization process");
        return;
    }

    QString description = ui->descriptionEdit->toPlainText().trimmed().replace('\n', ' ');

    {
        QString valRes = util->validateMwc713Str(description);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    if ( send->sendMwcOffline( selectedAccount, QString::number(amount), description, slatepacks, config->getSendLockOutput(), recipientWallet) )
        ui->progress->show();
}

void SendOffline::onSgnShowSendResult( bool success, QString message ) {
    ui->progress->hide();
    control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
}

void SendOffline::on_contactsButton_clicked()
{
    util::TimeoutLockObject to("SendOffline");

    dlg::SelectContact dlg(this, true, false, false);
    if (dlg.exec() == QDialog::Accepted) {
        QString address = dlg.getSelectedContact().pub_key;
        ui->recipientAddress->setText(address);
    }
}


}


