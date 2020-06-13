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

#include "e_transactions_w.h"
#include "ui_e_transactions.h"
#include <QFileDialog>
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include <QDebug>
#include "../dialogs_desktop/e_showproofdlg.h"
#include "../dialogs_desktop/e_showtransactiondlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wnd/e_transactions_b.h"
#include "../core/global.h"

// It is exception for Mobile, CSV export not likely needed into mobile wallet
#include "../util/Files.h"

namespace wnd {

Transactions::Transactions(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::Transactions)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    wallet = new bridge::Wallet(this);
    transaction = new bridge::Transactions(this);

    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Transactions::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnTransactions,
                      this, &Transactions::onSgnTransactions, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnCancelTransacton,
                      this, &Transactions::onSgnCancelTransacton, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnTransactionById,
                      this, &Transactions::onSgnTransactionById, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnExportProofResult,
                      this, &Transactions::onSgnExportProofResult, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnVerifyProofResult,
                      this, &Transactions::onSgnVerifyProofResult, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnNodeStatus,
                      this, &Transactions::onSgnNodeStatus, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnNewNotificationMessage,
                      this, &Transactions::onSgnNewNotificationMessage, Qt::QueuedConnection);

    ui->transactionTable->setHightlightColors(QColor(255,255,255,51), QColor(255,255,255,153) ); // Alpha: 0.2  - 0.6
    // Alpha delta for row stripe coloring. Range 0-255
    ui->transactionTable->setStripeAlfaDelta( 5 ); // very small number

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    initTableHeaders();

    onSgnWalletBalanceUpdated();
    requestTransactions();

    updateData();

}

Transactions::~Transactions()
{
    saveTableHeaders();
    delete ui;
}

void Transactions::initTableHeaders() {

    // Disabling to show the grid
    // Creatign columns
    QVector<int> widths = config->getColumnsWidhts("TransTblColWidth");
    if ( widths.size() != 8 ) {
        widths = QVector<int>{30,90,100,200,160,90,70,90};
    }
    Q_ASSERT( widths.size() == 8 );

    ui->transactionTable->setColumnWidths( widths );
}

void Transactions::saveTableHeaders() {
    config->updateColumnsWidhts("TransTblColWidth", ui->transactionTable->getColumnWidths());
}

int Transactions::calcPageSize() const {
    QSize sz1 = ui->transactionTable->size();
    QSize sz2 = ui->progressFrame->size();

    return ListWithColumns::getNumberOfVisibleRows( std::max(sz1.height(), sz2.height()) );
}

void Transactions::on_prevBtn_clicked()
{
    if (currentPagePosition > 0) {
        int pageSize = calcPageSize();
        currentPagePosition = std::max( 0, currentPagePosition-pageSize );
        updateData();
    }
}

void Transactions::on_nextBtn_clicked()
{
    if (currentPagePosition + shownTrans.size() < allTrans.size()) {
        int pageSize = calcPageSize();
        currentPagePosition = std::min( allTrans.size()-pageSize, currentPagePosition+pageSize );
        updateData();
    }
}

