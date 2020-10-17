// Copyright 2020 The MWC Developers
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

#include "e_showoutputdlg.h"
#include "ui_e_showoutputdlg.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"

namespace dlg {

ShowOutputDlg::ShowOutputDlg(QWidget *parent, const wallet::WalletOutput &output,
                             QString note,
                             bool canBeLocked, bool _locked) :
        control::MwcDialog(parent),
        ui(new Ui::ShowOutputDlg) {
    ui->setupUi(this);

    hodl = new bridge::HodlStatus(this);
    config = new bridge::Config(this);
    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);

    ui->status->setText(output.status);
    ui->height->setText(output.blockHeight);
    ui->confirms->setText(output.numOfConfirms);
    ui->mwc->setText(util::nano2one(output.valueNano));
    ui->locked->setText(output.lockedUntil);
    ui->coinBase->setText(output.coinbase ? "Yes" : "No");
    ui->tx->setText(QString::number(output.txIdx + 1));
    ui->commitment->setText(output.outputCommitment);

    blockExplorerUrl = config->getBlockExplorerUrl(config->getNetwork());

    if (!hodl->isInHodl("")) {
        ui->hodl->setText("N/A");
    }
    else {
        ui->hodl->setText( hodl->isOutputInHODL(output.outputCommitment) ? "Yes" : "No" );
    }

    commitment = output.outputCommitment;

    originalOutputNote = note;
    newOutputNote = note;
    ui->outputNote->setText(newOutputNote);

    locked = _locked;
    ui->lockOutput->setChecked(locked);
    ui->lockOutput->setVisible(canBeLocked && output.isUnspent());
}

ShowOutputDlg::~ShowOutputDlg() {
    delete ui;
}

void ShowOutputDlg::on_viewOutput_clicked() {
    util->openUrlInBrowser("https://" + blockExplorerUrl + "/#o" + commitment);
}

void ShowOutputDlg::on_okButton_clicked() {
    newOutputNote = ui->outputNote->text();
    locked = ui->lockOutput->isChecked();
    accept();
}

void dlg::ShowOutputDlg::on_cancelButton_clicked()
{
    reject();
}

}

