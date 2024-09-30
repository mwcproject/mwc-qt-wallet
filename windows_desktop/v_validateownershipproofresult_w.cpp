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

#include "v_validateownershipproofresult_w.h"
#include "ui_v_validateownershipproofresult_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../bridge/wallet_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

ValidateOwnershipProofResult::ValidateOwnershipProofResult(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::ValidateOwnershipProofResult)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    viewAcounts = new bridge::ViewOutputs(this);

    QObject::connect(wallet, &bridge::Wallet::sgnValidateOwnershipProof,
                             this, &ValidateOwnershipProofResult::sgnValidateOwnershipProof, Qt::QueuedConnection);
}

ValidateOwnershipProofResult::~ValidateOwnershipProofResult() {
    delete ui;
}

void ValidateOwnershipProofResult::sgnValidateOwnershipProof(QString network, QString message, QString viewingKey, QString torAddress, QString mqsAddress, QString error) {
    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Unable to validate ownership proof.\n\nError:" + error);
        viewAcounts->back();
    }

    ui->network->setText(network);
    ui->message->setText(message);
    ui->viewing_key->setText(viewingKey);
    ui->tor_address->setText(torAddress);
    ui->mqs_address->setText(mqsAddress);
}

void ValidateOwnershipProofResult::on_back_clicked() {
    viewAcounts->back();
}

}