void Transactions::updateData() {
    ui->nextBtn->setEnabled(false);
    ui->prevBtn->setEnabled(false);
    shownTrans.clear();
    ui->transactionTable->clearData();
    int pageSize = calcPageSize();

    updateButtons();

    if (currentPagePosition <0 || allTrans.size()<=0 || pageSize<=0) {
        ui->pageLabel->setText("");
        return;
    }
    else {
        int total = allTrans.size();
        // Updating page status
        if (total <= 1) {
            ui->pageLabel->setText( QString::number(total) +
                                    " of " + QString::number( total) );
        }
        else {
            ui->pageLabel->setText( QString::number(currentPagePosition+1) + "-" + QString::number( std::min(currentPagePosition+pageSize-1+1, total) ) +
                                " of " + QString::number( total) );
        }
        ui->nextBtn->setEnabled(currentPagePosition < total-pageSize);
        ui->prevBtn->setEnabled(currentPagePosition>0);

        // Updating tx table data
        shownTrans.clear();
        for (int i=currentPagePosition; i<allTrans.size() && pageSize>0; i++, pageSize-- ) {
            shownTrans.push_back(allTrans[i]);
        }

        QDateTime current = QDateTime::currentDateTime();

        ui->transactionTable->clearData();

        int expectedConfirmNumber = config->getInputConfirmationNumber();

        for ( int idx = shownTrans.size()-1; idx>=0; idx--) {
            const wallet::WalletTransaction trans = shownTrans[idx];

            double selection = 0.0;

            if ( trans.canBeCancelled() ) {
                int64_t age = trans.calculateTransactionAge(current);
                // 1 hours is a 1.0
                selection = age > 60 * 60 ?
                            1.0 : (double(age) / double(60 * 60));
            }

            QString transConfirmedStr = trans.confirmed ? "YES" : "NO";
            // if the node is online and in sync, display the number of confirmations instead
            // nodeHeight will be 0 if the node is offline or out of sync
            if (nodeHeight > 0 && trans.height > 0) {
                int needConfirms = trans.isCoinbase() ? mwc::COIN_BASE_CONFIRM_NUMBER : expectedConfirmNumber;
                // confirmations are 1 more than the difference between the node and transaction heights
                int64_t confirmations = nodeHeight - trans.height + 1;
                transConfirmedStr = QString::number(confirmations);
                if (needConfirms >= confirmations) {
                    transConfirmedStr += "/" + QString::number(needConfirms);
                }
            }

            ui->transactionTable->appendRow( QVector<QString>{
                    QString::number(  trans.txIdx+1 ),
                    trans.getTypeAsStr(),
                    trans.txid,
                    trans.address,
                    trans.creationTime,
                    util::nano2one(trans.coinNano),
                    transConfirmedStr,
                    trans.height<=0 ? "" : QString::number(trans.height)
            }, selection );
        }

    }
}

void Transactions::onSgnTransactions( QString acc, QString height, QVector<QString> transactions) {
    Q_UNUSED(height)

    if (acc != ui->accountComboBox->currentData().toString() )
        return;

    ui->progressFrame->hide();
    ui->transactionTable->show();

    account = acc;
    shownTrans.clear();
    allTrans.clear();

    for (QString & t : transactions ) {
        allTrans.push_back( wallet::WalletTransaction::fromJson(t) );
    }

    int pageSize = calcPageSize();
    currentPagePosition = std::max(0, allTrans.size() - pageSize);

    updateData();
}

void Transactions::onSgnExportProofResult(bool success, QString fn, QString msg ) {
    util::TimeoutLockObject to( "Transactions" );
    if (success) {
        dlg::ProofInfo proof;
        if (proof.parseProofText(msg)) {
            dlg::ShowProofDlg dlg(this, fn, proof );
            dlg.exec();
        }
        else {
            control::MessageBox::messageText(this, "Failure", "Internal error. Unable to decode the results of the proof located at  " + fn + "\n\n" + msg );
        }
    }
    else {
        control::MessageBox::messageText(this, "Failure", msg );
    }

}

void Transactions::onSgnVerifyProofResult(bool success, QString fn, QString msg ) {
    util::TimeoutLockObject to( "Transactions" );
    if (success) {
        dlg::ProofInfo proof;
        if (proof.parseProofText(msg)) {
            dlg::ShowProofDlg dlg(this, fn, proof );
            dlg.exec();
        }
        else {
            control::MessageBox::messageText(this, "Failure", "Internal error. Unable to decode the results of the proof from the file " + fn + "\n\n" + msg );
        }
    }
    else {
        control::MessageBox::messageText(this, "Failure", msg );
    }
}


void Transactions::requestTransactions() {
    allTrans.clear();
    shownTrans.clear();
    nodeHeight = -1;
    currentPagePosition = 0;

    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;

    ui->progressFrame->show();
    ui->transactionTable->hide();
    ui->transactionTable->clearData();

    // !!! Note, order is important even it is async. We want node status be processed first..
    wallet->requestNodeStatus(); // Need to know th height.
    wallet->requestTransactions(account, true);
    updateData();
}

// return null if nothing was selected
wallet::WalletTransaction * Transactions::getSelectedTransaction() {
    int row = ui->transactionTable->getSelectedRow();
    if (row<0 || row>=shownTrans.size())
        return nullptr;

    return &shownTrans[ shownTrans.size()-1-row ];
}

