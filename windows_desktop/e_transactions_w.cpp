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
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include <QDebug>
#include "../dialogs_desktop/e_showproofdlg.h"
#include "../dialogs_desktop/e_showtransactiondlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/e_transactions_b.h"
#include "../core/global.h"
#include <QStringList>
#include "zz_utils.h"
#include <QPaintEvent>

// It is exception for Mobile, CSV export not likely needed into mobile wallet
#include <QJsonArray>

#include "../util/Files.h"

namespace wnd {

TransactionRecord::TransactionRecord(QString _id, control::RichVBox *_parent, const wallet::WalletTransaction & trans,
            int _expectedConfirmNumber, const QString & _currentAccountPath,
            control::RichButtonPressCallback * _btnCallBack,
            bridge::Config * _config, bridge::Wallet * _wallet,
            int intialHeight) :
        QWidget(_parent),
        id(_id),
        parent(_parent),
        trans(trans),
        expectedConfirmNumber(_expectedConfirmNumber),
        currentAccountPath(_currentAccountPath),
        btnCallBack(_btnCallBack),
        config(_config),
        wallet(_wallet)
{
    setMinimumHeight(intialHeight);   // 1. reserve *at least* that much
    placeholderLayout = new QVBoxLayout(this);
    placeholderLayout->setContentsMargins(0,0,0,0);
}

void TransactionRecord::updateNote(const QString & note) {
    if (noteL) {
        noteL->setText(note);
        if (note.isEmpty())
            noteL->hide();
        else
            noteL->show();
    }
}

void TransactionRecord::paintEvent(QPaintEvent * evt) {
    Q_UNUSED(evt);

    if (control==nullptr) {
        control = buildRecordContent();
        placeholderLayout->addWidget(control);
        // set some non zero small height that is smaller than control has.
        parent->updateLayouts();
    }
}

// build the control instance
control::RichItem * TransactionRecord::buildRecordContent() {
    // if the node is online and in sync, display the number of confirmations instead of time
    // trans.confirmationTime format: 2020-10-13 04:36:54
    // Expected: Jan 2, 2020 / 2:07am
    QString txTimeStr = trans.confirmationTime;
    if (txTimeStr.isEmpty() || txTimeStr == "None")
        txTimeStr = trans.creationTime;

    QDateTime txTime = QDateTime::fromString(txTimeStr, Qt::ISODate);
    txTimeStr = txTime.toString("MMM d, yyyy / H:mmap");
    bool blocksPrinted = false;
    if (trans.confirmed && trans.chain_height > 0 && trans.output_height > 0) {
        int needConfirms = trans.isCoinbase() ? mwc::COIN_BASE_CONFIRM_NUMBER : expectedConfirmNumber;
        // confirmations are 1 more than the difference between the node and transaction heights
        qint64 confirmations = trans.chain_height - trans.output_height + 1;
        if (needConfirms >= confirmations) {
            txTimeStr = "(" + QString::number(confirmations) + "/" + QString::number(needConfirms) + " blocks)";
            blocksPrinted = true;
        }
    }

    control::RichItem *itm = control::createMarkedItem(id, this,
                                                       trans.canBeCancelled(), "");
    itm->setParent(parent);

    // We need to build the widget in order it will be able to calculat ethe size.
    // We are not adding buttons, will do that dynamically

    { // First line
        itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
        // Adding Icon and a text
        itm->addWidget( control::createLabel(itm, false, false, "#" + QString::number(trans.txIdx + 1)) ).addFixedHSpacer(10);

        if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::CANCELLED) {
            itm->addWidget(control::createIcon(itm, ":/img/iconClose.svg", control::ROW_HEIGHT,
                                               control::ROW_HEIGHT))
                    .addWidget(control::createLabel(itm, false, false, "Cancelled"));
        } else if (!trans.confirmed) {
            itm->addWidget(control::createIcon(itm, ":/img/iconUnconfirmed.svg", control::ROW_HEIGHT,
                                               control::ROW_HEIGHT))
                    .addWidget(control::createLabel(itm, false, false, "Unconfirmed"));
        } else if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::SEND) {
            itm->addWidget(control::createIcon(itm, ":/img/iconSent.svg", control::ROW_HEIGHT,
                                               control::ROW_HEIGHT))
                    .addWidget(control::createLabel(itm, false, false, "Sent"));
        } else if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::RECEIVE) {
            itm->addWidget(control::createIcon(itm, ":/img/iconReceived.svg", control::ROW_HEIGHT,
                                               control::ROW_HEIGHT))
                    .addWidget(control::createLabel(itm, false, false, "Received"));
        } else if (trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::COIN_BASE) {
            itm->addWidget(control::createIcon(itm, ":/img/iconCoinbase.svg", control::ROW_HEIGHT,
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

    // Line with amount
    {
        QString amount = util::nano2one(trans.coinNano);

        itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
        itm->addWidget(control::createLabel(itm, false, false, amount + " MWC", control::FONT_LARGE));

        itm->addHSpacer();

        if (!blocksPrinted && trans.chain_height > 0 && trans.output_height > 0) {
            itm->addWidget(control::createLabel(itm, false, true,
                                               "Conf: " + QString::number(trans.chain_height - trans.output_height + 1)));
        }
        Q_ASSERT(btnCallBack);
        if (trans.canBeCancelled()) {
            itm->addWidget(new control::RichButton(itm, "Cancel", 60, control::ROW_HEIGHT, "Cancel this transaction and unlock coins"));
            control::RichButton * cancelBtn = (control::RichButton *) itm->getCurrentWidget();
            cancelBtn->setCallback(btnCallBack, "Cancel:" + id);
        }
        // Can be reposted
        if (trans.transactionType == wallet::WalletTransaction::TRANSACTION_TYPE::SEND && !trans.confirmed && wallet->hasFinalizedData(trans.txid) ) {
            itm->addWidget(new control::RichButton(itm, "Repost", 60, control::ROW_HEIGHT, "Report this transaction to the network"));
            control::RichButton * repostBtn = (control::RichButton *) itm->getCurrentWidget();
            repostBtn->setCallback(btnCallBack,
                                   "Repost:" + id + ":" + currentAccountPath);
        }
        itm->pop();
    }

    // Line with ID
    {
        itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
        itm->addWidget(control::createLabel(itm, false, true, trans.txid, control::FONT_SMALL));
        itm->addHSpacer();

        if (trans.canBeCancelled()) {
            if ( ((trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::RECEIVE) != 0) && wallet->hasReceiveSlatepack(trans.txid) ) {
                itm->addWidget(new control::RichButton(itm, "View Slatepack", 120, control::ROW_HEIGHT, "View response Slatepack"));
                control::RichButton * btn = (control::RichButton *) itm->getCurrentWidget();
                btn->setCallback(btnCallBack, "SlatepackReceive:" + trans.txid);
            }

            if ( ((trans.transactionType & wallet::WalletTransaction::TRANSACTION_TYPE::SEND) != 0) && wallet->hasSendSlatepack(trans.txid) ) {
                itm->addWidget(new control::RichButton(itm, "View Slatepack", 120, control::ROW_HEIGHT, "View send init Slatepack"));
                control::RichButton * btn = (control::RichButton *) itm->getCurrentWidget();
                btn->setCallback(btnCallBack, "SlatepackSend:" + trans.txid);
            }
        }

        if ( wallet->generateTransactionProof(trans.txid).startsWith("{") ) {
            itm->addWidget(new control::RichButton(itm, "Proof", 60, control::ROW_HEIGHT,
                                                   "Generate proof file for this transaction. Proof file can be validated by public at MWC Block Explorer"));
            control::RichButton * proofBtn = (control::RichButton *) itm->getCurrentWidget();
            proofBtn->setCallback(btnCallBack, "Proof:" + id);
        }
        itm->pop();
    }


    // Address field...
    if (!trans.address.isEmpty()) {
        itm->hbox().setContentsMargins(0, 0, 0, 0);
        itm->addWidget(control::createLabel(itm, false, true, trans.address, control::FONT_SMALL));
        itm->addHSpacer().pop();
    }

    {
        QString txnNote = config->getTxNote(trans.txid);

        itm->hbox().setContentsMargins(0, 0, 0, 0);
        itm->addWidget(control::createLabel(itm, true, false, txnNote));
        //itm->addHSpacer();
        itm->pop();

        noteL = (QLabel *) itm->getCurrentWidget();
        if (txnNote.isEmpty())
            noteL->hide();
    }
    Q_ASSERT(noteL);

    itm->apply();
    return itm;
}


///////////////////////////////////////////////////////////////////////////////////////

static QString csvStr(QString str) {
    str.replace("\"", "");
    return "\""+str+"\"";
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
    util = new bridge::Util(this);

    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Transactions::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect(ui->transactionTable, &control::RichVBox::onItemActivated,
                     this, &Transactions::onItemActivated, Qt::QueuedConnection);

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    showIntegrityAccount = QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);

    updateAccountsData(wallet, ui->accountComboBox, !showIntegrityAccount, false);
    requestTransactions(true);

    updateData(true, {});

}

Transactions::~Transactions()
{
    delete ui;
}

void Transactions::updateData(bool resetScroller, const QVector<int> & heights) {
    ui->transactionTable->hide();
    ui->transactionTable->clearAll(resetScroller);

    QDateTime current = QDateTime::currentDateTime();
    int expectedConfirmNumber = config->getInputConfirmationNumber();
    QString currentAccountPath = accountComboData2AccountPath(ui->accountComboBox->currentData().toString()).second;

    int txCnt = 0;

    for ( int idx = allTrans.size()-1; idx>=0 && txCnt<5000; idx--, txCnt++) {
        TransactionData &tr = allTrans[idx];
        const wallet::WalletTransaction &trans = tr.trans;

        int h = 135;
        if (idx<heights.size())
            h = heights[idx];

        tr.record = new TransactionRecord(QString::number(idx), ui->transactionTable,
            trans, expectedConfirmNumber, currentAccountPath,
            this, config, wallet, h);

        ui->transactionTable->addItem(tr.record);
    }
    ui->transactionTable->apply();
    ui->transactionTable->show();
}

void Transactions::richButtonPressed(control::RichButton * button, QString coockie) {
    Q_UNUSED(button)
    QStringList res = coockie.split(':');
    if (res.size()<2) {
        Q_ASSERT(false);
        return;
    }

    QString id = res[0];

    if (id == "SlatepackReceive") {
        wallet->viewReceiveSlatepack(res[1]);
        return;
    }
    else if (id == "SlatepackSend") {
        wallet->viewSendSlatepack(res[1]);
        return;
    }

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

            QString error = wallet->cancelTransacton(tx2process.txid);
            if (error.isEmpty()) {
                requestTransactions(false);
            }
            else {
                control::MessageBox::messageText(this, "Failed to cancel transaction",
                    "Cancel request for transaction " + tx2process.txid + " has failed.\n\n");
            }
        }
    }
    else if (id=="Repost") {
        if (res.size()<3) {
            Q_ASSERT(false);
            return;
        }
        QString currentAccount = res[2];
        QString err = wallet->repostTransaction(tx2process.txid, config->isFluffSet() );

        if (err.isEmpty()) {
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "Transaction " + tx2process.txid + " was successfully reposted." );
            core::getWndManager()->messageTextDlg("Repost", "Transaction " + tx2process.txid + " was successfully reposted." );
        }
        else {
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::CRITICAL, "Failed to repost transaction " + tx2process.txid  + ". " + err );
            core::getWndManager()->messageTextDlg("Repost", "Failed to repost transaction " + tx2process.txid + ".\n\n" + err );
        }
    }
    else if ( id == "Proof" ) {
        util::TimeoutLockObject to("Transactions");

        QString proof = wallet->generateTransactionProof(tx2process.txid);

        if (!proof.startsWith("{")) {
            Q_ASSERT(false);
            control::MessageBox::messageText(this, "Need info",
                                             "Please select qualify transaction to generate a proof.\n\n" + proof);
            return;
        }

        QString fileName = util->getSaveFileName("Create transaction proof file",
                              "Transactions",
                              "transaction proof (*.proof)", ".proof");
        if (fileName.isEmpty())
            return;

        // In order to view proof, we need to decode it
        QString proofres = wallet->verifyTransactionProof(proof);
        if (!proofres.startsWith("{")) {
            control::MessageBox::messageText(this, "Proof generation error",
                                             "Unable to validate generated proof.\n\n" + proofres );
        }

        if ( !util::writeTextFile( fileName, {proof} ) ) {
            control::MessageBox::messageText(this, "File Save Error",
                                             "Unable to store the proof in the resulting file " + fileName );
            return;
        }


        dlg::ProofInfo proofInfo;
        proofInfo.parseProofRes(proofres);
        dlg::ShowProofDlg dlg(this, fileName, proofInfo );
        dlg.exec();
    }
    else {
        // Unexpected id
        Q_ASSERT(false);
    }
}

