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

#include "k_accounts_w.h"
#include "ui_k_accounts_w.h"
#include "../state/k_accounts.h"
#include "../util/stringutils.h"
#include "../control/messagebox.h"
#include "../control/inputdialog.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../state/timeoutlock.h"
#include "../core/HodlStatus.h"

namespace wnd {

Accounts::Accounts(QWidget *parent, state::Accounts * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::Accounts),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->accountList->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->accountList->setStripeAlfaDelta( 5 ); // very small number

    ui->accountList->setFocus();

    if (config::isColdWallet()) {
        ui->transferButton->hide();
    }

    inHodl = state->getContext()->hodlStatus->isInHodl("");

    initTableHeaders();

    refreshWalletBalance();

    updateButtons();
}


Accounts::~Accounts()
{
    state->wndDeleted(this);
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
        widths = QVector<int>{250,110,90,90,90};
    }
    Q_ASSERT( widths.size() == 5 );

    ui->accountList->setColumnWidths( widths );

    if (inHodl) {
        ui->accountList->setColumnCount(widths.size()+1);
        ui->accountList->setColumnWidth(widths.size(),60);
        QTableWidgetItem * itm = new QTableWidgetItem("HODL") ;
        ui->accountList->setHorizontalHeaderItem( widths.size(), itm );
    }
}

void Accounts::saveTableHeaders() {
    QVector<int>  width = ui->accountList->getColumnWidths();
    if (inHodl)
        width.pop_back();

    state->updateColumnsWidhts(width);
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

    core::HodlStatus * hodlStatus = state->getContext()->hodlStatus;
    const QMap<QString, QVector<wallet::WalletOutput> > & walletOutputs = state->getContext()->wallet->getwalletOutputs();

    for (auto & acc : accounts) {
        QVector<QString> data{ acc.accountName, util::nano2one(acc.currentlySpendable), util::nano2one(acc.awaitingConfirmation),
                               util::nano2one(acc.lockedByPrevTransaction), util::nano2one(acc.total) };

        if ( inHodl ) {
            QVector<wallet::WalletOutput> accountOutputs = walletOutputs.value(acc.accountName);

            int64_t hodlBalancePerClass = 0;

            for (const auto & out : accountOutputs) {
                core::HodlOutputInfo hOut = hodlStatus->getHodlOutput("", out.outputCommitment );
                if ( hOut.isValid() ) {
                    hodlBalancePerClass += out.valueNano;
                }
            }

            data.push_back( util::nano2one(hodlBalancePerClass) );
        }

        ui->accountList->appendRow( data, acc.accountName.startsWith("HODL") ? 0.8 : 0.0 );
    }

    ui->transferButton->setEnabled( accounts.size()>1 );
}

void Accounts::onAccountRenamed(bool success, QString errorMessage) {
    state::TimeoutLockObject to( state );

    refreshWalletBalance();
    if (!success) {
        control::MessageBox::messageText(this, "Account rename faulure", "Your account wasn't renamed.\n" + errorMessage);
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
    state::TimeoutLockObject to( state );
    bool ok = false;
    QString accountName = control::InputDialog::getText(this, tr("Add account"),
                                                tr("Please specify the name of a new account in your wallet"), "account name",
                                                "", 32,  &ok);
    if (!ok || accountName.isEmpty())
        return;

    if (accountName.startsWith("-")) {
        control::MessageBox::messageText(this, "Wrong account name",  "You can't start account name from '-' symbol." );
        return;
    }

    for ( auto & pref : mwc::BANNED_ACCOUT_PREFIXES ) {
        if (accountName.startsWith(pref)) {
            control::MessageBox::messageText(this, "Wrong account name",  "Please specify account name without prefix '" + pref + "'" );
            return;
        }
    }

    QPair <bool, QString> valRes = util::validateMwc713Str( accountName );
    if (!valRes.first) {
        control::MessageBox::messageText(this, "Account name", valRes.second );
        return;
    }

    // Check for account name
    {
        for (auto & acc : accounts) {
            if (acc.accountName == accountName) {
                control::MessageBox::messageText(this, "Account name", "Account with name '" + accountName +
                       "' already exists. Please specify a unique account name to create.");
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

    state::TimeoutLockObject to( state );

    int idx = ui->accountList->getSelectedRow();
    if (idx>=0 && idx<accounts.size() && accounts[idx].canDelete()) {
        MessageBox::RETURN_CODE res = MessageBox::questionText( this, "Delete account", "Are you sure that you want to delete this account?", "Yes", "No", false, true );
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

    state::TimeoutLockObject to( state );

    bool ok = false;
    QString name = control::InputDialog::getText(this, "Rename mwc account",
                                        QString("Input a new name for your account '") + accounts[idx].accountName + "'", "",
                                        accounts[idx].accountName, 32, &ok);
    if (!ok || name.isEmpty() || name==accounts[idx].accountName )
        return;

    QPair <bool, QString> valRes = util::validateMwc713Str( name );
    if (!valRes.first) {
        control::MessageBox::messageText(this, "Account name", valRes.second );
        return;
    }

    // check for name collision
    for ( auto & pref : mwc::BANNED_ACCOUT_PREFIXES ) {
        if (name.startsWith(pref)) {
            control::MessageBox::messageText(this, "Wrong account name",  "Please specify account name without prefix '" + pref + "'" );
            return;
        }
    }

    // Check unoquiness
    for ( int r=0;r<accounts.size(); r++ ) {
        if (r==idx)
            continue;

        if ( name == accounts[r].accountName ) {
            control::MessageBox::messageText(this, "Wrong account name", "Account name '" + name + "' already exist. Please specify unique account name." );
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

