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

#include "dialogs/h_hodlgetsignature.h"
#include "ui_h_hodlgetsignature.h"
#include "../util/crypto.h"

namespace dlg {

HodlGetSignature::HodlGetSignature(QWidget *parent, QString message) :
    control::MwcDialog(parent),
    ui(new Ui::HodlGetSignature)
{
    ui->setupUi(this);
    ui->message->setText(message);
    ui->continueButton->setEnabled(false);
}

HodlGetSignature::~HodlGetSignature()
{
    delete ui;
}

void HodlGetSignature::on_cancelButton_clicked()
{
    reject();
}

void HodlGetSignature::on_continueButton_clicked()
{
    signature = ui->signature->toPlainText();
    accept();
}

void HodlGetSignature::on_signature_textChanged()
{
    ui->continueButton->setEnabled( crypto::isSignatureValid(ui->signature->toPlainText()));

}

}