void Transactions::requestTransactions(bool resetScroller) {

    QVector<int> heights;
    if (!resetScroller)
        heights = getItemsHeights();

    allTrans.clear();

    QString accountPath = accountComboData2AccountPath(ui->accountComboBox->currentData().toString()).second;
    if (accountPath.isEmpty())
        return;

    // !!! Note, order is important even it is async. We want node status be processed first..
    QJsonArray txsJson = wallet->getTransactions(accountPath);

    for (const auto & t : txsJson ) {
        TransactionData dt;
        dt.trans = wallet::WalletTransaction::fromJson(t.toObject());
        dt.tx_note = config->getTxNote(dt.trans.txid);
        allTrans.push_back( dt );

    }
    updateData(resetScroller, heights);
}

QVector<int> Transactions::getItemsHeights() {
    QVector<int> res;
    for (auto & t : allTrans) {
        res.push_back( t.record->getCurrentHeight() );
    }
    return res;
}

void Transactions::on_validateProofButton_clicked()
{
    util::TimeoutLockObject to( "Transactions" );

    QString fileName = util->getOpenFileName("Open proof file",
                                 "Transactions",
                                 "transaction proof (*.proof);;All files (*.*)");
    if (fileName.isEmpty())
        return;

    QStringList proofData = util::readTextFile(fileName);
    if (proofData.size()!=1 || proofData[0].length()<3 || proofData[0][0]!='{') {
        control::MessageBox::messageText(this, "Invalid proof", "Provided file doesn't have valid transaction proof record");
        return;
    }

    QString proofres = wallet->verifyTransactionProof(proofData[0]);

    if (proofres.startsWith("{")) {
        dlg::ProofInfo proof;
        proof.parseProofRes(proofres);
        dlg::ShowProofDlg dlg(this, fileName, proof );
        dlg.exec();
    }
    else {
        control::MessageBox::messageText(this, "Invalid proof", proofres );
    }
}

