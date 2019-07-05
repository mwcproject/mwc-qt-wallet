#include "k_accounts_w.h"
#include "ui_k_accounts_w.h"
#include "../state/k_accounts.h"
#include "../util/stringutils.h"
#include <QInputDialog>
#include "../control/messagebox.h"
#include "../core/global.h"

namespace wnd {

Accounts::Accounts(QWidget *parent, state::Accounts * _state) :
    core::NavWnd(parent, _state->getStateMachine(), _state->getAppContext()),
    ui(new Ui::Accounts),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->accountList->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->accountList->setStripeAlfaDelta( 5 ); // very small number

    ui->accountList->setFocus();

    initTableHeaders();

    refreshWalletBalance();

    updateButtons();
}


Accounts::~Accounts()
{
    state->wndDeleted();
    saveTableHeaders();
    delete ui;
}

void Accounts::updateButtons() {
    int idx = ui->accountList->getSelectedRow();
    ui->renameButton->setEnabled( idx>0 ); // 0 is default and default we can't rename
    ui->deleteButton->setEnabled(idx>0 && idx<accounts.size() && accounts[idx].canDelete() ); // default cant delete as well
}


void Accounts::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 5 ) {
        widths = QVector<int>{150,150,150,150,150};
    }
    Q_ASSERT( widths.size() == 5 );

    ui->accountList->setColumnWidths( widths );
}

void Accounts::saveTableHeaders() {
    state->updateColumnsWidhts(ui->accountList->getColumnWidths());
}


void Accounts::startWaiting() {
    ui->accountList->clearData();
    ui->progress->show();
}


void Accounts::refreshWalletBalance()
{
    ui->progress->hide();

    accounts = state->getWalletBalance();
    // update the list with accounts
    ui->accountList->clearData();

    for (auto & acc : accounts) {
        QVector<QString> data{ acc.accountName, util::nano2one(acc.total), util::nano2one(acc.awaitingConfirmation),
                               util::nano2one(acc.lockedByPrevTransaction), util::nano2one(acc.currentlySpendable) };

        ui->accountList->appendRow( data, acc.accountName.startsWith("HODL") ? 0.8 : 0.0 );
    }

    ui->transferButton->setEnabled( accounts.size()>1 );
}

void Accounts::onAccountRenamed(bool success, QString errorMessage) {
    refreshWalletBalance();
    if (!success) {
        control::MessageBox::message(this, "Account rename faulure", "Your account wasn't renamed.\n" + errorMessage);
    }
}


void Accounts::on_refreshButton_clicked()
{
    state->updateWalletBalance();

    startWaiting();
}


void Accounts::on_transferButton_clicked()
{
    state->doTransferFunds();
}

void Accounts::on_addButton_clicked()
{
    bool ok = false;
    QString accountName = QInputDialog::getText(this, tr("Add account"),
                                                tr("Please specify the name of a new account in your wallet. Please note, there is no delete action for accounts."), QLineEdit::Normal,
                                                "", &ok);
    if (!ok || accountName.isEmpty())
        return;

    // Check for account name
    {
        for (auto & acc : accounts) {
            if (acc.accountName == accountName) {
                control::MessageBox::message(this, "Account name", "Account with name '" + accountName +
                       "' already exist. Please specify unique account name to create.");
                ok = false;
                break;
            }
        }

    }

    if (!ok)
        return;

    startWaiting();

    state->createAccount(accountName);

}

void Accounts::on_deleteButton_clicked()
{
    using namespace control;

    int idx = ui->accountList->getSelectedRow();
    if (idx>=0 && idx<accounts.size() && accounts[idx].canDelete()) {
        MessageBox::RETURN_CODE res = MessageBox::question( this, "Delete account", "Are you sure that you want to delet this account?", "Yes", "No", false, true );
        if (res == MessageBox::RETURN_CODE::BTN1 ) {
            state->deleteAccount( accounts[idx] );
            startWaiting();
        }
    }

}

void Accounts::on_renameButton_clicked()
{
    renameAccount( ui->accountList->getSelectedRow() );
}

void Accounts::renameAccount(int idx) {
    if (idx<0 || idx>=accounts.size())
        return;

    bool ok = false;
    QString name = QInputDialog::getText(this, "Rename mwc account",
                                        QString("Input a new name for your account '") + accounts[idx].accountName + "'", QLineEdit::Normal,
                                        accounts[idx].accountName, &ok);
    if (!ok || name.isEmpty() || name==accounts[idx].accountName )
        return;

    // check for name collision
    if ( name.startsWith( mwc::DEL_ACCONT_PREFIX ) ) {
        control::MessageBox::message(this, "Wrong account name", "Please specify without prefix '" + mwc::DEL_ACCONT_PREFIX + "'" );
        return;
    }

    // Check unoquiness
    for ( int r=0;r<accounts.size(); r++ ) {
        if (r==idx)
            continue;

        if ( name == accounts[r].accountName ) {
            control::MessageBox::message(this, "Wrong account name", "Account name '" + name + "' allready exist. Please specify unique account name." );
            return;
        }
    }


    state->renameAccount( accounts[idx], name );
    startWaiting();
}

void Accounts::on_accountList_itemSelectionChanged()
{
    updateButtons();
}

void Accounts::on_accountList_itemDoubleClicked(QTableWidgetItem *item)
{
    Q_UNUSED(item);
    renameAccount( ui->accountList->getSelectedRow() );
}

}

