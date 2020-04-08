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
#include "../state/state.h"
#include "ui_navmenuaccount.h"
#include "../state/statemachine.h"
#include "../core/appcontext.h"
#include "../control/messagebox.h"


namespace core {

NavMenuAccount::NavMenuAccount(QWidget *parent, state::StateContext * _context) :
        NavMenu(parent),
        ui(new Ui::NavMenuAccount),
        context(_context)
{
    ui->setupUi(this);
}

NavMenuAccount::~NavMenuAccount() {
    delete ui;
}

void NavMenuAccount::on_accountsButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::ACCOUNTS );
    close();
}

void NavMenuAccount::on_seedButton_clicked()
{
    // need to logout first, than switch to the seed

    if (context->stateMachine->canSwitchState()) {

        QString password = context->wallet->getPassword();

        if ( !password.isEmpty() ) {
            if (control::MessageBox::RETURN_CODE::BTN2 !=
                control::MessageBox::questionText(this, "Wallet Password",
                                                  "You are going to view wallet mnemonic passphrase.\n\nPlease input your wallet password to continue", "Cancel", "Confirm", false, true, 1.0,
                                                  password, control::MessageBox::RETURN_CODE::BTN2))
                return;
        }

        context->stateMachine->setActionWindow( state::STATE::SHOW_SEED);
    }
    close();
}

void NavMenuAccount::on_contactsButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::CONTACTS );
    close();
}

void NavMenuAccount::on_logoutButton_clicked()
{
    context->stateMachine->logout();
}

}
