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

#include "g_finalize_w.h"
#include "ui_g_finalize.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/g_finalize_b.h"
#include "../core/global.h"
#include "../dialogs_desktop/g_inputslatepackdlg.h"
#include "../bridge/wallet_b.h"

namespace wnd {

Finalize::Finalize(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::FinalizeUpload)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    finalize = new bridge::Finalize(this);
    util = new bridge::Util(this);
    wallet = new bridge::Wallet(this);

    QObject::connect(wallet, &bridge::Wallet::sgnDecodeSlatepack,
                     this, &wnd::Finalize::onSgnDecodeSlatepack, Qt::QueuedConnection);

    ui->slatepack_status->setText("");

    updateButtons();
}

Finalize::~Finalize()
{
    delete ui;
}

/*
void Finalize::on_pasteSlatepackBtn_clicked()
{
    util::TimeoutLockObject to( "FinalizeUpload" );

    dlg::InputSlatepackDlg inputSlateDlg("SendResponse", "Send response slate",
                                         util::FileTransactionType::FINALIZE,  this);

    if (inputSlateDlg.exec()  == QDialog::Accepted ) {
        finalize->uploadSlatepackTransaction( inputSlateDlg.getSlatepack(), inputSlateDlg.getSlateJson(), inputSlateDlg.getSenderAddress() );
    }
}*/

void Finalize::on_slatepackEdit_textChanged() {
    isSpValid = false;
    updateButtons();

    QString sp = ui->slatepackEdit->toPlainText().trimmed();
    initiateSlateVerification(sp);
}

void Finalize::on_continueButton_clicked() {
    finalize->uploadSlatepackTransaction( slatepack, slateJson, sender);
}

void Finalize::updateButtons() {
    ui->continueButton->setEnabled(isSpValid);
}

void Finalize::initiateSlateVerification(const QString &slate2check) {
    if (slate2check.isEmpty()) {
        ui->slatepack_status->setText("");
        return;
    }

    bool notInProgress = spInProgress.isEmpty();
    spInProgress = slate2check;
    if (notInProgress) {
        wallet->decodeSlatepack(slate2check, "Finalize");
    }
}


void Finalize::onSgnDecodeSlatepack(QString tag, QString error, QString slatepack, QString slateJson, QString content, QString sender, QString recipient) {
    Q_UNUSED(recipient)

    if (tag != "Finalize")
        return;

    if (spInProgress!=slatepack) {
        // data was chnaged - need to reprocess
        wallet->decodeSlatepack(spInProgress, "Finalize");
        return;
    }

    isSpValid = false;
    spInProgress = "";
    ui->slatepack_status->setText("");
    if (!error.isEmpty()) {
        ui->slatepack_status->setText("<b>" + error + "</b>");
    }
    else {
        if ("SendResponse" != content) {
            ui->slatepack_status->setText( "<b>Wrong slatepack content, expected SendResponse slatepack, get "+content+"</b>" );
        }
        else {
            // Validating Json
            QVector<QString> slateParseRes = util->parseSlateContent(slateJson, int(util::FileTransactionType::FINALIZE), sender );
            Q_ASSERT(slateParseRes.size() == 1 || slateParseRes.size() >= 2);
            if (slateParseRes.size() == 1) {
                // parser reported error
                ui->slatepack_status->setText("<b>" + slateParseRes[0] + "</b>" );
            }
            else {
                QString spDesk;
                QString senderStr;
                if (sender == "None") {
                    spDesk = "non encrypted Slatepack";
                } else {
                    spDesk = "encrypted Slatepack";
                    senderStr = ", receiver address " + sender;
                }

                // mwc is on nano units
                QString mwcStr = util->nano2one(slateParseRes[1]);

                ui->slatepack_status->setText("Finalizing " + spDesk + ", transaction " + slateParseRes[0] + senderStr);
                isSpValid = true;
                this->slatepack = slatepack;
                this->slateJson = slateJson;
                this->sender = sender;
            }
        }
    }

    QString textSp = ui->slatepackEdit->toPlainText().trimmed();

    if (slatepack != textSp) {
        initiateSlateVerification(textSp);
        isSpValid = false;
    }

    updateButtons();
}


}

