#include "accounts_w.h"
#include "ui_accounts.h"
#include "../state/accounts.h"
#include "../util/stringutils.h"
#include <QInputDialog>
#include "../control/messagebox.h"

namespace wnd {

Accounts::Accounts(QWidget *parent, state::Accounts * _state) :
    QWidget(parent),
    ui(new Ui::Accounts),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Accounts");

    QVector<wallet::AccountInfo> info = state->getWalletInfo();

    ui->accountList->setFocus();

    startWaiting();

    updateUiState();

    stopWaiting();
}


Accounts::~Accounts()
{
    delete ui;
}

void Accounts::updateUiState()
{
    // update the list with accounts
    ui->accountList->clear();
/*    QVector<QString> accounts( state->getAccounts() );
    for (auto & acc : accounts)
        ui->accountList->addItem(acc);

    wallet::WalletInfo info = state->getWalletInfo();

    ui->totalAmount->setText( util::nano2one( info.total ) + " MWC" );
    ui->lockedByPrevTrans->setText( util::nano2one( info.lockedByPrevTransaction ) + " MWC" );
    ui->awaitingConfirmation->setText( util::nano2one( info.awaitingConfirmation ) + " MWC" );
    ui->currentlySpendable->setText( util::nano2one( info.currentlySpendable ) + " MWC" );

    int curAccountIdx = accounts.indexOf(info.accountName);
    ui->accountList->setCurrentRow( curAccountIdx );
    currentAccountName = info.accountName;

    ui->selectButton->setEnabled(false);*/

}

void Accounts::startWaiting() {
 //  ui->waitingPlaceholder->show();
 //   QApplication::processEvents();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void Accounts::stopWaiting() {
 //   ui->waitingPlaceholder->hide();
 //   QApplication::processEvents();
     QApplication::restoreOverrideCursor();
}


void Accounts::on_addAccountButton_clicked()
{
    bool ok;
    QString accountName = QInputDialog::getText(this, tr("Add account"),
                                            tr("Please specify the name of a new account in your wallet. Please note, there is no delete action for accounts."), QLineEdit::Normal,
                                            "", &ok);
    if (!ok || accountName.isEmpty())
        return;

    startWaiting();
    state->createAccount(accountName);
/*    updateUiState();
    stopWaiting();

    if (!newAccResult.first) {
        control::MessageBox::message(this, "Error", "Wallet unable to add a new account.\nError: " + newAccResult.second);
    }*/
}

void Accounts::on_refreshButton_clicked()
{
    startWaiting();

    state->resync();
    updateUiState();

    stopWaiting();
}

void Accounts::on_accountList_itemActivated(QListWidgetItem *item)
{
    if (!item)
        return;


    QString account = item->text();

    if (account == currentAccountName) {
        return;
    }

    selectNewAccount(account);
}

void Accounts::selectNewAccount(QString account) {
    startWaiting();
    // Need to select an account
    state->switchAccount(account);

    updateUiState();
    stopWaiting();

}


QString Accounts::getSelectedAccount() {
    QListWidgetItem * item = ui->accountList->currentItem();
    if (item == nullptr)
        return "";

    return item->text();
}

void Accounts::on_accountList_itemSelectionChanged()
{
    QString newAcc = getSelectedAccount();
    if (newAcc.length()==0)
        return;

    ui->selectButton->setEnabled( newAcc != currentAccountName );
}

void Accounts::on_selectButton_clicked()
{
    QString newAcc = getSelectedAccount();
    if (newAcc.length()==0)
        return;

    selectNewAccount(newAcc);
}


}






