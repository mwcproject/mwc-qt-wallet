#include "c_newwallet_w.h"
#include "ui_c_newwallet.h"
#include "../state/a_initaccount.h"
#include "../control/messagebox.h"
#include "../util/widgetutils.h"
#include "../state/timeoutlock.h"

namespace wnd {

NewWallet::NewWallet(QWidget *parent, state::InitAccount * _state) :
    QWidget(parent),
    ui(new Ui::NewWallet),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Init your wallet");

    ui->radioCreateNew->setChecked(true);
    updateControls();

    ui->radioCreateNew->setFocus();

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

NewWallet::~NewWallet()
{
    delete ui;
}

void NewWallet::updateControls() {
    bool isNewChecked = ui->radioCreateNew->isChecked() || ui->radioHaveSeed->isChecked();
    ui->submitButton->setEnabled(isNewChecked);
}


void NewWallet::on_submitButton_clicked()
{
    state::TimeoutLockObject to( state );

    if ( ui->radioCreateNew->isChecked() )
        state->submitCreateChoice( state::InitAccount::NEW_WALLET_CHOICE::CREATE_NEW);
    else if (ui->radioHaveSeed->isChecked())
        state->submitCreateChoice( state::InitAccount::NEW_WALLET_CHOICE::CREATE_WITH_SEED);
    else {
        control::MessageBox::message(nullptr, "Please select", "Please select how you want to provision a new wallet");
        Q_ASSERT(false);
    }
}

void NewWallet::on_radioHaveSeed_clicked()
{
    updateControls();
}

void NewWallet::on_radioCreateNew_clicked()
{
    updateControls();
}

}
