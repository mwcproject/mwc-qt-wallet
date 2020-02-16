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
        // State where to go after login
        context->appContext->setActiveWndState(state::STATE::SHOW_SEED);
        context->wallet->logout(true);
        context->stateMachine->executeFrom(state::STATE::NONE);
    }
    close();
}

void NavMenuAccount::on_contactsButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::CONTACTS );
    close();
}

}

