#include <state/state.h>
#include "navmenuconfig.h"
#include "ui_navmenuconfig.h"
#include "../state/statemachine.h"

namespace core {

NavMenuConfig::NavMenuConfig(QWidget *parent, state::StateMachine * _stateMachine) :
        NavMenu(parent),
        ui(new Ui::NavMenuConfig),
        stateMachine(_stateMachine) {
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

}
