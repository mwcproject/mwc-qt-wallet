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

#include "v_viewhash_w.h"
#include "ui_v_viewhash_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../bridge/wallet_b.h"
#include <QPushButton>
#include "../control_desktop/messagebox.h"

namespace wnd {

ViewHash::ViewHash(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::ViewHash)
{
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    viewAcounts = new bridge::ViewOutputs(this);

    QObject::connect( wallet, &bridge::Wallet::sgnGetViewingKey,
                      this, &ViewHash::onSgnGetViewingKey, Qt::QueuedConnection);

    ui->walletViewingKeyLable->hide();
    ui->walletViewingKey->hide();
    ui->showWalletKeyButton->show();

    ui->startScanning->setEnabled(false);

    ui->viewingKey->setText( viewAcounts->getLastViewViewingKey() );
}

ViewHash::~ViewHash()
{
    delete ui;
}

void ViewHash::on_showWalletKeyButton_clicked()
{
    wallet->getViewingKey();
    ui->walletViewingKeyLable->show();
    ui->walletViewingKey->show();
    ui->walletViewingKey->setText("Retrieving Viewing Key, please wait...");
    ui->showWalletKeyButton->hide();
}

void ViewHash::onSgnGetViewingKey(QString viewingKey, QString error) {
    if (viewingKey.isEmpty()) {
        // Error,
        ui->walletViewingKeyLable->hide();
        ui->walletViewingKey->hide();
        ui->showWalletKeyButton->show();

        control::MessageBox::messageText( this, "Error", "Unable to retrieve the wallet viewing key.\n\n" + error);
    }
    else {
        ui->walletViewingKeyLable->show();
        ui->walletViewingKey->show();
        ui->showWalletKeyButton->hide();
        ui->walletViewingKey->setText(viewingKey);
    }
}

static bool validateHashKey(const QString & hash) {
    if (hash.size() == strlen("fbe0ab0d60e971d925c1eeb572eb037233872e4e5a3a47c3755989402faf13f6")) {
        QString lhash = hash.toLower();
        for (QChar ch : lhash) {
            if (!(ch.isDigit() || (ch >= 'a' && ch <= 'f'))) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void ViewHash::on_startScanning_clicked()
{
    QString hash = ui->viewingKey->text().trimmed();
    if (validateHashKey(hash)) {
        viewAcounts->startScanning(hash);
    }
    else {
        control::MessageBox::messageText( this, "Invalid Key", "Viewing key "+hash+" is invalid.\nPlease input valid Viewing Key.");
        ui->startScanning->setEnabled(false);
    }
}

void ViewHash::on_viewingKey_textChanged(const QString &hash)
{
    ui->startScanning->setEnabled(!hash.isEmpty());
}


void ViewHash::on_generateOwnershipProof_clicked()
{
    viewAcounts->generateOwnershipProofStart();
}


void ViewHash::on_viewOwnershipProof_clicked()
{
    viewAcounts->validateOwnershipProofStart();
}

}
