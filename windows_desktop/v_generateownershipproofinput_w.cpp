// Copyright 2024 The MWC Developers
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

#include "v_generateownershipproofinput_w.h"
#include "ui_v_generateownershipproofinput_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../bridge/wallet_b.h"

namespace wnd {

GenerateOwnershipProofInput::GenerateOwnershipProofInput(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::GenerateOwnershipProofInput) {
    ui->setupUi(this);

    viewAcounts = new bridge::ViewOutputs(this);

    updateGenerateBtn();
}

GenerateOwnershipProofInput::~GenerateOwnershipProofInput() {
    delete ui;
}


void GenerateOwnershipProofInput::on_back_clicked()
{
   viewAcounts->back();
}


void GenerateOwnershipProofInput::on_generate_clicked()
{
    QString message = ui->message2sign->toPlainText();
    bool isViewingKey = ui->select_viewing_key->isChecked();
    bool isTorAddress = ui->select_tor->isChecked();
    bool isMqsAddress = ui->select_mqs->isChecked();
    viewAcounts->generate_proof(message, isViewingKey, isTorAddress, isMqsAddress);
}


void GenerateOwnershipProofInput::on_message2sign_textChanged()
{
    updateGenerateBtn();
}


void GenerateOwnershipProofInput::on_select_viewing_key_stateChanged(int arg1)
{
    updateGenerateBtn();
}


void GenerateOwnershipProofInput::on_select_tor_stateChanged(int arg1)
{
    updateGenerateBtn();
}


void GenerateOwnershipProofInput::on_select_mqs_stateChanged(int arg1)
{
    updateGenerateBtn();
}

void GenerateOwnershipProofInput::updateGenerateBtn() {
    QString message = ui->message2sign->toPlainText();
    bool isViewingKey = ui->select_viewing_key->isChecked();
    bool isTorAddress = ui->select_tor->isChecked();
    bool isMqsAddress = ui->select_mqs->isChecked();

    bool canGenerate = (!message.isEmpty() && (isViewingKey || isTorAddress || isMqsAddress) );
    ui->generate->setEnabled(canGenerate);
}

}
