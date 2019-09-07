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
#include "../util/stringutils.h"
#include <QFileDialog>
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include <QDebug>
#include "../dialogs/showproofdlg.h"
#include "../dialogs/showtransactiondlg.h"

namespace wnd {

Transactions::Transactions(QWidget *parent, state::Transactions * _state) :
    core::NavWnd(parent, _state->getContext()),
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
    if ( widths.size() != 7 ) {
        widths = QVector<int>{30,90,100,200,160,90,70};
    }
    Q_ASSERT( widths.size() == 7 );

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
    state->requestTransactions(account, currentPagePosition, pageSize);
}

void Transactions::on_prevBtn_clicked()
{
    if (currentPagePosition > 0) {
        int pageSize = calcPageSize();
        currentPagePosition = std::max( 0, currentPagePosition-pageSize );

        buttonState = updatePages(currentPagePosition, totalTransactions, pageSize);
        state->requestTransactions(currentSelectedAccount(), currentPagePosition, pageSize);
    }
}

void Transactions::on_nextBtn_clicked()
{
    if (currentPagePosition + transactions.size() < totalTransactions ) {
        int pageSize = calcPageSize();
        currentPagePosition = std::min( totalTransactions-pageSize, currentPagePosition+pageSize );

        buttonState = updatePages(currentPagePosition, totalTransactions, pageSize);
        state->requestTransactions(currentSelectedAccount(), currentPagePosition, pageSize);
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

        ui->transactionTable->appendRow( QVector<QString>{
                QString::number(  trans.txIdx+1 ),
                trans.getTypeAsStr(),
                trans.txid,
                trans.address,
                trans.creationTime,
                util::nano2one(trans.coinNano),
                (trans.confirmed ? "YES":"NO")
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
            dlg::ShowProofDlg dlg(this, fn, proof );
            dlg.exec();
        }
        else {
            control::MessageBox::message(this, "Failure", "Internal error. Unable to decode the results of the proof located at  " + fn + "\n\n" + msg );
        }
    }
    else {
        control::MessageBox::message(this, "Failure", msg );
    }

}
void Transactions::showVerifyProofResults(bool success, QString fn, QString msg ) {
    state::TimeoutLockObject to( state );
    if (success) {
        dlg::ProofInfo proof;
        if (proof.parseProofText(msg)) {
            dlg::ShowProofDlg dlg(this, fn, proof );
            dlg.exec();
        }
        else {
            control::MessageBox::message(this, "Failure", "Internal error. Unable to decode the results of the proof from the file " + fn + "\n\n" + msg );
        }
    }
    else {
        control::MessageBox::message(this, "Failure", msg );
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

    if (!fileName.endsWith(".proof"))
        fileName += ".proof";

    // Update path
    QFileInfo flInfo(fileName);
    state->updateProofFilesPath(flInfo.path());

    state->generateMwcBoxTransactionProof( selected->txIdx, fileName );
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
    wallet::WalletTransaction * selected = Transactions::getSelectedTransaction();

    if (selected==nullptr)
        return;

    dlg::ShowTransactionDlg showTransDlg(this, *selected);
    showTransDlg.exec();
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
    if ( control::MessageBox::question(this, "Transaction cancellation",
            "Are you sure you want to cancet transaction #" + QString::number(selected->txIdx+1) +
                               ", TXID " + selected->txid, "No", "Yes", true, false) == control::MessageBox::RETURN_CODE::BTN2 ) {
        state->cancelTransaction(*selected);
    }
}

void Transactions::updateCancelTransacton(bool success, int64_t trIdx, QString errMessage) {
    state::TimeoutLockObject to( state );
    if (success) {
        requestTransactions(getSelectedAccount().accountName);

        control::MessageBox::message(this, "Transaction was cancelled", "Transaction number " + QString::number(trIdx+1) + " was successfully cancelled");
    }
    else {
        control::MessageBox::message(this, "Failed to cancel transaction", "Cancel request for transaction number " + QString::number(trIdx+1) + " was failed.\n\n" + errMessage);
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


