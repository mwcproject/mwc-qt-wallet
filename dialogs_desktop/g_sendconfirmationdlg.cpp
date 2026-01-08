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

#include "g_sendconfirmationdlg.h"
#include "ui_g_sendconfirmationdlg.h"
#include <Qt>
#include <QTextDocument>
#include <QScreen>
#include <QTextBlock>
#include <QScrollBar>
#include <QThread>
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../util_desktop/widgetutils.h"
#include "../util/ui.h"
#include "../util/crypto.h"
#include "../control_desktop/messagebox.h"

namespace dlg {

SendConfirmationDlg::SendConfirmationDlg( QWidget *parent, QString title, QString message, double widthScale,
                                         int _inputsNum ) :
     MwcDialog(parent),
    ui(new Ui::SendConfirmationDlg),
    messageBody(message),
    inputsNum(_inputsNum)
{
    ui->setupUi(this);
    util = new bridge::Util(this);
    config = new bridge::Config(this);
    wallet = new bridge::Wallet(this);


    if (widthScale!=1.0) {
        // Let's ujust Width first
        int w = maximumWidth();
        w = int(w*widthScale);
        setMaximumWidth(w);

        QSize sz = size();
        sz.setWidth( sz.width()*widthScale );
        resize(sz);
        setMinimumWidth(sz.width());

        adjustSize();
    }

    ui->title->setText(title);

    utils::resizeEditByContent(this, ui->text, false, message);

    ui->fluffCheckBox->setCheckState( config->isFluffSet() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

    ui->declineButton->setFocus();
    ui->confirmButton->setFocus();

    ui->confirmButton->adjustSize();
    ui->declineButton->adjustSize();

    ui->outputsEdit->setText(QString::number( config->getChangeOutputs() ));

    adjustSize();

    updateMessageText();
    checkPasswordStatus();
}

SendConfirmationDlg::~SendConfirmationDlg()
{
    delete ui;
}

void SendConfirmationDlg::updateMessageText() {
    int outputs = ui->outputsEdit->text().toInt();
    QString message = messageBody;
    if (outputs>0 && outputs <= 10) {
        uint64_t txnFee = util::calcTxnFee(inputsNum, outputs, 1);
        QString txnFeeStr = util::txnFeeToString(txnFee);
        message += "\n\nTransaction fee: " + txnFeeStr;
    }
    ui->text->setText(message);
}

void SendConfirmationDlg::checkPasswordStatus() {
    bool ok = wallet->checkPassword(ui->passwordEdit->text());
    ui->confirmButton->setEnabled(ok);
    if (ok)
        ui->confirmButton->setFocus();
}


void SendConfirmationDlg::on_passwordEdit_textChanged(const QString &)
{
    checkPasswordStatus();
}

void SendConfirmationDlg::on_declineButton_clicked()
{
    reject();
}

void SendConfirmationDlg::on_confirmButton_clicked()
{
    int outs = ui->outputsEdit->text().toInt();
    if (!(outs>0 && outs <= 10)) {
        control::MessageBox::messageText(this, "Outputs value",
                                         "Please specify outputs number from 1 to 10.");
        return;
    }
    config->updateSendCoinsParams(config->getInputConfirmationNumber(), outs);

    config->setFluff(ui->fluffCheckBox->isChecked());
    accept();
}

void SendConfirmationDlg::on_outputsEdit_textChanged(const QString &)
{
    updateMessageText();
}



}

