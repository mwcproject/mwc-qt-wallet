#include "e_transactions_w.h"
#include "ui_e_transactions.h"
#include "state/e_transactions.h"
#include "../util/stringutils.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include <QMessageBox>

namespace wnd {

Transactions::Transactions(QWidget *parent, state::Transactions * _state) :
    QWidget(parent),
    ui(new Ui::Transactions),
    state(_state)
{
    ui->setupUi(this);

    ui->transactionTable->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->transactionTable->setStripeAlfaDelta( 5 ); // very small number


    ui->progress->initLoader(true);
    ui->progressFrame->hide();

//    QString title = "Transactions for account " + state->getCurrentAccountName();
  //  state->setWindowTitle(title);

    updateWalletBalance();

    initTableHeaders();

    requestTransactions();
}

Transactions::~Transactions()
{
    state->resetWnd();
    saveTableHeaders();
    delete ui;
}

void Transactions::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 9 ) {
        widths = QVector<int>{30,90,150,200,100,50,100,50,50};
    }
    Q_ASSERT( widths.size() == 9 );

    ui->transactionTable->setColumnWidths( widths );
}

void Transactions::saveTableHeaders() {
    state->updateColumnsWidhts(ui->transactionTable->getColumnWidths());
}


void Transactions::setTransactionData(QString account, long height, const QVector<wallet::WalletTransaction> & trans) {

    ui->progressFrame->hide();
    ui->transactionTable->show();

    Q_UNUSED(height);

    // Update active account just to guarantee be in sync. Normally account should be the same
    int curIdx = ui->accountComboBox->currentIndex();

    bool accountOK = false;
    if ( curIdx>=0 && curIdx<accountInfo.size() ) {
        accountOK = (accountInfo[curIdx].accountName == account);
    }

    if (!accountOK) {
        for ( int t=0; t<accountInfo.size(); t++ ) {
            if (accountInfo[t].accountName == account) {
                ui->accountComboBox->setCurrentIndex(t);
                accountOK = true;
                break;
            }
        }
    }

    Q_ASSERT(accountOK);
    if (!accountOK) {
        control::MessageBox::message(this, "Internal Error", "wmc713 transaction output doesn't match previous data. We recommend you restart this app.");
        // not exiting, error is not fatal. Let's upadte transaction data
    }

    transactions = trans;

    QDateTime current = QDateTime::currentDateTime();

    ui->transactionTable->clearData();

    for ( int idx = transactions.size()-1; idx>=0; idx--) {
        const wallet::WalletTransaction trans = transactions[idx];

        double selection = 0.0;

        if ( trans.canBeCancelled() ) {
            long age = trans.calculateTransactionAge(current);
            // 1 hours is a 1.0
            selection = age > 60 * 60 ?
                1.0 : (double(age) / double(60 * 60));
        }

        ui->transactionTable->appendRow( QVector<QString>{
                QString::number(  trans.txIdx ),
                trans.getTypeAsStr(),
                trans.txid,
                trans.address,
                trans.creationTime,
                (trans.confirmed ? "YES":"NO"),
                trans.confirmationTime,
                util::nano2one(trans.coinNano),
                (trans.proof ? "YES":"NO")
        }, selection );
    }

    updateButtons();
}

void Transactions::showExportProofResults(bool success, QString fn, QString msg ) {
    if (success) {
        QMessageBox::information(this, "Success", "Your transaction proof located at " + fn + "\n\n" + msg );
    }
    else {
        control::MessageBox::message(this, "Failure", msg );
    }

}
void Transactions::showVerifyProofResults(bool success, QString fn, QString msg ) {
    if (success) {
        QMessageBox::information(this, "Success", "Proof at " + fn + ":\n\n" + msg );
    }
    else {
        control::MessageBox::message(this, "Failure", msg );
    }
}


void Transactions::requestTransactions() {

    ui->progressFrame->show();
    ui->transactionTable->hide();

    ui->transactionTable->clearData();
    state->requestTransactions();

    updateButtons();
}

