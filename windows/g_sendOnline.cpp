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
#include "../dialogs/sendcoinsparamsdialog.h"
#include "../control/messagebox.h"
#include "../util/address.h"
#include "../state/g_Send.h"
#include "../state/timeoutlock.h"
#include "../dialogs/w_selectcontact.h"

namespace wnd {

SendOnline::SendOnline(QWidget *parent,
        const wallet::AccountInfo & _selectedAccount, int64_t _amount,
        state::Send * _state, state::Contacts * _contactsState ) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::SendOnline),
    state(_state),
    contactsState(_contactsState ),
    selectedAccount(_selectedAccount),
    amount(_amount)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();
    ui->apiSecretEdit->hide();

    ui->fromAccount->setText("From account: " + selectedAccount.accountName );
    ui->amount2send->setText( "Amount to send: " + (amount<0 ? "All" : util::nano2one(amount)) + " mwc" );
}

SendOnline::~SendOnline()
{
    state->destroyOnlineWnd(this);
    delete ui;
}


void SendOnline::on_contactsButton_clicked()
{
    state::TimeoutLockObject to( state );

    // Get the contacts

    dlg::SelectContact dlg(this, contactsState);
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
    state::TimeoutLockObject to( state );

    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }
}

void SendOnline::on_sendButton_clicked()
{
    state::TimeoutLockObject to( state );

    if ( !state->isNodeHealthy() ) {
        control::MessageBox::messageText(this, "Unable to send", "Your MWC-Node, that wallet connected to, is not ready.\n"
                                                                     "MWC-Node need to be connected to few peers and finish blocks synchronization process");
        return;
    }


    QString sendTo = ui->sendEdit->text().trimmed();

    {
        QPair<bool, QString> valRes = util::validateMwc713Str(sendTo);
        if (!valRes.first) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes.second);
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

    // Check the address. Try contacts first
    QString address = sendTo;

    // Let's  verify address first
    QPair< bool, util::ADDRESS_TYPE > res = util::verifyAddress(address);
        if ( !res.first ) {
            control::MessageBox::messageText(this, "Incorrect Input",
                                         "Please specify correct address to send your MWC" );
            ui->sendEdit->setFocus();
            return;
    }

    QString apiSecret;
    if (res.second == util::ADDRESS_TYPE::HTTPS) {
        apiSecret = ui->apiSecretEdit->text();
        QPair<bool, QString> valRes = util::validateMwc713Str(apiSecret);
        if (!valRes.first) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes.second);
            ui->apiSecretEdit->setFocus();
            return;
        }
    }

    QString description = ui->descriptionEdit->toPlainText().trimmed();

    {
        QPair<bool, QString> valRes = util::validateMwc713Str(description);
        if (!valRes.first) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes.second);
            ui->descriptionEdit->setFocus();
            return;
        }
    }

    // Ask for confirmation
    if ( control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::questionText(this,"Confirm Send request",
                                  "You are sending " + (amount < 0 ? "all" : util::nano2one(amount)) + " mwc to address\n" + address, "Decline", "Confirm", false, true, state->getWalletPassword(), control::MessageBox::RETURN_CODE::BTN2 ) )
        return;

    ui->progress->show();

    state->sendMwcOnline( selectedAccount, res.second, address, amount, description, apiSecret );
}

void SendOnline::sendRespond( bool success, const QStringList & errors ) {
    state::TimeoutLockObject to( state );

    ui->progress->hide();

    if (success) {
        control::MessageBox::messageText(this, "Success", "Your mwc was successfully sent to recipient");
        ui->sendEdit->setText("");
        ui->descriptionEdit->setText("");
        return;
    }

    QString errMsg = util::formatErrorMessages(errors);

    if (errMsg.isEmpty())
        errMsg = "Your send request was failed by some reasons";
    else
        errMsg = "Your send request was failed:\n" + errMsg;

    control::MessageBox::messageText( this, "Send request failed", errMsg );
}

void SendOnline::on_sendEdit_textChanged(const QString & address)
{
    QPair< bool, util::ADDRESS_TYPE > res = util::verifyAddress(address);

    if (res.first && res.second==util::ADDRESS_TYPE::HTTPS)
        ui->apiSecretEdit->show();
    else
        ui->apiSecretEdit->hide();
}


}


