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
#include "../bridge/wnd/g_finalize_b.h"
#include "../util/Json.h" // Just for a contant value
#include <QClipboard>
#include <QJsonDocument>
#include <QToolTip>

namespace wnd {

const QString INIT_TAG = "InitialSP";
const QString FINALIZE_TAG = "FinalizeSP";


ResultedSlatepack::ResultedSlatepack(QWidget *parent, QString slatepack, QString tx_uuid, int backStateId, QString txExtension,
                                     bool enableFinalize) :
        core::NavWnd(parent),
        ui(new Ui::ResultedSlatepack) {
    ui->setupUi(this);
    this->backStateId = backStateId;
    this->txExtension = txExtension;
    this->slatepack = slatepack;
    this->tx_uuid = tx_uuid;
    this->enableFinalize = enableFinalize;

    stateMachine = new bridge::StateMachine(this);
    config = new bridge::Config(this);
    util = new bridge::Util(this);
    wallet = new bridge::Wallet(this);
    finalize = new bridge::Finalize(this);

    ui->qr_code_window->setContent(slatepack);
    ui->slatepackContent->setText(slatepack);

    if (!enableFinalize) {
        // No finalize, hiding the UI
        ui->finalizeSlatepack->hide();
        ui->finalizeSlatepackBtn->hide();
        ui->horizontalSpacer_3->changeSize(1, 1);
    } else {
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
                                             "Slatepack transaction (*" + txExtension + ")", txExtension);

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
    QString sp = ui->finalizeSlatepack->toPlainText().trimmed();
    initiateSlateVerification(sp, FINALIZE_TAG);
}

void ResultedSlatepack::initiateSlateVerification(const QString &slate2check, QString tag) {
    if (slate2check.isEmpty())
        return;

    QJsonObject decodedSlatepackJson = wallet->decodeSlatepack(slate2check);
    wallet::DecodedSlatepack decoded = wallet::DecodedSlatepack::fromJson(decodedSlatepackJson);

    ui->finalizeSlatepackBtn->setEnabled(false);

    if (!decoded.error.isEmpty()) {
        // we don't report error..
        return;
    }

    if (tag == FINALIZE_TAG) {
        if (decoded.content != "SendResponse") {
            if (lastReportedError != 2) {
                lastReportedError = 2;
                control::MessageBox::messageText(this, "Wrong Slatepack",
                                                 "Here you can finalize only the Slatepack from this transaction only. This slatepack doesn't have Response data.");
                ui->finalizeSlatepack->setFocus();
            }
            return;
        }
    }

    if (decoded.slate.isEmpty())
        return;

    int txType = int(util::FileTransactionType::RECEIVE);
    if (tag == FINALIZE_TAG) {
        txType = int(util::FileTransactionType::FINALIZE);
    }

    QString slateJson = QJsonDocument(decoded.slate).toJson(QJsonDocument::Compact);

    // Normal case:
    // res[0] = transactionId
    // res[1] = amount
    QVector<QString> parseResult = util->parseSlateContent(slateJson, txType, "");
    if (parseResult.size() == 1) {
        if (lastReportedError != 3) {
            lastReportedError = 3;
            control::MessageBox::messageText(this, "Wrong Slatepack",
                                             "Unable to parse the decoded Slate.\n" + parseResult[0]);
            ui->finalizeSlatepack->setFocus();
        }
        return; // Some parsing error. We don't printing the error message
    }
    Q_ASSERT(parseResult.size() > 1);

    // From init slate we are storing transaction UUID
    if (tag == INIT_TAG) {
        transactionUUID = parseResult[0];
        return;
    }

    if (parseResult[0] != transactionUUID) {
        // It is wrong transaction, let's report it.
        if (lastReportedError != 1) {
            lastReportedError = 1;
            control::MessageBox::messageText(this, "Wrong Slatepack",
                                             "Here you can finalize only the Slatepack from this transaction only. This slatepack from different transaction");
            ui->finalizeSlatepack->setFocus();
        }
        return;
    }

    // we are good here, can finalize...
    slate2finalize = slate2check;
    ui->finalizeSlatepackBtn->setEnabled(true);

    finalize->uploadSlatepackTransaction( slate2check, slateJson, decoded.sender, false );
}

void ResultedSlatepack::on_finalizeSlatepackBtn_clicked() {
    if (slate2finalize.isEmpty())
        return;

    // disabling to prevent double clicking...
    ui->finalizeSlatepackBtn->setEnabled(false);

    finalize->finalizeSlatepack(slate2finalize, tx_uuid, "",  false, backStateId);
}

void wnd::ResultedSlatepack::on_copySlatepackToClipboardBtn_clicked() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(slatepack);

    QRect btnPos = ui->copySlatepackToClipboardBtn->geometry();
    QPoint tooltipPoint(btnPos.left()-10, btnPos.top()-45);
    tooltipPoint = ui->tab_string->mapToGlobal(tooltipPoint);
    QToolTip::showText( tooltipPoint,  "Slatepack is copied into the clipboard", this);
}

void wnd::ResultedSlatepack::on_copyImageToClipboardBtn_clicked() {
    QImage image = ui->qr_code_window->generateQrImage();
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(image);

    QRect btnPos = ui->copyImageToClipboardBtn->geometry();
    QPoint tooltipPoint(btnPos.left(), btnPos.top()-45);
    tooltipPoint = ui->tab_qrcode->mapToGlobal(tooltipPoint);
    QToolTip::showText( tooltipPoint,  "QR image is copied into the clipboard", this);
}

}