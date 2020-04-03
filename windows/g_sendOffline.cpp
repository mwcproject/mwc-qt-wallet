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
#include "../dialogs/sendcoinsparamsdialog.h"
#include "state/g_Send.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include <QFileDialog>
#include "../util/ui.h"

namespace wnd {

SendOffline::SendOffline(QWidget *parent, const wallet::AccountInfo & _selectedAccount, int64_t _amount, state::Send * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::SendOffline),
    state(_state),
    selectedAccount(_selectedAccount),
    amount(_amount)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);

    ui->fromAccount->setText("From account: " + selectedAccount.accountName );
    ui->amount2send->setText( "Amount to send: " + (amount<0 ? "All" : util::nano2one(amount)) + " mwc" );
}

SendOffline::~SendOffline()
{
    state->destroyOfflineWnd(this);
    delete ui;
}

void wnd::SendOffline::on_settingsBtn_clicked()
{
    state::TimeoutLockObject to( state );

    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }
}

void SendOffline::on_sendButton_clicked()
{
    state::TimeoutLockObject to( state );

    if ( !state->isNodeHealthy() ) {
        control::MessageBox::messageText(this, "Unable to send", "Your MWC-Node, that wallet connected to, is not ready.\n"
                                                                     "MWC-Node need to be connected to few peers and finish blocks synchronization process");
        return;
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

    core::SendCoinsParams sendParams = state->getSendCoinsParams();

    QStringList outputs;
    if (! util::getOutputsToSend( selectedAccount.accountName, sendParams.changeOutputs, amount, state->getContext()->hodlStatus, this, outputs ) )
        return; // User reject something

    if ( control::MessageBox::RETURN_CODE::BTN2 != control::MessageBox::questionText(this,"Confirm Send request",
                         "You are sending " + (amount < 0 ? "all" : util::nano2one(amount)) +
                         " mwc offline.\nYour init transaction slate will be stored at the file.", "Decline", "Confirm", false, true, 1.0,
                         state->getWalletPassword(), control::MessageBox::RETURN_CODE::BTN2 ) )
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create initial transaction file"),
                                                    state->getFileGenerationPath(),
                                                    tr("MWC init transaction (*.tx)"));

    if (fileName.length()==0)
        return;

    if (!fileName.endsWith(".tx"))
        fileName += ".tx";

    // Update path
    QFileInfo flInfo(fileName);
    state->updateFileGenerationPath( flInfo.path() );

    ui->progress->show();

    state->sendMwcOffline(  selectedAccount, amount, description, fileName, outputs, sendParams.changeOutputs );
}

void SendOffline::showSendMwcOfflineResult( bool success, QString message ) {
    state::TimeoutLockObject to( state );

    ui->progress->hide();
    control::MessageBox::messageText(this, success ? "Success" : "Failure", message );
}




}

