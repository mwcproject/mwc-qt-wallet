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

#include "g_sendOnline.h"
#include "ui_g_sendOnline.h"
#include "../dialogs_desktop/sendcoinsparamsdialog.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/g_sendconfirmationdlg.h"
#include "../dialogs_desktop/w_selectcontact.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/g_send_b.h"

namespace wnd {

SendOnline::SendOnline(QWidget *parent,
                       QString _account, int64_t _amount) :
    core::NavWnd(parent),
    ui(new Ui::SendOnline),
    account(_account),
    amount(_amount)
{
    ui->setupUi(this);

    util = new bridge::Util(this);
    config = new bridge::Config(this);
    send = new bridge::Send(this);

    QObject::connect( send, &bridge::Send::sgnShowSendResult,
                      this, &SendOnline::onSgnShowSendResult, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();
    ui->apiSecretEdit->hide();

    ui->fromAccount->setText("From account: " + account );
    ui->amount2send->setText( "Amount to send: " + (amount<0 ? "All" : util->nano2one(QString::number(amount))) + " MWC" );
}

SendOnline::~SendOnline()
{
    delete ui;
}


void SendOnline::on_contactsButton_clicked()
{
    util::TimeoutLockObject to("SendOnline");

    // Get the contacts

    dlg::SelectContact dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        core::ContactRecord selectedContact = dlg.getSelectedContact();
        ui->sendEdit->setText( selectedContact.address );
        ui->contactNameLable->setText("     Contact: " + selectedContact.name );
        ui->contactNameLable->show();
        ui->formatsLable->hide();
    }
}


void SendOnline::on_sendEdit_textEdited(const QString &)
{
    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();
    ui->formatsLable->show();
}

void SendOnline::on_settingsBtn_clicked()
{
    util::TimeoutLockObject to("SendOnline");

    SendCoinsParamsDialog dlg(this, config->getInputConfirmationNumber(),
                config->getChangeOutputs());
    if (dlg.exec() == QDialog::Accepted) {
        config->updateSendCoinsParams( dlg.getInputConfirmationNumber(), dlg.getChangeOutputs() );
    }
}

void SendOnline::on_sendButton_clicked()
{
    util::TimeoutLockObject to("SendOnline");

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

    QString description = ui->descriptionEdit->toPlainText().trimmed();

    {
        QString valRes = util->validateMwc713Str(description);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    QString apiSecret = ui->apiSecretEdit->text();
    {
        QString valRes = util->validateMwc713Str(apiSecret);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->apiSecretEdit->setFocus();
            return;
        }
    }

    ui->progress->show();
    if (!send->sendMwcOnline( account, QString::number(amount), sendTo, apiSecret, description))
        ui->progress->hide();
}

void SendOnline::onSgnShowSendResult( bool success, QString message ) {
    util::TimeoutLockObject to("SendOnline");

    ui->progress->hide();

    if (success) {
        control::MessageBox::messageText(this, "Success", "Your MWC was successfully sent to recipient");
        ui->sendEdit->setText("");
        ui->descriptionEdit->setText("");
        return;
    }

    control::MessageBox::messageText( this, "Send request failed", message );
}

void SendOnline::on_sendEdit_textChanged(const QString & address)
{
    // NOTE!!!!  Foeign API can be protected with a secret. But because that secret is shared with owner API,
    // it is impossible to share it. Becasue of that there is no reason for QT wallet to support that case. It adds confusion.
/*    QString addrType = util->verifyAddress(address);
    if ( addrType == "https" || addrType=="tor" )
        ui->apiSecretEdit->show();
    else
        ui->apiSecretEdit->hide();*/
}


}


