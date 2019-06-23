#include "receivecoins_w.h"
#include "ui_receivecoins.h"
#include "../state/e_listening.h"
#include "../state/transactions.h"
#include "../control/messagebox.h"
#include <QFileDialog>

namespace wnd {

ReceiveCoins::ReceiveCoins(QWidget *parent,
                           state::Listening * _listeningState,
                           state::Transactions * _transactionsState) :
    QWidget(parent),
    ui(new Ui::ReceiveCoins),
    listeningState(_listeningState),
    transactionsState(_transactionsState)
{
    ui->setupUi(this);

    Q_ASSERT(listeningState);
    Q_ASSERT(transactionsState);

    listeningState->setWindowTitle("Recieve MWC");

    // Hide it from UI
    ui->mwcBoxConnectBtn->hide();

    updateBoxState();

    // transactions
    ui->transactionsBox->setTitle("Transactions for account " + transactionsState->getCurrentAccountName());
    initTableHeaders();
    updateTransactionTable();

    // Start listening
/*    QPair<bool, QString> status = listeningState->getBoxListeningStatus();
    if (!status.first) {
        // start listening
        QPair<bool, QString> res = listeningState->startBoxListeningStatus();
        if (!res.first)
            control::MessageBox::message(this,"Listening error", "Unable to process sart listening request. Error: " + res.second);
    }*/
    updateBoxState();

}

ReceiveCoins::~ReceiveCoins()
{
    saveTableHeaders();
    delete ui;
}

/*void ReceiveCoins::on_mwcBoxConnectBtn_clicked()
{
    QPair<bool, QString> status = listeningState->getBoxListeningStatus();
    if (status.first) {
        // stop listening
        QPair<bool, QString> res = listeningState->stopBoxListeningStatus();
        if (!res.first)
            control::MessageBox::message(this,"Listening error", "Unable to process stop listening request. Error: " + res.second);
    }
    else {
        // start listening
        QPair<bool, QString> res = listeningState->startBoxListeningStatus();
        if (!res.first)
            control::MessageBox::message(this,"Listening error", "Unable to process satrt listening request. Error: " + res.second);
    }

    updateBoxState();
}*/

void ReceiveCoins::on_processResponceFileButton_clicked()
{
    /*QString fileName = QFileDialog::getOpenFileName(this, tr("Open your response file"),
                                                          "",
                                                            tr("MWC response (*.response)"));

    if (fileName.length()==0)
        return;

    // Update path
    QFileInfo flInfo(fileName);


    QPair<bool, QString> sendRes = fileTransactionsState->processResponseFile(fileName);
    if (sendRes.first) {
        control::MessageBox::message(this, "Success",
                 "You sucessfully process and publish transaction from " + fileName );
        return;
    }
    else {
        control::MessageBox::message(this, "Error",
                 "Transaction processing was failed.\nError: " + sendRes.second );
        return;
    }*/
}

void ReceiveCoins::updateBoxState() {
/*    QPair<bool, QString> status = listeningState->getBoxListeningStatus();
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
    }*/
}


void ReceiveCoins::initTableHeaders() {

    // Disabling to show the grid

    // Creatign columns
    QVector<int> widths = transactionsState->getColumnsWidhts();
    if ( widths.size() != 9 ) {
        widths = QVector<int>{10,70,150,200,130,50,130,45,40};
    }
    Q_ASSERT( widths.size() == 9 );

    ui->transactionTable->setColumnWidths(widths);
}

void ReceiveCoins::saveTableHeaders() {
    transactionsState->updateColumnsWidhts( ui->transactionTable->getColumnWidths() );
}


void ReceiveCoins::updateTransactionTable() {

/*    QVector<wallet::WalletTransaction> transactions = transactionsState->getTransactions();
    int rowNum = transactions.size();

    ListWithColumns * tt = ui->transactionTable;
    tt->clearData();

    for ( int i=0; i<rowNum; i++ ) {
        auto & trans = transactions[i];

        tt->appendRow(QVector<QString>{QString::number( trans.txIdx ),
                                     trans.getTypeAsStr(),
                                     trans.txid,
                                     trans.address,
                                     trans.creationTime,
                                     trans.confirmed ? "Yes":"No",
                                     trans.confirmationTime,
                                     util::nano2one(trans.coinNano),
                                     trans.proof ? "Yes":"No"
                        });
    }*/
}

void ReceiveCoins::on_refreshButton_clicked()
{
    updateTransactionTable();
}


}