void Transactions::on_exportButton_clicked() {
    util::TimeoutLockObject to("Transactions");

    if (allTrans.isEmpty()) {
        control::MessageBox::messageText(this, "Export Error",
                    "You don't have any transactions to export.");
        return;
    }

    QString exportingFileName = util->getSaveFileName("Export Transactions",
                                             "TxExportCsv",
                                             "Export Options (*.csv)",
                                             ".csv");

    if (exportingFileName.isEmpty())
        return;

    // qt-wallet displays the transactions last to first
    // however when exporting the transactions, we want to export first to last
    QStringList exportRecords;

    // retrieve the first transaction and get the CSV headers
    const wallet::WalletTransaction & trans = allTrans[0].trans;
    QString csvHeaders = trans.getCSVHeaders({"Tx Note", "Transaction Messages"});
    exportRecords << csvHeaders;

    // now retrieve the remaining transactions and add them to our list
    for ( int idx=0; idx < allTrans.size(); idx++) {
        const auto & tx = allTrans[idx];
        QStringList messages;
        for (const auto & msg : tx.trans.messages) {
            messages.push_back(msg.message);
        }
        QString csvValues = tx.trans.toStringCSV( { csvStr(tx.tx_note), csvStr(messages.join(", ")) } );
        exportRecords << csvValues;
    }
    // Note: Mobile doesn't expect to export anything. That is why we are breaking bridge rule here and usung util::writeTextFile directly
    // warning: When using a debug build, avoid testing with an existing file which has
    //          read-only permissions. writeTextFile will hit a Q_ASSERT causing qt-wallet
    //          to crash.
    bool exportOk = util::writeTextFile(exportingFileName, exportRecords);
    if (!exportOk)
    {
        control::MessageBox::messageText(this, "Error", "Export unable to write to file: " + exportingFileName);
    }
    else
    {
        // some users may have a large number of transactions which take time to write to the file
        // so indicate when the file write has completed
        control::MessageBox::messageText(this, "Success", "Exported transactions to file: " + exportingFileName);
    }
}

