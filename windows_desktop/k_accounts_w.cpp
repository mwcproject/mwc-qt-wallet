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
#include "../control_desktop/messagebox.h"
#include "../control_desktop/inputdialog.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/k_accounts_b.h"
#include "../bridge/util_b.h"

namespace wnd {

Accounts::Accounts(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Accounts)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    hodlStatus = new bridge::HodlStatus(this);
    wallet = new bridge::Wallet(this);
    accState = new bridge::Accounts(this);
    util = new bridge::Util(this);

    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Accounts::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnAccountCreated,
                      this, &Accounts::onSgnAccountCreated, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnAccountRenamed,
                      this, &Accounts::onSgnAccountRenamed, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->accountList->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->accountList->setStripeAlfaDelta( 5 ); // very small number

    ui->accountList->setFocus();

    if (config->isColdWallet()) {
        ui->transferButton->hide();
    }

    inHodl = hodlStatus->isInHodl("");

    initTableHeaders();

    onSgnWalletBalanceUpdated();

    updateButtons();
}


Accounts::~Accounts()
{
    saveTableHeaders();
    delete ui;
}

void Accounts::updateButtons() {
    int idx = ui->accountList->getSelectedRow();
    ui->renameButton->setEnabled( idx>0 ); // 0 is default and default we can't rename
    ui->deleteButton->setEnabled(idx>0 && idx<accounts.size() &&  accState->canDeleteAccount(accounts[idx]) ); // default cant delete as well
}

void Accounts::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = config->getColumnsWidhts("AccountTblColWidth");
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

    config->updateColumnsWidhts("AccountTblColWidth", width);
}


void Accounts::startWaiting() {
    ui->accountList->clearData();
    ui->progress->show();
}


void Accounts::onSgnWalletBalanceUpdated()
{
    ui->progress->hide();

    accounts = wallet->getWalletBalance(true,false,false);

    QVector<QString> data2show = accState->getAccountsBalancesToShow(inHodl);

    // update the list with accounts
    ui->accountList->clearData();
    int rowSz = inHodl ? 6: 5;

    for (int r=0; r<data2show.size()-rowSz+1; r+=rowSz) {
        QVector<QString> data;
        for (int w=0; w<rowSz; w++) {
            data.push_back( data2show[r+w] );
        }
        ui->accountList->appendRow( data );
    }

    ui->transferButton->setEnabled( accounts.size()>1 );
}

void Accounts::onSgnAccountRenamed(bool success, QString errorMessage) {
    util::TimeoutLockObject to( "Accounts" );

    wallet->requestWalletBalanceUpdate();
    if (!success) {
        control::MessageBox::messageText(this, "Account rename failure", "Your account wasn't renamed.\n" + errorMessage);
    }
}

void Accounts::onSgnAccountCreated( QString newAccountName) {
    Q_UNUSED(newAccountName)
    onSgnWalletBalanceUpdated();
}


void Accounts::on_refreshButton_clicked()
{
    wallet->requestWalletBalanceUpdate();
    startWaiting();
}


void Accounts::on_transferButton_clicked()
{
    accState->doTransferFunds();
}

void Accounts::on_addButton_clicked()
{
    util::TimeoutLockObject to( "Accounts" );
    bool ok = false;
    QString accountName = control::InputDialog::getText(this, tr("Add account"),
                                                tr("Please specify the name of a new account in your wallet"), "account name",
                                                "", 32,  &ok);
    if (!ok || accountName.isEmpty())
        return;

    QString err = accState->validateNewAccountName(accountName);
    if (!err.isEmpty()) {
        control::MessageBox::messageText(this, "Wrong account name",  err );
        return;
    }

    startWaiting();
    wallet->createAccount(accountName);
}

void Accounts::on_deleteButton_clicked()
{
    using namespace control;

    util::TimeoutLockObject to("Accounts" );

    int idx = ui->accountList->getSelectedRow();
    if (idx>=0 && idx<accounts.size() && accState->canDeleteAccount(accounts[idx])) {
        core::WndManager::RETURN_CODE res = MessageBox::questionText( this, "Delete account", "Are you sure that you want to delete this account?",
                "Yes", "No",
                "Continue and delete this account",
                "Cancel, I want to keep this account",
                false, true );
        if (res == core::WndManager::RETURN_CODE::BTN1 ) {
            accState->deleteAccount( accounts[idx] );
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

    util::TimeoutLockObject to( "Accounts" );

    bool ok = false;
    QString name = control::InputDialog::getText(this, "Rename mwc account",
                                        QString("Input a new name for your account '") + accounts[idx] + "'", "",
                                        accounts[idx], 32, &ok);
    if (!ok || name.isEmpty() || name==accounts[idx] )
        return;

    QString err = accState->validateNewAccountName(name);
    if (!err.isEmpty()) {
        control::MessageBox::messageText(this, "Wrong account name",  err );
        return;
    }

    wallet->renameAccount( accounts[idx], name );
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

