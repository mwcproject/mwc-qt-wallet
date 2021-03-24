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

#include "navmenuaccount.h"
#include "ui_navmenuaccount.h"
#include "../control_desktop/messagebox.h"
#include "../bridge/statemachine_b.h"
#include "../bridge/wallet_b.h"
#include "../state/state.h"

namespace core {

NavMenuAccount::NavMenuAccount(QWidget *parent) :
        NavMenu(parent),
        ui(new Ui::NavMenuAccount)
{
    ui->setupUi(this);
    stateMachine = new bridge::StateMachine(this);
    wallet = new bridge::Wallet(this);
}

NavMenuAccount::~NavMenuAccount() {
    delete ui;
}

void NavMenuAccount::on_accountsButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::ACCOUNTS );
    close();
}

void NavMenuAccount::on_seedButton_clicked()
{
    // need to logout first, than switch to the seed

    if (stateMachine->canSwitchState(state::STATE::SHOW_SEED)) {

        QString passwordHash = wallet->getPasswordHash();

        if ( !passwordHash.isEmpty() ) {
            if (WndManager::RETURN_CODE::BTN2 !=
                control::MessageBox::questionText(this, "Wallet Password",
                                                  "You are going to view wallet mnemonic passphrase.", "Cancel", "Confirm",
                                                  "Cancel operation", "Verify password and show mnemonic passphrase",
                                                  false, true, 1.0,
                                                  passwordHash, WndManager::RETURN_CODE::BTN2))
                return;
        }
        // passwordHash should contain raw password value form the messgage box
        stateMachine->activateShowSeed(passwordHash);
    }
    close();
}

void NavMenuAccount::on_contactsButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::CONTACTS );
    close();
}

void NavMenuAccount::on_logoutButton_clicked()
{
    stateMachine->logout();
}

}
