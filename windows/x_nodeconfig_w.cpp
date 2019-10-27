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

#include "windows/x_nodeconfig_w.h"
#include "ui_x_nodeconfig_w.h"
#include "../state/x_walletconfig.h"

namespace wnd {


NodeConfig::NodeConfig(QWidget *parent, state::WalletConfig * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::NodeConfig),
    state(_state)
{
    ui->setupUi(this);

    walletCfg = state->getWalletConfig();

    if (walletCfg.getNetwork().toLower().contains("main"))
        ui->radioMainNet->setChecked(true);
    else
        ui->radioFloonet->setChecked(true);

    updateApplyBtn();
}

NodeConfig::~NodeConfig()
{
    delete ui;
}

void NodeConfig::updateApplyBtn() {
    ui->applyButton->setEnabled( walletCfg.getNetwork() != getSelectedNetwork() );
}

QString NodeConfig::getSelectedNetwork() const {
    if ( ui->radioMainNet->isChecked() )
        return "Mainnet";
    else
        return "Floonet";
}


void NodeConfig::on_radioMainNet_clicked()
{
    updateApplyBtn();
}

void NodeConfig::on_radioFloonet_clicked()
{
    updateApplyBtn();
}

void NodeConfig::on_applyButton_clicked()
{
    // data path is expected to be updated on the start.
    walletCfg.setDataPathWithNetwork( "Fixme", getSelectedNetwork() );

    state->setWalletConfig(walletCfg, true);
    state->restartMwcQtWallet();
}


}
