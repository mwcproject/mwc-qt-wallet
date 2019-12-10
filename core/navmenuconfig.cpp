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

#include <state/state.h>
#include "navmenuconfig.h"
#include "ui_navmenuconfigwallet.h"
#include "ui_navmenuconfignode.h"
#include "ui_navmenuconfigcoldwlt.h"
#include "../state/statemachine.h"
#include "../core/appcontext.h"
#include "../core/Config.h"
#include "../control/messagebox.h"

namespace core {

NavMenuConfig::NavMenuConfig(QWidget *parent, state::StateContext * _context ) :
        NavMenu(parent),
        context(_context) {

    if (config::isOnlineNode()) {
        uiNode = new Ui::NavMenuConfigNode;
        uiNode->setupUi(this);
    }
    else if (config::isColdWallet()) {
        uiColdWallet  = new Ui::NavMenuConfigColdWlt;
        uiColdWallet ->setupUi(this);
    }
    else {
        uiWallet = new Ui::NavMenuConfigWallet;
        uiWallet->setupUi(this);

    }
}

NavMenuConfig::~NavMenuConfig() {
    if (uiWallet)
        delete uiWallet;

    if (uiNode)
        delete uiNode;
}

void NavMenuConfig::on_walletConfigButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::WALLET_CONFIG );
    close();
}

void NavMenuConfig::on_outputsButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::OUTPUTS );
    close();
}

void NavMenuConfig::on_mwcmqButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::LISTENING );
    close();
}

void NavMenuConfig::on_nodeOverviewButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::NODE_INFO );
    close();
}

void NavMenuConfig::on_selectRunningModeButton_clicked() {
    context->stateMachine->setActionWindow( state::STATE::WALLET_RUNNING_MODE );
    close();
}

void NavMenuConfig::on_resyncButton_clicked()
{
    if (control::MessageBox::questionText(this, "Re-sync account with a node", "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue",
                       "Yes", "No", false,true) == control::MessageBox::RETURN_CODE::BTN1 ) {
        // Starting resync

        context->appContext->pushCookie("PrevState", (int)context->appContext->getActiveWndState() );
        context->stateMachine->setActionWindow( state::STATE::RESYNC );
    }
    close();
}

}

