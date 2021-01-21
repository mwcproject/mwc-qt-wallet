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

#include "g_filetransaction_receive_w.h"
#include "ui_g_filetransaction_receive.h"
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/g_sendconfirmationdlg.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/e_receive_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../core/global.h"

namespace wnd {

FileTransactionReceive::FileTransactionReceive(QWidget *parent,
                                 const QString & _fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
                                 int nodeHeight) :
        core::NavWnd(parent),
        ui(new Ui::FileTransactionReceive),
        fileNameOrSlatepack(_fileNameOrSlatepack)
{
    ui->setupUi(this);
    txUuid = transInfo.transactionId;

    receive = new bridge::Receive(this);
    wallet = new bridge::Wallet(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);

    QObject::connect( receive, &bridge::Receive::sgnHideProgress,
                      this, &FileTransactionReceive::onSgnHideProgress, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    Q_ASSERT(!transInfo.amount_fee_not_defined);
    ui->mwcLabel->setText( util::nano2one( transInfo.amount ) + " MWC" );

    ui->transactionIdLabel->setText(txUuid);
    ui->lockHeightLabel->setText( transInfo.lock_height>nodeHeight ? util::longLong2Str(transInfo.lock_height) : "-" );
    ui->receiverAddressLabel->setText(transInfo.fromAddress.isEmpty() ? "-" : transInfo.fromAddress);
    ui->senderMessage->setText( transInfo.senderMessage );
}

FileTransactionReceive::~FileTransactionReceive() {
    delete ui;
}

void FileTransactionReceive::on_cancelButton_clicked() {
    receive->cancelReceive();
}

void FileTransactionReceive::on_processButton_clicked()
{
    util::TimeoutLockObject to( "FileTransactionReceive" );

    QString description = ui->receiverMessage->toPlainText().trimmed().replace('\n', ' ');

    {
        QString valRes = util->validateMwc713Str(description);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->receiverMessage->setFocus();
            return;
        }
    }

    ui->progress->show();

    if (fileNameOrSlatepack.startsWith("BEGINSLATE")) {
        receive->receiveSlatepack( fileNameOrSlatepack, description );
    }
    else
    { // file
        receive->receiveFile( fileNameOrSlatepack, description );
    }
}

void FileTransactionReceive::onSgnHideProgress() {
    ui->progress->hide();
}

}


