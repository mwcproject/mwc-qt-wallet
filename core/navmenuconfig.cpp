#include <state/state.h>
#include "navmenuconfig.h"
#include "ui_navmenuconfig.h"
#include "../state/statemachine.h"
#include "../core/appcontext.h"
#include "../control/messagebox.h"

namespace core {

NavMenuConfig::NavMenuConfig(QWidget *parent, state::StateMachine * _stateMachine, core::AppContext * _appContext) :
        NavMenu(parent),
        ui(new Ui::NavMenuConfig),
        stateMachine(_stateMachine),
        appContext(_appContext) {
    ui->setupUi(this);
}

NavMenuConfig::~NavMenuConfig() {
    delete ui;
}

void NavMenuConfig::on_walletConfigButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::WALLET_CONFIG );
}

void NavMenuConfig::on_outputsButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::OUTPUTS );
}

void NavMenuConfig::on_mwcmqButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::LISTENING );
}

void NavMenuConfig::on_resyncButton_clicked()
{
    if (control::MessageBox::question(this, "Re-sync account with a node", "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue",
                       "Yes", "No", false,true) == control::MessageBox::BTN1 ) {
        // Starting resync

        appContext->pushCookie("PrevState", (int)appContext->getActiveWndState() );
        stateMachine->setActionWindow( state::STATE::RESYNC );
    }
}

}

