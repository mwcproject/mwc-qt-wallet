#include "transactions_w.h"
#include "ui_transactions.h"
#include "../state/transactions.h"
#include "../util/stringutils.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include "showproofdialog.h"

namespace wnd {

Transactions::Transactions(QWidget *parent, state::Transactions * _state) :
    QWidget(parent),
    ui(new Ui::Transactions),
    state(_state)
{
    ui->setupUi(this);

    QString title = "Transactions for account " + state->getCurrentAccountName();
    state->setWindowTitle(title);

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


void Transactions::updateTransactionTable() {

    transactions = state->getTransactions();

    ui->transactionTable->clearData();

    for (const wallet::WalletTransaction & trans : transactions) {
        ui->transactionTable->appendRow( QVector<QString>{
                                             QString::number(  trans.txIdx ),
                                             trans.getTypeAsStr(),
                                             trans.txid,
                                             trans.address,
                                             trans.creationTime,
                                             (trans.confirmed ? "Yes":"No"),
                                             trans.confirmationTime,
                                             util::nano2one(trans.coinNano),
                                             (trans.proof ? "Yes":"No")
                                         } );
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
        control::MessageBox::message(this, "Error",
                              "Unable to verify this proof of the transaction.\nError: " + proofInfo.errorMessage);
        return;
    }

    ShowProofDialog showProofDlg(this, fileName, proofInfo);
    showProofDlg.exec();
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

    wallet::WalletProofInfo resultProof = state->generateMwcBoxTransactionProof( selected->txIdx, fileName );

    if (!resultProof.successed) {
        control::MessageBox::message(this, "Error",
                              "Unable to generate proof for this transaction.\nError: " + resultProof.errorMessage);
        return;
    }

    ShowProofDialog showProofDlg(this, fileName, resultProof);
    showProofDlg.exec();
}

void Transactions::on_transactionTable_itemSelectionChanged()
{
    updateProofState();
}

}

