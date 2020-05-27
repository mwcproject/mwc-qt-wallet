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

#include "dialogs/h_hodlclaimwallet.h"
#include "ui_h_hodlclaimwallet.h"
#include "../util/crypto.h"
#include "../control/messagebox.h"

namespace dlg {

HodlClaimWallet::HodlClaimWallet(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::HodlClaimWallet)
{
    ui->setupUi(this);
    ui->claimForThisWalletCheck->setChecked(true);
    ui->coldWalletPublicKeyHash->setEnabled(false);
}

HodlClaimWallet::~HodlClaimWallet()
{
    delete ui;
}

void HodlClaimWallet::on_claimForThisWalletCheck_stateChanged(int state)
{
    Q_UNUSED(state)

    bool hashEnabled = !ui->claimForThisWalletCheck->isChecked();

    ui->coldWalletPublicKeyHash->setEnabled(hashEnabled);
    ui->coldWalletKeyHashLabel->setEnabled(hashEnabled);
    ui->coldWalletPublicKeyHashLable->setEnabled(hashEnabled);
}

void HodlClaimWallet::on_cancelButton_clicked()
{
    reject();
}

void HodlClaimWallet::on_continueButton_clicked()
{
    if (!ui->claimForThisWalletCheck->isChecked()) {
        QString hash = ui->coldWalletPublicKeyHash->text();
        if (!crypto::isHashValid(hash)) {
            control::MessageBox::messageText(this, "Incorrect Hash Value",
                   "Please input correct value for your Cold Wallet Public Key Hash");
            ui->coldWalletPublicKeyHash->setFocus();
            return;
        }

        coldWalletPubKeyHash = hash;
    }
    accept();
}

}

