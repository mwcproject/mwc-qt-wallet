#include "navmenuaccount.h"
#include "../state/state.h"
#include "ui_navmenuaccount.h"
#include "../state/statemachine.h"


namespace core {

NavMenuAccount::NavMenuAccount(QWidget *parent, state::StateMachine * _stateMachine) :
        NavMenu(parent),
        ui(new Ui::NavMenuAccount),
        stateMachine(_stateMachine)
{
    ui->setupUi(this);
}

NavMenuAccount::~NavMenuAccount() {
    delete ui;
}

void NavMenuAccount::on_accountsButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::ACCOUNTS );
}

void NavMenuAccount::on_seedButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::SHOW_SEED );
}

void NavMenuAccount::on_contactsButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::CONTACTS );
}

}

