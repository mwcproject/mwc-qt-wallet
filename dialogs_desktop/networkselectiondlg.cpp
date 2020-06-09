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

#include "networkselectiondlg.h"
#include "ui_networkselectiondlg.h"


namespace dlg {

NetworkSelectionDlg::NetworkSelectionDlg(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::NetworkSelectionDlg)
{
    ui->setupUi(this);
    ui->radioMainNet->setChecked(true);
}

NetworkSelectionDlg::~NetworkSelectionDlg()
{
    delete ui;
}

void NetworkSelectionDlg::on_cancelButton_clicked()
{
    reject();
}

void NetworkSelectionDlg::on_submitButton_clicked()
{
    network = ui->radioMainNet->isChecked() ? state::InitAccount::MWC_MAIN_NET : state::InitAccount::MWC_FLOO_NET;
    accept();
}

}

