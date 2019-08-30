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
    Q_UNUSED(transaction);

    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->transactionType->setText(transactionType);
    ui->processButton->setText(processButtonName);

    ui->mwcLabel->setText( util::nano2one( transInfo.amount ) + " mwc" );
    ui->transactionIdLabel->setText(transInfo.transactionId);
    ui->lockHeightLabel->setText( transInfo.lock_height>nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-" );
    ui->message->setText( transInfo.message );
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
    ui->progress->show();
    handler->ftContinue( transactionFileName );
}

void FileTransaction::hideProgress() {
    ui->progress->hide();
}


}

