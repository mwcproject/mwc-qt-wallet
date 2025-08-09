// Copyright 2025 The MWC Developers
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


#include "g_sendconfirmationspdlg.h"

#include <QThread>
#include "ui_g_sendconfirmationspdlg.h"
#include "../util/ui.h"
#include "../util/crypto.h"
#include "../util_desktop/widgetutils.h"
#include "../control_desktop/messagebox.h"
#include "../bridge/config_b.h"

namespace dlg {

SendConfirmationSlatePackDlg::SendConfirmationSlatePackDlg(QWidget *parent, QString title, QString _messageBody, double widthScale,
                                         int _inputsNum, int ttl, const QString & _passwordHash)
    : MwcDialog(parent),
    ui(new Ui::SendConfirmationSlatePackDlg),
    ttl_blocks(-1),
    passwordHash(_passwordHash),
    messageBody(_messageBody),
    inputsNum(_inputsNum)
{
    ui->setupUi(this);
    config = new bridge::Config(this);

    ui->title->setText(title);
    ui->TTLEdit->setText(QString::number(ttl));
    ui->outputsEdit->setText(QString::number( config->getChangeOutputs() ));

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

    updateMessageText();
    checkPasswordStatus();
}

SendConfirmationSlatePackDlg::~SendConfirmationSlatePackDlg()
{
    delete ui;
}

void SendConfirmationSlatePackDlg::updateMessageText() {
    int outputs = ui->outputsEdit->text().toInt();
    QString message = messageBody;
    if (outputs>0 && outputs <= 10) {
        uint64_t txnFee = util::calcTxnFee(inputsNum, outputs, 1);
        QString txnFeeStr = util::txnFeeToString(txnFee);
        message += "\n\nTransaction fee: " + txnFeeStr;
    }
    ui->text->setText(message);
}

void SendConfirmationSlatePackDlg::checkPasswordStatus() {
    QThread::msleep(200); // Ok for human and will prevent brute force from UI attack (really crasy scenario, better to attack mwc713 if you already get the host).
    bool ok = crypto::calcHSA256Hash(ui->passwordEdit->text()) == passwordHash;
    ui->confirmBtn->setEnabled(ok);
    if (ok)
        ui->confirmBtn->setFocus();
}

void SendConfirmationSlatePackDlg::on_confirmBtn_clicked()
{
    passwordHash = ui->passwordEdit->text();

    int ttl = ui->TTLEdit->text().toInt();
    if (ttl < 10) {
            control::MessageBox::messageText(this, "TTL value",
                                    "Please specify valid TTL value. Minimum valid TTL value is 10.");
            return;
    }
    ttl_blocks = ttl;

    int outs = ui->outputsEdit->text().toInt();
    if (!(outs>0 && outs <= 10)) {
        control::MessageBox::messageText(this, "Outputs value",
                                "Please specify outputs number from 1 to 10.");
        return;
    }

    config->updateSendCoinsParams(config->getInputConfirmationNumber(), outs);

    accept();
}


void SendConfirmationSlatePackDlg::on_declineBtn_clicked()
{
    reject();
}


void SendConfirmationSlatePackDlg::on_outputsEdit_textChanged(const QString &)
{
    updateMessageText();
}


void SendConfirmationSlatePackDlg::on_passwordEdit_textChanged(const QString &)
{
    checkPasswordStatus();
}

}
