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
#include "state/e_transactions.h"
#include "../util/Files.h"
#include "../util/stringutils.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include <QDebug>
#include "dialogs/e_showproofdlg.h"
#include "dialogs/e_showtransactiondlg.h"
#include "../core/global.h"

namespace wnd {

Transactions::Transactions(QWidget *parent, state::Transactions * _state, const wallet::WalletConfig & _walletConfig) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::Transactions),
    state(_state),
    walletConfig(_walletConfig)
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

    updatePages(-1, -1, -1);

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
    if ( widths.size() != 8 ) {
        widths = QVector<int>{30,90,100,200,160,90,70,90};
    }
    Q_ASSERT( widths.size() == 8 );

    ui->transactionTable->setColumnWidths( widths );
}

void Transactions::saveTableHeaders() {
    state->updateColumnsWidhts(ui->transactionTable->getColumnWidths());
}

int Transactions::calcPageSize() const {
    QSize sz1 = ui->transactionTable->size();
    QSize sz2 = ui->progressFrame->size();

    return ListWithColumns::getNumberOfVisibleRows( std::max(sz1.height(), sz2.height()) );
}


void Transactions::setTransactionCount(QString account, int count) {
    // Init arrays and request the data...
    currentPagePosition = 0; // position at the paging...
    totalTransactions = count;

    if ( account != currentSelectedAccount() ) {
        qDebug() << "Transactions::setOutputCount ignored because of account name";
        return;
    }

    int pageSize = calcPageSize();
    currentPagePosition = std::max(0, totalTransactions-pageSize);
    buttonState = updatePages(currentPagePosition, totalTransactions, pageSize);

    // Requesting the output data
    state->requestTransactions(account, currentPagePosition, pageSize, true); // refresh call, sync need to be enforced
}

void Transactions::on_prevBtn_clicked()
{
    if (currentPagePosition > 0) {
        int pageSize = calcPageSize();
        currentPagePosition = std::max( 0, currentPagePosition-pageSize );

        buttonState = updatePages(currentPagePosition, totalTransactions, pageSize);
        state->requestTransactions(currentSelectedAccount(), currentPagePosition, pageSize, false);
    }
}

void Transactions::on_nextBtn_clicked()
{
    if (currentPagePosition + transactions.size() < totalTransactions ) {
        int pageSize = calcPageSize();
        currentPagePosition = std::min( totalTransactions-pageSize, currentPagePosition+pageSize );

        buttonState = updatePages(currentPagePosition, totalTransactions, pageSize);
        state->requestTransactions(currentSelectedAccount(), currentPagePosition, pageSize, false);
    }
}

QPair<bool,bool> Transactions::updatePages( int currentPos, int total, int pageSize ) {
    ui->nextBtn->setEnabled(false);
    ui->prevBtn->setEnabled(false);
    if (currentPos <0 || total<=0 || pageSize<=0) {
        ui->pageLabel->setText("");
        return QPair<bool,bool>(false,false);
    }
    else {
        if (total <= 1) {
            ui->pageLabel->setText( QString::number(total) +
                                    " of " + QString::number( total) );
        }
        else {
            ui->pageLabel->setText( QString::number(currentPos+1) + "-" + QString::number( std::min(currentPos+pageSize-1+1, total) ) +
                                " of " + QString::number( total) );
        }
        return QPair<bool,bool>(currentPos>0,currentPos < total-pageSize);
    }
}

