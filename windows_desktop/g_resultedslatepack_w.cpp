// Copyright 2021 The MWC Developers
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

#include "g_resultedslatepack_w.h"
#include "ui_g_resultedslatepack_w.h"
#include "../control_desktop/QrCodeWidget.h"
#include "../control_desktop/messagebox.h"
#include "../bridge/statemachine_b.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wallet_b.h"
#include "../util/Json.h" // Just for a contant value

namespace wnd {

const QString INIT_TAG = "InitialSP";
const QString FINALIZE_TAG = "FinalizeSP";


ResultedSlatepack::ResultedSlatepack(QWidget *parent, QString slatepack, int backStateId, QString txExtension, bool enableFinalize) :
        core::NavWnd(parent),
        ui(new Ui::ResultedSlatepack) {
    ui->setupUi(this);
    this->backStateId = backStateId;
    this->txExtension = txExtension;
    this->slatepack = slatepack;
    this->enableFinalize = enableFinalize;

    stateMachine = new bridge::StateMachine(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);
    wallet = new bridge::Wallet(this);

    QObject::connect(wallet, &bridge::Wallet::sgnDecodeSlatepack,
                     this, &wnd::ResultedSlatepack::onSgnDecodeSlatepack, Qt::QueuedConnection);
    QObject::connect(wallet, &bridge::Wallet::sgnFinalizeSlatepack,
                     this, &wnd::ResultedSlatepack::sgnFinalizeSlatepack, Qt::QueuedConnection);

    ui->qr_code_window->setContent(slatepack);
    ui->slatepackContent->setText(slatepack);

    if (!enableFinalize) {
        // No finalize, hiding the UI
        ui->finalizeSlatepack->hide();
        ui->finalizeSlatepackBtn->hide();
        ui->horizontalSpacer_7->changeSize(1,1);
    }
    else {
        ui->finalizeSlatepackBtn->setEnabled(false);

        // Requesting UUID for this transaction
        initiateSlateVerification(slatepack, INIT_TAG);
    }
}

ResultedSlatepack::~ResultedSlatepack() {
    delete ui;
}

void ResultedSlatepack::on_backButton_clicked() {
    stateMachine->setActionWindow(backStateId);
}

void ResultedSlatepack::on_saveQrImageBtn_clicked() {
    QString fileName = util->getSaveFileName("Save QR code image",
                                             "ResultedSlatepack",
                                             "QR code Image (*.png)", ".png");

    if (fileName.isEmpty())
        return;

    ui->qr_code_window->generateQrImage(fileName);
}

void ResultedSlatepack::on_saveSlatepackBtn_clicked() {
    QString fileName = util->getSaveFileName("Save Slatepack",
                                             "ResultedSlatepack",
                                             "Slatepack tramsaction (*" + txExtension + ")", txExtension);

    if (fileName.isEmpty())
        return;

    // Write some text into the file
    util->writeTextFile(fileName, {slatepack});
}

void ResultedSlatepack::on_backButton_2_clicked() {
    // It is duplicate UI button
    on_backButton_clicked();
}

void ResultedSlatepack::on_finalizeSlatepack_textChanged() {
    if (spInProgress.isEmpty()) {
        QString sp = ui->finalizeSlatepack->toPlainText().trimmed();
        initiateSlateVerification(sp, FINALIZE_TAG);
    }
}

void ResultedSlatepack::initiateSlateVerification(const QString &slate2check, QString tag) {
    if (slate2check.isEmpty())
        return;

    spInProgress = slate2check;
    wallet->decodeSlatepack(slate2check, tag);
}

void ResultedSlatepack::sgnFinalizeSlatepack( QString tagId, QString error, QString txUuid ) {
    if (tagId != "ResultedSlatepack")
        return;

    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Finalization error", "Unable to finalize the transaction.\n" + error);
        return;
    }

    Q_ASSERT(txUuid == transactionUUID);
    control::MessageBox::messageText(this,"Finalize Transaction", "Transaction " + txUuid + " was finalized successfully.");
    // pressing back button. We are done here
    on_backButton_clicked();
}

void ResultedSlatepack::on_finalizeSlatepackBtn_clicked() {
    if (slate2finalize.isEmpty())
        return;

    // disabling to prevent double clicking...
    ui->finalizeSlatepackBtn->setEnabled(false);

    wallet->finalizeSlatepack(slate2finalize, false, "ResultedSlatepack");
}

void ResultedSlatepack::onSgnDecodeSlatepack(QString tag, QString error, QString slatepack, QString slateJson, QString content, QString sender, QString recipient) {
    Q_UNUSED(recipient)
    Q_UNUSED(sender)

    spInProgress = "";
    ui->finalizeSlatepackBtn->setEnabled(false);

    if (!error.isEmpty()) {
        // we don't report error..
        return;
    }

    if (tag==FINALIZE_TAG) {
        if (content != "SendResponse") {
            if (lastReportedError != 2) {
                lastReportedError = 2;
                control::MessageBox::messageText(this, "Wrong Slatepack",
                                                 "Here you can finalize only the Slatepack from this transaction only. This slatepack doesn't have Response data.");
                ui->finalizeSlatepack->setFocus();
            }
            return;
        }
    }

    if (slateJson.isEmpty())
        return;

    int txType = int (util::FileTransactionType::RECEIVE);
    if (tag==FINALIZE_TAG) {
        txType = int (util::FileTransactionType::FINALIZE);
    }

    // Normal case:
    // res[0] = transactionId
    // res[1] = amount
    QVector<QString> parseResult = util->parseSlateContent(slateJson, txType, "" );
    if (parseResult.size()==1) {
        if (lastReportedError!=3) {
            lastReportedError = 3;
            control::MessageBox::messageText(this, "Wrong Slatepack", "Unable to parse the decoded Slate.\n" + parseResult[0]);
            ui->finalizeSlatepack->setFocus();
        }
        return; // Some parsing error. We don't printing the error message
    }
    Q_ASSERT( parseResult.size()>1 );

    // From init slate we are storing transaction UUID
    if ( tag == INIT_TAG) {
        transactionUUID = parseResult[0];
        return;
    }

    if ( parseResult[0] != transactionUUID ) {
        // It is wrong transaction, let's report it.
        if (lastReportedError!=1) {
            lastReportedError = 1;
            control::MessageBox::messageText(this, "Wrong Slatepack", "Here you can finalize only the Slatepack from this transaction only. This slatepack from different transaction" );
            ui->finalizeSlatepack->setFocus();
        }
        return;
    }

    // we are good here, can finalize...
    slate2finalize = slatepack;
    ui->finalizeSlatepackBtn->setEnabled(true);

    QString textSp = ui->finalizeSlatepack->toPlainText().trimmed();

    if (slatepack != textSp) {
        initiateSlateVerification(textSp, tag);
    }
}


}
