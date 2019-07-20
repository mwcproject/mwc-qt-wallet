#include <state/state.h>
#include "navmenuconfig.h"
#include "ui_navmenuconfig.h"
#include "../state/statemachine.h"
#include "../core/appcontext.h"
#include "../control/messagebox.h"

namespace core {

NavMenuConfig::NavMenuConfig(QWidget *parent, state::StateContext * _context ) :
        NavMenu(parent),
        ui(new Ui::NavMenuConfig),
        context(_context) {
    ui->setupUi(this);
}

NavMenuConfig::~NavMenuConfig() {
    delete ui;
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

void NavMenuConfig::on_resyncButton_clicked()
{
    if (control::MessageBox::question(this, "Re-sync account with a node", "Account re-sync will validate transactions and outputs for your accounts. Re-sync can take several minutes.\nWould you like to continue",
                       "Yes", "No", false,true) == control::MessageBox::BTN1 ) {
        // Starting resync

        context->appContext->pushCookie("PrevState", (int)context->appContext->getActiveWndState() );
        context->stateMachine->setActionWindow( state::STATE::RESYNC );
    }
    close();
}

}

