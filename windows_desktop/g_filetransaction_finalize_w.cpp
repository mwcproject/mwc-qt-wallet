
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

#include "g_filetransaction_finalize_w.h"
#include "ui_g_filetransaction_finalize.h"
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/g_sendconfirmationdlg.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/g_finalize_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../core/global.h"

namespace wnd {

FileTransactionFinalize::FileTransactionFinalize(QWidget *parent,
                                 const QString & _fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                                 int nodeHeight) :
        core::NavWnd(parent),
        ui(new Ui::FileTransactionFinalize),
        fileNameOrSlatepack(_fileNameOrSlatepack)
{
    ui->setupUi(this);
    txUuid = transInfo.transactionId;

    finalize = new bridge::Finalize(this);
    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);

    QObject::connect( finalize, &bridge::Finalize::sgnHideProgress,
                      this, &FileTransactionFinalize::onSgnHideProgress, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnTransactionById,
                      this, &FileTransactionFinalize::sgnTransactionById, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    if (transInfo.amount_fee_not_defined) {
        ui->mwcLabel->setText("-");
        // Requesting tranbsaction info form the wallet.
        // It is a normal case, compact slate doesn;t have all info
        wallet->requestTransactionById( wallet->getCurrentAccountName(), transInfo.transactionId );
        ui->progress->show();
    }
    else {
        ui->mwcLabel->setText(util::nano2one(transInfo.amount) + " MWC");
    }

    ui->transactionIdLabel->setText(txUuid);
    ui->lockHeightLabel->setText( transInfo.lock_height>nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-" );
    ui->receiverAddressLabel->setText(transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress);
    ui->senderMessage->setText(transInfo.senderMessage);
    ui->receiverMessage->setText( transInfo.receiverMessage );

    if (!finalize->needResultTxFileName()) {
        QSize rc = ui->resultLocationFrame->frameSize();
        ui->resultLocationFrame->hide();

        QRect cbRc = ui->cancelButton->frameGeometry();
        cbRc.moveTop( cbRc.top() - rc.height() );
        ui->cancelButton->setGeometry( cbRc );

        cbRc = ui->processButton->frameGeometry();
        cbRc.moveTop( cbRc.top() - rc.height() );
        ui->processButton->setGeometry( cbRc );

        ui->primaryFrame->setMinimumHeight( ui->primaryFrame->minimumHeight() - rc.height() );
    }
    else
    { // set default file name if possible
        if (!fileNameOrSlatepack.startsWith("BEGINSLATE")) {
            // it is a filename
            QString resFN = fileNameOrSlatepack;
            if (resFN.endsWith(".response"))
                resFN = resFN.left(resFN.length() - int(strlen(".response")));

            resFN += ".mwctx";
            ui->resultingTxFileName->setText(resFN);
        }
    }
}

FileTransactionFinalize::~FileTransactionFinalize() {
    delete ui;
}

void FileTransactionFinalize::sgnTransactionById( bool success, QString account, QString height, QString transaction,
                         QVector<QString> outputs, QVector<QString> messages ) {

    Q_UNUSED(account)
    Q_UNUSED(height)
    Q_UNUSED(outputs)

    ui->progress->hide();

    if (!success)
        return;

    wallet::WalletTransaction txDetails = wallet::WalletTransaction::fromJson(transaction);

    ui->mwcLabel->setText( util::nano2one( std::abs(txDetails.coinNano) ) + " MWC" );
    if (messages.length()>0) { // my message need to read form the saved transaction data
        QString senderMsg = messages[0];
        if (senderMsg.isEmpty())
            senderMsg = "None";
        ui->senderMessage->setText(senderMsg);
    }
}


void FileTransactionFinalize::on_cancelButton_clicked() {
    finalize->cancelFileFinalization();
}

void FileTransactionFinalize::on_processButton_clicked()
{
    util::TimeoutLockObject to( "FileTransactionFinalize" );

    QString resTxFN;
    if ( finalize->needResultTxFileName() ) {
        resTxFN = ui->resultingTxFileName->text();
        if (resTxFN.isEmpty()) {
            control::MessageBox::messageText( this, "Input value", "Please specify the file name for the resulting transaction." );
            ui->resultingTxFileName->setFocus();
            return;
        }

        QPair <bool, QString> res = util::validateMwc713Str(resTxFN);
        if (!res.first) {
            control::MessageBox::messageText( this, "Input", res.second );
            ui->resultingTxFileName->setFocus();
            return;
        }
    }
    else {
        // Check if node healthy first
        if (!finalize->isNodeHealthy()) {
            control::MessageBox::messageText(this, "Unable to finalize", "Your MWC Node, that wallet connected to, is not ready to finalize transactions.\n"
                                                                         "MWC Node need to be connected to few peers and finish blocks synchronization process");
            return;
        }
    }
    QString walletPasswordHash = wallet->getPasswordHash();
    if (!walletPasswordHash.isEmpty()) {
            dlg::SendConfirmationDlg confirmDlg(this, "Confirm Finalize Request",
                                            "You are finalizing transaction for " + ui->mwcLabel->text(),
                                            1.0, walletPasswordHash );
            if (confirmDlg.exec() != QDialog::Accepted)
                return;
    }
    ui->progress->show();

    if (fileNameOrSlatepack.startsWith("BEGINSLATE")) {
        finalize->finalizeSlatepack( fileNameOrSlatepack, txUuid, resTxFN, config->isFluffSet() );
    }
    else
    { // file
        finalize->finalizeFile( fileNameOrSlatepack, resTxFN, config->isFluffSet() );
    }
}

void FileTransactionFinalize::onSgnHideProgress() {
    ui->progress->hide();
}

void FileTransactionFinalize::on_resultTransFileNameSelect_clicked()
{

    QString fileName = util->getSaveFileName("Resulting MWC transaction",
                                             "resultTx",
                                             "MWC transaction (*.mwctx)",
                                             ".mwctx");

    if (fileName.isEmpty())
        return;

    ui->resultingTxFileName->setText(fileName);
}

}