void Transactions::updateButtons() {
    wallet::WalletTransaction * selected = getSelectedTransaction();

    ui->generateProofButton->setEnabled( selected!=nullptr && selected->proof );
    ui->deleteButton->setEnabled( selected!=nullptr && selected->canBeCancelled() );
}

void Transactions::on_refreshButton_clicked()
{
    requestTransactions();
}

void Transactions::on_validateProofButton_clicked()
{
    util::TimeoutLockObject to( "Transactions" );

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open proof file"),
                                 config->getPathFor("Transactions"),
                                 tr("transaction proof (*.proof);;All files (*.*)"));

    if (fileName.length()==0)
        return;
    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    // Update path
    QFileInfo flInfo(fileName);
    config->updatePathFor( "Transactions", flInfo.path());

    wallet->verifyTransactionProof(fileName);
}

void Transactions::on_generateProofButton_clicked()
{
    util::TimeoutLockObject to( "Transactions" );

    wallet::WalletTransaction * selected = getSelectedTransaction();

    if (! ( selected!=nullptr && selected->proof ) ) {
        control::MessageBox::messageText(this, "Need info",
                              "Please select qualify transaction to generate a proof.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create transaction proof file"),
                            config->getPathFor("Transactions"),
                            tr("transaction proof (*.proof)"));

    if (fileName.length()==0)
        return;
    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        return;
    }

    if (!fileName.endsWith(".proof"))
        fileName += ".proof";

    // Update path
    QFileInfo flInfo(fileName);
    config->updatePathFor("Transactions", flInfo.path());

    wallet->generateTransactionProof( QString::number(selected->txIdx), fileName );
}

void Transactions::on_exportButton_clicked()
{
    util::TimeoutLockObject to( "Transactions" );

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Transactions"),
                                                    config->getPathFor("TxExportCsv"),
                                                    tr("Export Options (*.csv)"));

    if (fileName.length()==0)
        return;

    QFileInfo flInfo(fileName);
    config->updatePathFor("TxExportCsv", flInfo.path());

    // check to ensure a file extension was specified as getSaveFileName
    // allows files without an extension to be specified
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        // if no file extension is specified, default to exporting CSV files
        fileName += ".csv";
    }

    if (allTrans.isEmpty()) {
        control::MessageBox::messageText(this, "Export Error", "You don't have any transactions to export.");
        return;
    }

    // qt-wallet displays the transactions last to first
    // however when exporting the transactions, we want to export first to last
    QStringList exportRecords;

    // retrieve the first transaction and get the CSV headers
    wallet::WalletTransaction trans = allTrans[0];
    QString csvHeaders = trans.getCSVHeaders();
    exportRecords << csvHeaders;
    QString csvValues = trans.toStringCSV();
    exportRecords << csvValues;

    // now retrieve the remaining transactions and add them to our list
    for ( int idx=1; idx < allTrans.size(); idx++) {
        wallet::WalletTransaction trans = allTrans[idx];
        QString csvValues = trans.toStringCSV();
        exportRecords << csvValues;
    }
    // Note: Mobile doesn't expect to export anything. That is why we are breaking bridge rule here and usung util::writeTextFile directly
    // warning: When using a debug build, avoid testing with an existing file which has
    //          read-only permissions. writeTextFile will hit a Q_ASSERT causing qt-wallet
    //          to crash.
    bool exportOk = util::writeTextFile(fileName, exportRecords);
    if (!exportOk)
    {
        control::MessageBox::messageText(this, "Error", "Export unable to write to file: " + fileName);
    }
    else
    {
        // some users may have a large number of transactions which take time to write to the file
        // so indicate when the file write has completed
        control::MessageBox::messageText(this, "Success", "Exported transactions to file: " + fileName);
    }
    return;
}

void Transactions::on_transactionTable_itemSelectionChanged()
{
    updateButtons();
}

void Transactions::on_transactionTable_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    wallet::WalletTransaction * selected = getSelectedTransaction();
    QString account = ui->accountComboBox->currentData().toString();

    if (account.isEmpty() || selected==nullptr)
        return;

    // respond will come at updateTransactionById
    wallet->requestTransactionById(account, QString::number(selected->txIdx) );

    ui->progressFrame->show();
    ui->transactionTable->hide();
}

