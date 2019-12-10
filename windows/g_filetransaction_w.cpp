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

#include "g_filetransaction_w.h"
#include "ui_g_filetransaction.h"
#include "../state/g_Finalize.h"
#include "../util/stringutils.h"
#include <QFileDialog>
#include "../control/messagebox.h"

namespace wnd {


FileTransaction::FileTransaction(QWidget *parent, FileTransactionWndHandler * _handler,
                                 const QString & fileName, const util::FileTransactionInfo & transInfo,
                                 const wallet::WalletTransaction & transaction,
                                 int nodeHeight,
                                 QString transactionType, QString processButtonName) :
        core::NavWnd(parent, _handler->getContext()),
        ui(new Ui::FileTransaction),
        handler(_handler),
        transactionFileName(fileName)
{
    Q_UNUSED(transaction)

    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->transactionType->setText(transactionType);
    ui->processButton->setText(processButtonName);

    ui->mwcLabel->setText( util::nano2one( transInfo.amount ) + " mwc" );
    ui->transactionIdLabel->setText(transInfo.transactionId);
    ui->lockHeightLabel->setText( transInfo.lock_height>nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-" );
    ui->message->setText( transInfo.message );

    if (!handler->needResultTxFileName()) {
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

        QString resFN = fileName;
        if (resFN.endsWith(".response"))
            resFN = resFN.left( resFN.length() - int(strlen(".response")) );

        resFN += ".mwctx";
        ui->resultingTxFileName->setText( resFN );
    }
}

FileTransaction::~FileTransaction() {
    handler->deleteFileTransactionWnd(this);
    delete ui;
}

void FileTransaction::on_cancelButton_clicked() {
    handler->ftBack();
}

void FileTransaction::on_processButton_clicked()
{
    QString resTxFN;
    if ( handler->needResultTxFileName() ) {
        resTxFN = ui->resultingTxFileName->text();
        if (resTxFN.isEmpty()) {
            control::MessageBox::message( this, "Input value", "Please specify the file name for the resulting transaction." );
            ui->resultingTxFileName->setFocus();
            return;
        }

        QPair <bool, QString> res = util::validateMwc713Str(resTxFN);
        if (!res.first) {
            control::MessageBox::message( this, "Input", res.second );
            ui->resultingTxFileName->setFocus();
            return;
        }

    }

    QString walletPassword = handler->getContext()->wallet->getPassword();
    if (walletPassword.isEmpty() ||
        control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::questionText(this,"Confirm Finalize request",
                                      "You are finalizing transaction for " + ui->mwcLabel->text(), "Decline", "Confirm",
                                      false, true, walletPassword, control::MessageBox::RETURN_CODE::BTN2 ) )
    {
            ui->progress->show();
            handler->ftContinue( transactionFileName );
    }
}

void FileTransaction::hideProgress() {
    ui->progress->hide();
}

void FileTransaction::on_resultTransFileNameSelect_clicked()
{

    QString fileName = QFileDialog::getSaveFileName(this, tr("Resulting MWC transaction"),
                                                    handler->getResultTxPath(),
                                                    tr("MWC transaction (*.mwctx)"));


    if (fileName.length()==0)
        return;

    if (!fileName.endsWith(".mwctx"))
           fileName += ".mwctx";

    // Update path
    QFileInfo flInfo(fileName);
    handler->updateResultTxPath(flInfo.path());

    ui->resultingTxFileName->setText(fileName);
}


}


