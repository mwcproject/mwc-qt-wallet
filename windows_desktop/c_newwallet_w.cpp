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

#include "c_newwallet_w.h"
#include "ui_c_newwallet.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/widgetutils.h"
#include "../bridge/wnd/a_initaccount_b.h"

// Need for enum consts
#include "../state/a_initaccount.h"

namespace wnd {

NewWallet::NewWallet(QWidget *parent) :
    core::PanelBaseWnd(parent),
    ui(new Ui::NewWallet)
{
    ui->setupUi(this);

    initAccount = new bridge::InitAccount(this);

    ui->radioCreateNew->setChecked(true);
    ui->radioMainNet->setChecked(true);
    updateControls();

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

void NewWallet::panelWndStarted() {
    ui->radioCreateNew->setFocus();
}

NewWallet::~NewWallet()
{
    delete ui;
}

void NewWallet::updateControls() {
    bool isNewChecked = ui->radioCreateNew->isChecked() || ui->radioHaveSeed->isChecked();
    ui->submitButton->setEnabled(isNewChecked);
}


void NewWallet::on_submitButton_clicked()
{
    Q_ASSERT(ui->radioCreateNew->isChecked() || ui->radioHaveSeed->isChecked());
    Q_ASSERT(ui->radioMainNet->isChecked() || ui->radioFloonet->isChecked());

    state::InitAccount::NEW_WALLET_CHOICE newWalletChoice = ui->radioCreateNew->isChecked() ? state::InitAccount::NEW_WALLET_CHOICE::CREATE_NEW : state::InitAccount::NEW_WALLET_CHOICE::CREATE_WITH_SEED;
    state::InitAccount::MWC_NETWORK       mwcNetworkChoice = ui->radioMainNet->isChecked() ? state::InitAccount::MWC_NETWORK::MWC_MAIN_NET      : state::InitAccount::MWC_NETWORK::MWC_FLOO_NET;

    initAccount->submitWalletCreateChoices(newWalletChoice, mwcNetworkChoice);
}

void NewWallet::on_radioHaveSeed_clicked()
{
    updateControls();
}

void NewWallet::on_radioCreateNew_clicked()
{
    updateControls();
}

}
