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

void TransactionData::setBtns(control::RichItem * _ritem, control::RichButton * _cancelBtn,
             control::RichButton * _repostBtn, control::RichButton * _proofBtn,
             QLabel * _noteL) {
    ritem = _ritem;
    cancelBtn = _cancelBtn;
    repostBtn = _repostBtn;
    proofBtn = _proofBtn;
    noteL = _noteL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    QObject::connect(ui->transactionTable, &control::RichVBox::onItemActivated,
                     this, &Transactions::onItemActivated, Qt::QueuedConnection);

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    onSgnWalletBalanceUpdated();
    requestTransactions();

    updateData();

}

Transactions::~Transactions()
{
    delete ui;
}

void Transactions::updateData() {
    ui->transactionTable->clearAll();

    QDateTime current = QDateTime::currentDateTime();
    int expectedConfirmNumber = config->getInputConfirmationNumber();
    QString currentAccount = ui->accountComboBox->currentData().toString();

    int txCnt = 0;

    for ( int idx = allTrans.size()-1; idx>=0 && txCnt<5000; idx--, txCnt++) {
        TransactionData &tr = allTrans[idx];
        const wallet::WalletTransaction &trans = tr.trans;

        // if the node is online and in sync, display the number of confirmations instead of time
        // trans.confirmationTime format: 2020-10-13 04:36:54
        // Expected: Jan 2, 2020 / 2:07am
        QString txTimeStr = trans.confirmationTime;
        if (txTimeStr.isEmpty() || txTimeStr == "None")
            txTimeStr = trans.creationTime;

        QDateTime txTime = QDateTime::fromString(txTimeStr, "HH:mm:ss dd-MM-yyyy");
        txTimeStr = txTime.toString("MMM d, yyyy / H:mmap");
        bool blocksPrinted = false;
        if (trans.confirmed && nodeHeight > 0 && trans.height > 0) {
            int needConfirms = trans.isCoinbase() ? mwc::COIN_BASE_CONFIRM_NUMBER : expectedConfirmNumber;
            // confirmations are 1 more than the difference between the node and transaction heights
            int64_t confirmations = nodeHeight - trans.height + 1;
            if (needConfirms >= confirmations) {
                txTimeStr = "(" + QString::number(confirmations) + "/" + QString::number(needConfirms) + " blocks)";
                blocksPrinted = true;
            }
        }

        control::RichItem *itm = control::createMarkedItem(QString::number(idx), ui->transactionTable,
                                                           trans.canBeCancelled());

        { // First line
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            // Adding Icon and a text
            itm->addWidget( control::createLabel(itm, false, false, "#" + QString::number(trans.txIdx + 1)) ).addFixedHSpacer(10);

            if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::CANCELLED) {
                itm->addWidget(control::createIcon(itm, ":/img/iconClose@2x.svg", control::ROW_HEIGHT,
                                                   control::ROW_HEIGHT))
                        .addWidget(control::createLabel(itm, false, false, "Cancelled"));
            } else if (!trans.confirmed) {
                itm->addWidget(control::createIcon(itm, ":/img/iconUnconfirmed@2x.svg", control::ROW_HEIGHT,
                                                   control::ROW_HEIGHT))
                        .addWidget(control::createLabel(itm, false, false, "Unconfirmed"));
            } else if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::SEND) {
                itm->addWidget(control::createIcon(itm, ":/img/iconSent@2x.svg", control::ROW_HEIGHT,
                                                   control::ROW_HEIGHT))
                        .addWidget(control::createLabel(itm, false, false, "Sent"));
            } else if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::RECEIVE) {
                itm->addWidget(control::createIcon(itm, ":/img/iconReceived@2x.svg", control::ROW_HEIGHT,
                                                   control::ROW_HEIGHT))
                        .addWidget(control::createLabel(itm, false, false, "Received"));
            } else if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::COIN_BASE) {
                itm->addWidget(control::createIcon(itm, ":/img/iconCoinbase@2x.svg", control::ROW_HEIGHT,
                                                   control::ROW_HEIGHT))
                        .addWidget(control::createLabel(itm, false, false, "CoinBase"));
            } else {
                Q_ASSERT(false);
            }

            // Update with time or blocks
            itm->addHSpacer().addWidget(control::createLabel(itm, false, true, txTimeStr));

            itm->pop();
        } // First line

        itm->addWidget(control::createHorzLine(itm));

        control::RichButton *cancelBtn = nullptr;
        control::RichButton *repostBtn = nullptr;

        // Line with amount
        {
            QString amount = util::nano2one(trans.coinNano);

            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            itm->addWidget(control::createLabel(itm, false, false, amount + " MWC", control::FONT_LARGE));

            itm->addHSpacer();

            if (!blocksPrinted && nodeHeight > 0 && trans.height > 0) {
                itm->addWidget(control::createLabel(itm, false, true,
                                                   "Conf: " + QString::number(nodeHeight - trans.height + 1)));
            }
            if (trans.canBeCancelled()) {
                itm->addWidget(new control::RichButton(itm, "Cancel", 60, control::ROW_HEIGHT, "Cancel this transaction and unlock coins"));
                cancelBtn = (control::RichButton *) itm->getCurrentWidget();
                cancelBtn->setCallback(this, "Cancel:" + QString::number(idx));
            }
            // Can be reposted
            if (trans.transactionType == wallet::WalletTransaction::TRANSACTION_TYPE::SEND && !trans.confirmed) {
                itm->addWidget(new control::RichButton(itm, "Repost", 60, control::ROW_HEIGHT, "Report this transaction to the network"));
                repostBtn = (control::RichButton *) itm->getCurrentWidget();
                repostBtn->setCallback(this,
                                       "Repost:" + QString::number(idx) + ":" + currentAccount);
            }
            itm->pop();
        }

        control::RichButton *proofBtn = nullptr;

        // Line with ID
        {
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            itm->addWidget(control::createLabel(itm, false, true, trans.txid, control::FONT_SMALL));
            itm->addHSpacer();
            if (trans.proof) {
                itm->addWidget(new control::RichButton(itm, "Proof", 60, control::ROW_HEIGHT,
                                                       "Generate proof file for this transaction. Proof file can be validated by public at MWC Block Explorer"));
                proofBtn = (control::RichButton *) itm->getCurrentWidget();
                proofBtn->setCallback(this, "Proof:" + QString::number(idx));
            }
            itm->pop();
        }


        // Address field...
        if (!trans.address.isEmpty()) {
            itm->addWidget(control::createLabel(itm, false, true, trans.address, control::FONT_SMALL));
        }

        // And the last optional line is comment
        QString txnNote = config->getTxNote(trans.txid);

        itm->addWidget(control::createLabel(itm, true, false, txnNote));
        QLabel *noteL = (QLabel *) itm->getCurrentWidget();
        if (txnNote.isEmpty())
            noteL->hide();

        itm->apply();

        tr.setBtns(itm, cancelBtn, repostBtn, proofBtn, noteL);

        ui->transactionTable->addItem(itm);
    }
    ui->transactionTable->apply();
}

