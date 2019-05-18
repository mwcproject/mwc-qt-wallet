#include "receivecoins_w.h"
#include "ui_receivecoins.h"
#include "../state/listening.h"
#include "../state/filetransactions.h"
#include "../state/transactions.h"
#include <QMessageBox>
#include <QFileDialog>

namespace wnd {

ReceiveCoins::ReceiveCoins(QWidget *parent,
                           state::Listening * _listeningState,
                           state::FileTransactions * _fileTransactionsState,
                           state::Transactions * _transactionsState) :
    QWidget(parent),
    ui(new Ui::ReceiveCoins),
    listeningState(_listeningState),
    fileTransactionsState(_fileTransactionsState),
    transactionsState(_transactionsState)
{
    ui->setupUi(this);

    Q_ASSERT(listeningState);
    Q_ASSERT(fileTransactionsState);
    Q_ASSERT(transactionsState);

    updateBoxState();

    // transactions
    initTableHeaders();
    updateTransactionTable();

}

ReceiveCoins::~ReceiveCoins()
{
    saveTableHeaders();
    delete ui;
}

void ReceiveCoins::on_mwcBoxConnectBtn_clicked()
{
    QPair<bool, QString> status = listeningState->getBoxListeningStatus();
    if (status.first) {
        // stop listening
        QPair<bool, QString> res = listeningState->stopBoxListeningStatus();
        if (!res.first)
            QMessageBox::critical(this,"Listening error", "Unable to process stop listening request. Error: " + res.second);
    }
    else {
        // start listening
        QPair<bool, QString> res = listeningState->startBoxListeningStatus();
        if (!res.first)
            QMessageBox::critical(this,"Listening error", "Unable to process satrt listening request. Error: " + res.second);
    }

    updateBoxState();
}

void ReceiveCoins::on_processResponceFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open your response file"),
                                                          fileTransactionsState->getFileGenerationPath(),
                                                            tr("MWC response (*.response)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);
    fileTransactionsState->updateFileGenerationPath( flInfo.path() );

    QPair<bool, QString> sendRes = fileTransactionsState->processResponseFile(fileName);
    if (sendRes.first) {
        QMessageBox::information(this, "Process transaction",
                 "You sucessfully process and publish transaction from " + fileName );
        return;
    }
    else {
        QMessageBox::critical(this, "Process transaction",
                 "Transaction processing was failed. Error: " + sendRes.second );
        return;
    }
}

void ReceiveCoins::updateBoxState() {
    QPair<bool, QString> status = listeningState->getBoxListeningStatus();
    if (status.first) {
        ui->mwcBoxStatus->setText( "Online" +  (status.second.length()==0 ? "" : (", "+ status.second)));
        ui->mwcBoxConnectBtn->setText( "Stop" );

        QPair<QString,int> boxAddress = listeningState->getBoxAddress();

        ui->mwcBoxAddress->setText( boxAddress.first );
    }
    else {
        ui->mwcBoxConnectBtn->setText( "Start" );
        ui->mwcBoxStatus->setText( "Offline" );
        ui->mwcBoxAddress->setText("N/A");
    }
}


void ReceiveCoins::initTableHeaders() {

    // Disabling to show the grid
    ui->transactionTable->setShowGrid(false);

    // Creatign columns
    QVector<int> widths = transactionsState->getColumnsWidhts();
    if ( widths.size() != 9 ) {
        widths = QVector<int>{10,30,50,100,50,10,50,30,30};
    }
    Q_ASSERT( widths.size() == 9 );
    Q_ASSERT( ui->transactionTable->columnCount() == widths.size() );

    for (int u=0;u<widths.size();u++)
        ui->transactionTable->setColumnWidth(u,widths[u]);
}

void ReceiveCoins::saveTableHeaders() {
    int cc = ui->transactionTable->columnCount();
    QVector<int> widths( cc );
    for (int t=0;t<cc;t++)
        widths[t] = ui->transactionTable->columnWidth(t);

    transactionsState->updateColumnsWidhts(widths);
}


void ReceiveCoins::updateTransactionTable() {

    QVector<wallet::WalletTransaction> transactions = transactionsState->getTransactions();
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
}

void ReceiveCoins::on_refreshButton_clicked()
{
    updateTransactionTable();
}


}


