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

#include "y_selectmode_w.h"
#include "ui_y_selectmode.h"
#include "../bridge/wnd/y_selectmode_b.h"
#include "../core/Config.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

SelectMode::SelectMode(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::SelectMode)
{
    ui->setupUi(this);
    selectMode = new bridge::SelectMode(this);

    runMode = selectMode->getWalletRunMode();

    switch (runMode) {
        case int(config::WALLET_RUN_MODE::ONLINE_WALLET): {
            ui->radioOnlineWallet->setChecked(true);
            break;
        }
        case int(config::WALLET_RUN_MODE::ONLINE_NODE): {
            ui->radioOnlineNode->setChecked(true);
            break;
        }
        case int(config::WALLET_RUN_MODE::COLD_WALLET): {
            ui->radioColdWallet->setChecked(true);
            break;
        }
        default:
            Q_ASSERT(false);
    }

    updateButtons();
}

SelectMode::~SelectMode() {
    delete ui;
}

int SelectMode::getRunMode() const {
    if ( ui->radioOnlineWallet->isChecked() )
        return int(config::WALLET_RUN_MODE::ONLINE_WALLET);
    if ( ui->radioOnlineNode->isChecked() )
        return int(config::WALLET_RUN_MODE::ONLINE_NODE);
    if ( ui->radioColdWallet->isChecked() )
        return int(config::WALLET_RUN_MODE::COLD_WALLET);

    Q_ASSERT(false);
    return int(config::WALLET_RUN_MODE::ONLINE_WALLET);
}

void SelectMode::updateButtons() {
    ui->applyButton->setEnabled( runMode != getRunMode() );
}

void SelectMode::on_applyButton_clicked() {
    if ( core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText(this, "Waller Run Mode",
            "Changing running mode required restart.",
            "Cancel", "Continue",
            "Drop my request and don't change running mode",
            "Continue, change the running mode and restart this app",
            false, true) ) {
        selectMode->updateWalletRunMode(SelectMode::getRunMode());
    }
}

void SelectMode::on_radioOnlineWallet_clicked() {
    updateButtons();
}

void SelectMode::on_radioOnlineNode_clicked() {
    updateButtons();
}

void SelectMode::on_radioColdWallet_clicked() {
    updateButtons();
}


}
