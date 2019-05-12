#include "transactions_w.h"
#include "ui_transactions.h"
#include "../state/transactions.h"
#include "../util/stringutils.h"
#include <QFileDialog>
#include <QMessageBox>
#include "showproofdialog.h"

namespace wnd {

Transactions::Transactions(QWidget *parent, state::Transactions * _state) :
    QWidget(parent),
    ui(new Ui::Transactions),
    state(_state)
{
    ui->setupUi(this);

    initTableHeaders();

    updateTransactionTable();
}

Transactions::~Transactions()
{
    saveTableHeaders();
    delete ui;
}

void Transactions::initTableHeaders() {

    // Disabling to show the grid
    ui->transactionTable->setShowGrid(false);

    // Creatign columns
    QVector<int> widths = state->getColumnsWidhts();
    if ( widths.size() != 9 ) {
        widths = QVector<int>{10,30,50,100,50,10,50,30,30};
    }
    Q_ASSERT( widths.size() == 9 );
    Q_ASSERT( ui->transactionTable->columnCount() == widths.size() );

    for (int u=0;u<widths.size();u++)
        ui->transactionTable->setColumnWidth(u,widths[u]);
}

void Transactions::saveTableHeaders() {
    int cc = ui->transactionTable->columnCount();
    QVector<int> widths( cc );
    for (int t=0;t<cc;t++)
        widths[t] = ui->transactionTable->columnWidth(t);

    state->updateColumnsWidhts(widths);
}


void Transactions::updateTransactionTable() {

    transactions = state->getTransactions();
    int rowNum = transactions.size();

    QTableWidget * tt = ui->transactionTable;

    tt->clearContents();
    tt->setRowCount(rowNum);

    Q_ASSERT( ui->transactionTable->columnCount() == 9 );
    tt->setSortingEnabled(false);

    for ( int i=0; i<rowNum; i++ ) {
        auto & trans = transactions[i];

        tt->setItem( i, 0, new QTableWidgetItem(QString::number(i+1) ));
        tt->setItem( i, 1, new QTableWidgetItem(trans.getTypeAsStr()) );
        tt->setItem( i, 2, new QTableWidgetItem(trans.txid ) );
        tt->setItem( i, 3, new QTableWidgetItem(trans.address ) );
        tt->setItem( i, 4, new QTableWidgetItem(trans.creationTime ) );
        tt->setItem( i, 5, new QTableWidgetItem(trans.confirmed ? "Yes":"No" ) );
        tt->setItem( i, 6, new QTableWidgetItem(trans.confirmationTime ) );
        tt->setItem( i, 7, new QTableWidgetItem(util::nano2one(trans.coinNano) ) );
        tt->setItem( i, 8, new QTableWidgetItem(trans.proof ? "Yes":"No" ) );
    }

    updateProofState();

}

// return null if nothing was selected
wallet::WalletTransaction * Transactions::getSelectedTransaction() {
    QList<QTableWidgetItem *> selItms = ui->transactionTable->selectedItems();
    if (selItms.size()==0)
        return nullptr;

    int row = selItms.front()->row();
    if (row<0 || row>=transactions.size())
        return nullptr;

    return &transactions[row];
}

void Transactions::updateProofState() {
    wallet::WalletTransaction * selected = Transactions::getSelectedTransaction();

    ui->generateProofButton->setEnabled( selected!=nullptr && selected->proof );
}


void Transactions::on_refreshButton_clicked()
{
    updateTransactionTable();
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

    wallet::WalletProofInfo proofInfo = state->verifyMwcBoxTransactionProof(fileName);
    if (!proofInfo.successed) {
        QMessageBox::critical(this, "MWC Transaction Proof",
                              "Unable to verify this proof of the transaction. Error: " + proofInfo.errorMessage);
        return;
    }

    ShowProofDialog * showProofDlg = new ShowProofDialog(this, fileName, proofInfo);
    showProofDlg->exec();
}

void Transactions::on_generateProofButton_clicked()
{
    wallet::WalletTransaction * selected = Transactions::getSelectedTransaction();

    if (! ( selected!=nullptr && selected->proof ) ) {
        QMessageBox::information(this, "MWC Transaction Proof",
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

    wallet::WalletProofInfo resultProof = state->generateMwcBoxTransactionProof( selected->txIdx, fileName );

    if (!resultProof.successed) {
        QMessageBox::critical(this, "MWC Transaction Proof",
                              "Unable to generate proof for this transaction. Error: " + resultProof.errorMessage);
        return;
    }

    ShowProofDialog * showProofDlg = new ShowProofDialog(this, fileName, resultProof);
    showProofDlg->exec();
}

void Transactions::on_transactionTable_itemSelectionChanged()
{
    updateProofState();
}

}