void Transactions::onSgnNodeStatus( bool online, QString errMsg, int _nodeHeight, int peerHeight, QString totalDifficulty, int connections ) {
    Q_UNUSED(errMsg);
    Q_UNUSED(peerHeight);
    Q_UNUSED(totalDifficulty);
    Q_UNUSED(connections);

    if (online)
        nodeHeight = _nodeHeight;
}

void Transactions::onSgnTransactionById(bool success, QString account, QString height, QString transactionJson,
                          QVector<QString> outputsJson, QVector<QString> messages) {

    Q_UNUSED(account)
    Q_UNUSED(height)

    ui->progressFrame->hide();
    ui->transactionTable->show();

    util::TimeoutLockObject to( "Transactions" );

    if (!success) {
        control::MessageBox::messageText(this, "Transaction details",
                                         "Internal error. Transaction details are not found.");
        return;
    }

    wallet::WalletTransaction transaction = wallet::WalletTransaction::fromJson(transactionJson);

    QVector<wallet::WalletOutput> outputs;
    for (auto & json : outputsJson)
        outputs.push_back( wallet::WalletOutput::fromJson(json));


    QString txnNote = config->getTxNote(transaction.txid);
    dlg::ShowTransactionDlg showTransDlg(this, account,  transaction, outputs, messages, txnNote);
    connect(&showTransDlg, &dlg::ShowTransactionDlg::saveTransactionNote, this, &Transactions::saveTransactionNote);
    showTransDlg.exec();
}

void Transactions::on_accountComboBox_activated(int index)
{
    Q_UNUSED(index);
    QString account = ui->accountComboBox->currentData().toString();
    if (!account.isEmpty())
        wallet->switchAccount(account);
    requestTransactions();
}

void Transactions::on_deleteButton_clicked()
{
    util::TimeoutLockObject to( "Transactions" );
    wallet::WalletTransaction * selected = getSelectedTransaction();

    if (! ( selected!=nullptr && !selected->confirmed ) ) {
        control::MessageBox::messageText(this, "Need info",
                              "Please select qualify transaction to cancel.");
        return;
    }

    wallet::WalletTransaction tx2del = *selected;

    if ( control::MessageBox::questionText(this, "Transaction cancellation",
            "Are you sure you want to cancel transaction #" + QString::number(tx2del.txIdx+1) +
                               ", TXID " + tx2del.txid, "No", "Yes", true, false) == core::WndManager::RETURN_CODE::BTN2 ) {

        wallet->requestCancelTransacton( account, QString::number(tx2del.txIdx));
    }
}

void Transactions::onSgnCancelTransacton(bool success, QString account, QString trIdxStr, QString errMessage) {
    Q_UNUSED(account)
    Q_UNUSED(errMessage)

    ui->progressFrame->hide();
    ui->transactionTable->show();

    int64_t trIdx = trIdxStr.toLongLong();

    util::TimeoutLockObject to("Transactions");
    if (success) {
        requestTransactions();
        control::MessageBox::messageText(this, "Transaction was cancelled", "Transaction number " + QString::number(trIdx+1) + " was successfully cancelled");
    }
    else {
        control::MessageBox::messageText(this, "Failed to cancel transaction", "Cancel request for transaction number " + QString::number(trIdx+1) + " has failed.\n\n");
    }
}

void Transactions::onSgnWalletBalanceUpdated() {
    // Pairs: [ account, full_name ]
    QVector<QString> accounts = wallet->getWalletBalance(true,false,true);
    QString selectedAccount = wallet->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx=0;
    for ( int i=1; i<accounts.size(); i+=2) {
        if (accounts[i-1] == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem( accounts[i], QVariant(accounts[i-1]) );
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

void Transactions::onSgnNewNotificationMessage(int level, QString message) // level: notify::MESSAGE_LEVEL values
{
    Q_UNUSED(level)
    if (message.contains("Changing transaction")) {
        on_refreshButton_clicked();
    }
}

void Transactions::saveTransactionNote(QString txUuid, QString note) {
    if (note.isEmpty()) {
        config->deleteTxNote(txUuid);
    }
    else {
        // add new note or update existing note for this commitment
        config->updateTxNote(txUuid, note);
    }
}

}