void Transactions::onItemActivated(QString itemId) {
    int idx = itemId.toInt();
    if (idx<0 || idx>=allTrans.size()) {
        Q_ASSERT(false);
        return;
    }

    QString accountPath = accountComboData2AccountPath(ui->accountComboBox->currentData().toString()).second;
    if (accountPath.isEmpty()) {
        Q_ASSERT(false);
        return;
    }

    TransactionData & selectedTx = allTrans[idx];

    // respond will come at updateTransactionById
    util::TimeoutLockObject to("Transactions");

    QVector<wallet::WalletOutput> outputs;
    QVector<QString> txOutputs;
    txOutputs.append(selectedTx.trans.inputs);
    txOutputs.append(selectedTx.trans.outputs);

    QJsonArray txOutputsJson = wallet->getOutputsByCommits(accountPath, txOutputs);
    for (const auto & outJson : txOutputsJson ) {
        outputs.push_back(wallet::WalletOutput::fromJson(outJson.toObject()));
    }

    dlg::ShowTransactionDlg showTransDlg(this, selectedTx.trans, outputs,
                                                        selectedTx.trans.messages, selectedTx.tx_note);

    if (showTransDlg.exec() == QDialog::Accepted) {
        if ( selectedTx.tx_note != showTransDlg.getTransactionNote()) {
            selectedTx.tx_note = showTransDlg.getTransactionNote();
            if (selectedTx.tx_note.isEmpty()) {
                config->deleteTxNote(selectedTx.trans.txid);
            } else {
                // add new note or update existing note for this commitment
                config->updateTxNote(selectedTx.trans.txid, selectedTx.tx_note);
            }

            // Updating the UI
            for (const auto &tx : allTrans) {
                if (tx.trans.txid == selectedTx.trans.txid) {
                    tx.record->updateNote(selectedTx.tx_note);
                }
            }
        }
    }

}

void Transactions::on_accountComboBox_activated(int index)
{
    Q_UNUSED(index);
    QString accountPath = accountComboData2AccountPath(ui->accountComboBox->currentData().toString()).second;
    if (!accountPath.isEmpty())
        wallet->switchAccountById(accountPath);
    requestTransactions(true);
}


void Transactions::onSgnWalletBalanceUpdated() {
    updateAccountsData(wallet, ui->accountComboBox, !showIntegrityAccount, false);
    requestTransactions(false);
}

}