void Transactions::richButtonPressed(control::RichButton * button, QString coockie) {
    Q_UNUSED(button);

    QStringList res = coockie.split(':');
    if (res.size()<2) {
        Q_ASSERT(false);
        return;
    }

    QString id = res[0];
    int idx = res[1].toInt();

    if (idx<0 || idx>=allTrans.size()) {
        Q_ASSERT(false); // might happen during refresh.
        return;
    }

    wallet::WalletTransaction tx2process = allTrans[idx].trans;

    if (id=="Cancel") {
        util::TimeoutLockObject to("Transactions");

        if (control::MessageBox::questionText(this, "Transaction cancellation",
                                              "Are you sure you want to cancel transaction #" +
                                              QString::number(tx2process.txIdx + 1) +
                                              ", TXID " + tx2process.txid,
                                              "No", "Yes",
                                              "Keep my transaction, I am expecting it to be finalized",
                                              "Continue and cancel the transaction, I am not expecting it to be finalized",
                                              true, false) == core::WndManager::RETURN_CODE::BTN2) {

            wallet->requestCancelTransacton(account, QString::number(tx2process.txIdx));
        }
    }
    else if (id=="Repost") {
        if (res.size()<3) {
            Q_ASSERT(false);
            return;
        }
        QString currentAccount = res[2];
        wallet->repost( currentAccount, tx2process.txIdx, config->isFluffSet() );
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Processing repost for transaction #" + QString::number(tx2process.txIdx+1) );
    }
    else if ( id == "Proof" ) {
        util::TimeoutLockObject to("Transactions");

        if (!tx2process.proof) {
            Q_ASSERT(false);
            control::MessageBox::messageText(this, "Need info",
                                             "Please select qualify transaction to generate a proof.");
            return;
        }

        QString fileName = QFileDialog::getSaveFileName(this, tr("Create transaction proof file"),
                                                        config->getPathFor("Transactions"),
                                                        tr("transaction proof (*.proof)"));

        if (fileName.length() == 0)
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

        wallet->generateTransactionProof(QString::number(tx2process.txIdx), fileName);
    }
    else {
        // Unexpected id
        Q_ASSERT(false);
    }

}


