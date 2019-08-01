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

    // State where to go after login
    context->appContext->setActiveWndState(state::STATE::SHOW_SEED);
    context->wallet->logout(true);
    context->stateMachine->executeFrom( state::STATE::NONE);
    close();
}

void NavMenuAccount::on_contactsButton_clicked()
{
    context->stateMachine->setActionWindow( state::STATE::CONTACTS );
    close();
}

}

