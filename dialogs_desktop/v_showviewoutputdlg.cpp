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

#include "v_showviewoutputdlg.h"
#include "ui_v_showviewoutputdlg.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"

namespace dlg {

ShowViewOutputDlg::ShowViewOutputDlg(QWidget *parent, const wallet::WalletOutput &output) :
        control::MwcDialog(parent),
        ui(new Ui::ShowViewOutputDlg) {

    ui->setupUi(this);

    config = new bridge::Config(this);
    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);

    ui->height->setText(output.blockHeight);
    ui->confirms->setText(output.numOfConfirms);
    ui->mwc->setText(util::nano2one(output.valueNano));
    ui->locked->setText(output.lockedUntil);
    ui->coinBase->setText(output.coinbase ? "Yes" : "No");
    ui->commitment->setText(output.outputCommitment);

    blockExplorerUrl = config->getBlockExplorerUrl(config->getNetwork());

    commitment = output.outputCommitment;
}

ShowViewOutputDlg::~ShowViewOutputDlg() {
    delete ui;
}

void ShowViewOutputDlg::on_viewOutput_clicked() {
    util->openUrlInBrowser("https://" + blockExplorerUrl + "/#o" + commitment);
}

void ShowViewOutputDlg::on_okButton_clicked() {
    accept();
}

}