// return null if nothing was selected
wallet::WalletTransaction * Transactions::getSelectedTransaction() {
    QList<QTableWidgetItem *> selItms = ui->transactionTable->selectedItems();
    if (selItms.size()==0)
        return nullptr;

    int row = selItms.front()->row();
    if (row<0 || row>=transactions.size())
        return nullptr;

    return &transactions[ transactions.size()-1-row ];
}

void Transactions::updateButtons() {
    wallet::WalletTransaction * selected = Transactions::getSelectedTransaction();

    ui->generateProofButton->setEnabled( selected!=nullptr && selected->proof );
    ui->deleteButton->setEnabled( selected!=nullptr && selected->canBeCancelled() );
}


void Transactions::on_refreshButton_clicked()
{
    requestTransactions();
}

void Transactions::on_validateProofButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open proof file"),
                                 state->getProofFilesPath(),
                                 tr("transaction proof (*.proof)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateProofFilesPath( flInfo.path() );

    state->verifyMwcBoxTransactionProof(fileName);
}

void Transactions::on_generateProofButton_clicked()
{
    wallet::WalletTransaction * selected = Transactions::getSelectedTransaction();

    if (! ( selected!=nullptr && selected->proof ) ) {
        control::MessageBox::message(this, "Need info",
                              "Please select qualify transaction to generate a proof.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create transaction proof file"),
                                                          state->getProofFilesPath(),
                                                          tr("transaction proof (*.proof)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    state->updateProofFilesPath(flInfo.path());

    state->generateMwcBoxTransactionProof( selected->txIdx, fileName );
}

void Transactions::on_transactionTable_itemSelectionChanged()
{
    updateButtons();
}

void Transactions::on_accountComboBox_activated(int index)
{
    if (index>=0 && index<accountInfo.size()) {
        updateAccountInfo(index);
        state->switchCurrentAccount( accountInfo[index] );
        requestTransactions();
    }
}

void Transactions::on_deleteButton_clicked()
{
    wallet::WalletTransaction * selected = Transactions::getSelectedTransaction();

    if (! ( selected!=nullptr && !selected->confirmed ) ) {
        control::MessageBox::message(this, "Need info",
                              "Please select qualify transaction to cancel.");
        return;
    }

    if ( QMessageBox::question(this, "Transaction cancellation", "Are you sure you want to cancet transaction #" + QString::number(selected->txIdx) +
                               ", TXID " + selected->txid)  == QMessageBox::Yes ) {
        state->cancelTransaction(*selected);
    }
}

void Transactions::updateCancelTransacton(bool success, long trIdx, QString errMessage) {
    if (success) {
        requestTransactions();

        control::MessageBox::message(this, "Transaction was cancelled", "Transaction number " + QString::number(trIdx) + " was successfully cancelled");
    }
    else {
        control::MessageBox::message(this, "Failed to cancel transaction", "Cancel request for transaction number " + QString::number(trIdx) + " was failed.\n\n" + errMessage);
    }
}

void Transactions::updateWalletBalance() {
    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for (auto & info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( info.accountName + "    Total: " + util::nano2one( info.total ) + "mwc  " +
                                      "Spendable: " + util::nano2one(info.currentlySpendable) + "  Locked: " + util::nano2one(info.lockedByPrevTransaction) +
                                      "  Awaiting Confirmation: " + util::nano2one( info.awaitingConfirmation ),
                                      QVariant(idx++) );

//                                      + util::nano2one(info.currentlySpendable) + " of " + util::nano2one(info.total) + " mwc", QVariant(idx++) );
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
    updateAccountInfo(selectedAccIdx);
}

void Transactions::updateAccountInfo(int accIdx) {
    if (accIdx<0 || accIdx>=accountInfo.size())
        return;

    ui->accInfoFrame->hide();

    wallet::AccountInfo info = accountInfo[accIdx];

    ui->totalLabel->setText( "Total:            " + util::nano2one( info.total ) + " mwc" );
    ui->awaitingLabel->setText( "Awaiting Confirmation: " + util::nano2one( info.awaitingConfirmation ) + " mwc" );
    ui->lockedLabel->setText( "Locked by prev. transactions: " + util::nano2one( info.lockedByPrevTransaction ) + " mwc" );
    ui->spendableLabel->setText( "Currently Spendable: " + util::nano2one( info.currentlySpendable ) + " mwc" );
}



}