void Transactions::onSgnTransactions( QString acc, QString height, QVector<QString> transactions) {
    Q_UNUSED(height)

    if (acc != ui->accountComboBox->currentData().toString() )
        return;

    ui->progressFrame->hide();
    ui->transactionTable->show();

    account = acc;
    allTrans.clear();

    for (QString & t : transactions ) {
        TransactionData dt;
        dt.trans = wallet::WalletTransaction::fromJson(t);
        allTrans.push_back( dt );
    }

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
    nodeHeight = -1;

    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;

    ui->progressFrame->show();
    ui->transactionTable->hide();
    ui->transactionTable->clearAll();

    // !!! Note, order is important even it is async. We want node status be processed first..
    wallet->requestNodeStatus(); // Need to know th height.
    wallet->requestTransactions(account, true);
    updateData();
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
    const wallet::WalletTransaction & trans = allTrans[0].trans;
    QString csvHeaders = trans.getCSVHeaders();
    exportRecords << csvHeaders;
    QString csvValues = trans.toStringCSV();
    exportRecords << csvValues;

    // now retrieve the remaining transactions and add them to our list
    for ( int idx=1; idx < allTrans.size(); idx++) {
        const wallet::WalletTransaction & trans = allTrans[idx].trans;
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

void Transactions::onItemActivated(QString itemId) {
    int idx = itemId.toInt();
    if (idx<0 || idx>=allTrans.size()) {
        Q_ASSERT(false);
        return;
    }

    QString account = ui->accountComboBox->currentData().toString();
    const wallet::WalletTransaction & selected = allTrans[idx].trans;

    // respond will come at updateTransactionById
    wallet->requestTransactionById(account, QString::number(selected.txIdx) );

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
    if (  showTransDlg.exec() == QDialog::Accepted ) {
        if (txnNote != showTransDlg.getTransactionNote() ) {
            txnNote = showTransDlg.getTransactionNote();
            if (txnNote.isEmpty()) {
                config->deleteTxNote(transaction.txid);
            } else {
                // add new note or update existing note for this commitment
                config->updateTxNote(transaction.txid, txnNote);
            }

            // Updating the UI
            for (const auto & tx : allTrans) {
                if (tx.trans.txid == transaction.txid) {
                    tx.noteL->setText(txnNote);
                    if (txnNote.isEmpty())
                        tx.noteL->hide();
                    else
                        tx.noteL->show();
                }
            }
        }
    }

}

void Transactions::on_accountComboBox_activated(int index)
{
    Q_UNUSED(index);
    QString account = ui->accountComboBox->currentData().toString();
    if (!account.isEmpty())
        wallet->switchAccount(account);
    requestTransactions();
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

}


