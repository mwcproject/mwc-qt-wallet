#include "e_transactions_w.h"
#include "ui_e_transactions.h"
#include "state/e_transactions.h"
#include "../util/stringutils.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include <QMessageBox>
#include "../state/timeoutlock.h"

namespace wnd {

Transactions::Transactions(QWidget *parent, state::Transactions * _state) :
    core::NavWnd(parent, _state->getContext() ),
    ui(new Ui::Transactions),
    state(_state)
{
    ui->setupUi(this);

    ui->transactionTable->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->transactionTable->setStripeAlfaDelta( 5 ); // very small number

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    QString accName = updateWalletBalance();

    initTableHeaders();

    requestTransactions(accName);
}

Transactions::~Transactions()
{
    saveTableHeaders();
    state->resetWnd(this);
    delete ui;
}

void Transactions::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 7 ) {
        widths = QVector<int>{30,100,150,200,100,100,100};
    }
    Q_ASSERT( widths.size() == 7 );

    ui->transactionTable->setColumnWidths( widths );
}

void Transactions::saveTableHeaders() {
    state->updateColumnsWidhts(ui->transactionTable->getColumnWidths());
}


void Transactions::setTransactionData(QString account, int64_t height, const QVector<wallet::WalletTransaction> & trans) {

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

    transactions = trans;

    QDateTime current = QDateTime::currentDateTime();

    ui->transactionTable->clearData();

    for ( int idx = transactions.size()-1; idx>=0; idx--) {
        const wallet::WalletTransaction trans = transactions[idx];

        double selection = 0.0;

        if ( trans.canBeCancelled() ) {
            int64_t age = trans.calculateTransactionAge(current);
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
                util::nano2one(trans.coinNano),
                (trans.confirmed ? "YES":"NO")
        }, selection );
    }

    updateButtons();
}

void Transactions::showExportProofResults(bool success, QString fn, QString msg ) {
    state::TimeoutLockObject to( state );
    if (success) {
        QMessageBox::information(this, "Success", "Your transaction proof located at " + fn + "\n\n" + msg );
    }
    else {
        control::MessageBox::message(this, "Failure", msg );
    }

}
void Transactions::showVerifyProofResults(bool success, QString fn, QString msg ) {
    state::TimeoutLockObject to( state );
    if (success) {
        QMessageBox::information(this, "Success", "Proof at " + fn + ":\n\n" + msg );
    }
    else {
        control::MessageBox::message(this, "Failure", msg );
    }
}


void Transactions::requestTransactions(QString account) {

    ui->progressFrame->show();
    ui->transactionTable->hide();

    ui->transactionTable->clearData();
    state->requestTransactions(account);

    updateButtons();
}

// return null if nothing was selected
wallet::WalletTransaction * Transactions::getSelectedTransaction() {
    int row = ui->transactionTable->getSelectedRow();
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
    requestTransactions(getSelectedAccount().accountName);
}

void Transactions::on_validateProofButton_clicked()
{
    state::TimeoutLockObject to( state );

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
    state::TimeoutLockObject to( state );

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
        state->switchCurrentAccount( accountInfo[index] );
        requestTransactions( accountInfo[index].accountName );
    }
}

void Transactions::on_deleteButton_clicked()
{
    state::TimeoutLockObject to( state );
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

void Transactions::updateCancelTransacton(bool success, int64_t trIdx, QString errMessage) {
    state::TimeoutLockObject to( state );
    if (success) {
        requestTransactions(getSelectedAccount().accountName);

        control::MessageBox::message(this, "Transaction was cancelled", "Transaction number " + QString::number(trIdx) + " was successfully cancelled");
    }
    else {
        control::MessageBox::message(this, "Failed to cancel transaction", "Cancel request for transaction number " + QString::number(trIdx) + " was failed.\n\n" + errMessage);
    }
}

QString Transactions::updateWalletBalance() {
    accountInfo = state->getWalletBalance();
    QString selectedAccount = state->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for (auto & info : accountInfo) {
        if (info.accountName == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( info.getLongAccountName(), QVariant(idx++) );
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
    return accountInfo[selectedAccIdx].accountName;
}

wallet::AccountInfo Transactions::getSelectedAccount() const {
    int idx = ui->accountComboBox->currentIndex();
    if (idx<0)
        idx = 0;

    return accountInfo[idx];
}

}



