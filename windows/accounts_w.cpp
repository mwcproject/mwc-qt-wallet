#include "accounts_w.h"
#include "ui_accounts.h"
#include "../state/accounts.h"
#include "../util/stringutils.h"
#include <QMovie>
#include <QInputDialog>
#include <QMessageBox>

namespace wnd {

Accounts::Accounts(QWidget *parent, state::Accounts * _state) :
    QWidget(parent),
    ui(new Ui::Accounts),
    state(_state)
{
    ui->setupUi(this);

    wallet::WalletInfo info = state->getWalletInfo();


    QLabel *lbl = ui->waitingPlaceholder;
    QMovie *movie = new QMovie(":/img/img/loader.gif", QByteArray(), this);
    lbl->setMovie(movie);
    lbl->hide();

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
    QVector<QString> accounts( state->getAccounts() );
    for (auto & acc : accounts)
        ui->accountList->addItem(acc);

    wallet::WalletInfo info = state->getWalletInfo();

    ui->totalAmount->setText( util::nano2one( info.total ) );
    ui->lockedByPrevTrans->setText( util::nano2one( info.lockedByPrevTransaction ) );
    ui->awaitingConfirmation->setText( util::nano2one( info.awaitingConfirmation ) );
    ui->currentlySpendable->setText( util::nano2one( info.currentlySpendable ) );

    int curAccountIdx = accounts.indexOf(info.accountName);
    ui->accountList->setCurrentRow( curAccountIdx );
}

void Accounts::startWaiting() {
   ui->waitingPlaceholder->show();
    QApplication::processEvents();
}

void Accounts::stopWaiting() {
    ui->waitingPlaceholder->hide();
    QApplication::processEvents();
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
    QPair<bool, QString> newAccResult = state->addAccount(accountName);
    updateUiState();
    stopWaiting();

    if (!newAccResult.first) {
        QMessageBox::critical(this, "Unable to add an account", "Wallet unable to add a new account. Error: " + newAccResult.second);
    }
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

    startWaiting();

    QString account = item->text();

    wallet::WalletInfo info = state->getWalletInfo();
    if (account==info.accountName) {
        stopWaiting();
        return; // already selected
    }

    // Need to select an account
    state->activateAccount(account);

    updateUiState();
    stopWaiting();
}


}