QString Transactions::currentSelectedAccount() {
    int curIdx = ui->accountComboBox->currentIndex();

    if ( curIdx>=0 && curIdx<accountInfo.size() )
        return accountInfo[curIdx].accountName;

    return "";
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

        QString transConfirmedStr = trans.confirmed ? "YES" : "NO";
        // if the node is online and in sync, display the number of confirmations instead
        // nodeHeight will be 0 if the node is offline or out of sync
        if (nodeHeight > 0 && trans.height > 0) {
            int confirms = confirmNumber;
            if (trans.isCoinbase()) {
                confirms = mwc::COIN_BASE_CONFIRM_NUMBER;
            }
            // confirmations are 1 more than the difference between the node and transaction heights
            int64_t confirmations = nodeHeight - trans.height + 1;
            transConfirmedStr = QString::number(confirmations);
            if (confirms >= confirmations) {
                transConfirmedStr += "/" + QString::number(confirms);
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

    ui->prevBtn->setEnabled( buttonState.first );
    ui->nextBtn->setEnabled( buttonState.second );

    updateButtons();
}

void Transactions::showExportProofResults(bool success, QString fn, QString msg ) {
    state::TimeoutLockObject to( state );
    if (success) {
        dlg::ProofInfo proof;
        if (proof.parseProofText(msg)) {
            dlg::ShowProofDlg dlg(this, fn, proof, walletConfig );
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
void Transactions::showVerifyProofResults(bool success, QString fn, QString msg ) {
    state::TimeoutLockObject to( state );
    if (success) {
        dlg::ProofInfo proof;
        if (proof.parseProofText(msg)) {
            dlg::ShowProofDlg dlg(this, fn, proof, walletConfig );
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


void Transactions::requestTransactions(QString account) {

    ui->progressFrame->show();

    ui->transactionTable->hide();

    ui->transactionTable->clearData();

    updatePages(-1, -1, -1);

    state->requestTransactionCount(account);

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
    wallet::WalletTransaction * selected = getSelectedTransaction();

    ui->generateProofButton->setEnabled( selected!=nullptr && selected->proof );
    ui->deleteButton->setEnabled( selected!=nullptr && selected->canBeCancelled() );
}

void Transactions::triggerRefresh() {
    if ( ui->progressFrame->isHidden() )
        on_refreshButton_clicked();
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
                                 tr("transaction proof (*.proof);;All files (*.*)"));

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

    wallet::WalletTransaction * selected = getSelectedTransaction();

    if (! ( selected!=nullptr && selected->proof ) ) {
        control::MessageBox::messageText(this, "Need info",
                              "Please select qualify transaction to generate a proof.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Create transaction proof file"),
                                                          state->getProofFilesPath(),
                                                          tr("transaction proof (*.proof)"));

    if (fileName.length()==0)
        return;

    if (!fileName.endsWith(".proof"))
        fileName += ".proof";

    // Update path
    QFileInfo flInfo(fileName);
    state->updateProofFilesPath(flInfo.path());

    state->generateMwcBoxTransactionProof( selected->txIdx, fileName );
}

void Transactions::on_exportButton_clicked()
{
    state::TimeoutLockObject to( state );

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Transactions"),
                                                          state->getProofFilesPath(),
                                                          tr("Export Options (*.csv)"));

    if (fileName.length()==0)
        return;

    // check to ensure a file extension was specified as getSaveFileName
    // allows files without an extension to be specified
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive))
    {
        // if no file extension is specified, default to exporting CSV files
        fileName += ".csv";
    }

    // qt-wallet displays the transactions last to first
    // however when exporting the transactions, we want to export first to last
    QStringList exportRecords;

    const QVector<wallet::WalletTransaction>& currentTxs = state->getTransactions();
    // retrieve the first transaction and get the CSV headers
    wallet::WalletTransaction trans = currentTxs[0];
    QString csvHeaders = trans.getCSVHeaders();
    exportRecords << csvHeaders;
    QString csvValues = trans.toStringCSV();
    exportRecords << csvValues;

    // now retrieve the remaining transactions and add them to our list
    for ( int idx=1; idx < currentTxs.size(); idx++) {
        wallet::WalletTransaction trans = currentTxs[idx];
        QString csvValues = trans.toStringCSV();
        exportRecords << csvValues;
    }
    // warning: When using a debug build, avoid testing with an existing file which has
    //          read-only permissions. writeTextFile will hit a Q_ASSERT causing qt-wallet
    //          to crash.
    bool exportOk = util::writeTextFile(fileName, exportRecords);
    if (!exportOk)
    {
        control::MessageBox::messageText(nullptr, "Error", "Export unable to write to file: " + fileName);
    }
    else
    {
        // some users may have a large number of transactions which take time to write to the file
        // so indicate when the file write has completed
        control::MessageBox::messageText(nullptr, "Success", "Exported transactions to file: " + fileName);
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
    state::TimeoutLockObject to( state );
    wallet::WalletTransaction * selected = getSelectedTransaction();

    if (selected==nullptr)
        return;

    // respond will come at updateTransactionById
    state->getTransactionById(getSelectedAccount().accountName, selected->txIdx );

    ui->progressFrame->show();
    ui->transactionTable->hide();
}

void Transactions::updateTransactionById(bool success, QString account, int64_t height,
                               wallet::WalletTransaction transaction,
                               QVector<wallet::WalletOutput> outputs,
                               QVector<QString> messages) {

    Q_UNUSED(account)
    Q_UNUSED(height)

    ui->progressFrame->hide();
    ui->transactionTable->show();

    state::TimeoutLockObject to( state );

    if (!success) {
        control::MessageBox::messageText(this, "Transaction details",
                                         "Internal error. Transaction details are not found.");
        return;
    }

    dlg::ShowTransactionDlg showTransDlg(this, walletConfig, transaction, outputs, messages);
    showTransDlg.exec();
}

void Transactions::setConfirmData(int64_t _nodeHeight, int _confirmNumber) {
    nodeHeight    = _nodeHeight;
    confirmNumber = _confirmNumber;
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
    wallet::WalletTransaction * selected = getSelectedTransaction();

    if (! ( selected!=nullptr && !selected->confirmed ) ) {
        control::MessageBox::messageText(this, "Need info",
                              "Please select qualify transaction to cancel.");
        return;
    }
    if ( control::MessageBox::questionText(this, "Transaction cancellation",
            "Are you sure you want to cancel transaction #" + QString::number(selected->txIdx+1) +
                               ", TXID " + selected->txid, "No", "Yes", true, false) == control::MessageBox::RETURN_CODE::BTN2 ) {
        state->cancelTransaction(*selected);
    }
}

void Transactions::updateCancelTransacton(bool success, int64_t trIdx, QString errMessage) {
    Q_UNUSED(errMessage)

    state::TimeoutLockObject to( state );
    if (success) {
        requestTransactions(getSelectedAccount().accountName);

        control::MessageBox::messageText(this, "Transaction was cancelled", "Transaction number " + QString::number(trIdx+1) + " was successfully cancelled");
    }
    else {
        control::MessageBox::messageText(this, "Failed to cancel transaction", "Cancel request for transaction number " + QString::number(trIdx+1) + " has failed.\n\n");
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


