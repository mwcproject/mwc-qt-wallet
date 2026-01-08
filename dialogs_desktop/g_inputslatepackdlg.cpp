// Copyright 2020 The MWC Developers
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

#include "g_inputslatepackdlg.h"

#include <QJsonDocument>

#include "ui_g_inputslatepackdlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"

namespace dlg {

InputSlatepackDlg::InputSlatepackDlg(QString _expectedContent, QString _expectedContentDescription, util::FileTransactionType _txType, QWidget *parent) :
        control::MwcDialog(parent),
        ui(new Ui::InputSlatepackDlg) {
    ui->setupUi(this);
    ui->slatepack_status->hide();
    expectedContent = _expectedContent;
    expectedContentDescription = _expectedContentDescription;
    txType = _txType;

    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);

    updateButtons(false);
}

InputSlatepackDlg::~InputSlatepackDlg() {
    delete ui;
}

void InputSlatepackDlg::on_slatepackEdit_textChanged() {
    QString sp = ui->slatepackEdit->toPlainText().trimmed();
    initiateSlateVerification(sp);
}

void InputSlatepackDlg::on_cancelButton_clicked() {
    reject();
}

void InputSlatepackDlg::on_continueButton_clicked() {
    accept();
}

void InputSlatepackDlg::updateButtons(bool enable) {
    ui->continueButton->setEnabled(enable);
}

void InputSlatepackDlg::initiateSlateVerification(const QString &slate2check) {
    if (slate2check.isEmpty()) {
        ui->slatepack_status->hide();
        return;
    }

    QJsonObject decodedSpJson = wallet->decodeSlatepack(slate2check);
    wallet::DecodedSlatepack decodedSp = wallet::DecodedSlatepack::fromJson(decodedSpJson);

    bool isSpValid = false;
    ui->slatepack_status->hide();
    if (!decodedSp.error.isEmpty()) {
        ui->slatepack_status->show();
        ui->slatepack_status->setText("<b>" + decodedSp.error + "</b>");
    }
    else {
        if (expectedContent != decodedSp.content) {
            ui->slatepack_status->show();
            ui->slatepack_status->setText( "<b>Wrong slatepack content, expected " + expectedContentDescription + "</b>" );
        }
        else {
            // Validating Json
            QString slateJson = QJsonDocument(decodedSp.slate).toJson(QJsonDocument::Compact);
            QVector<QString> slateParseRes = util->parseSlateContent(slateJson, int(txType), decodedSp.sender );
            Q_ASSERT(slateParseRes.size() == 1 || slateParseRes.size() >= 2);
            if (slateParseRes.size() == 1) {
                // parser reported error
                ui->slatepack_status->show();
                ui->slatepack_status->setText("<b>" + slateParseRes[0] + "</b>" );
            }
            else {
                QString spDesk;
                QString senderStr;
                if (decodedSp.sender.isEmpty()) {
                    spDesk = "non encrypted Slatepack";
                } else {
                    spDesk = "encrypted Slatepack";
                    if (txType== util::FileTransactionType::RECEIVE)
                        senderStr = " from " + decodedSp.sender;
                    else if (txType== util::FileTransactionType::FINALIZE)
                        senderStr = ", receiver address " + decodedSp.sender;
                    else {
                        Q_ASSERT(false);
                    }
                }

                // mwc is on nano units
                QString mwcStr = util->nano2one(slateParseRes[1]);

                if (txType== util::FileTransactionType::RECEIVE) {
                    ui->slatepack_status->setText("You receive " + spDesk + " for " + mwcStr + " MWC" + senderStr);
                }
                else if (txType== util::FileTransactionType::FINALIZE) {
                    ui->slatepack_status->setText("Finalizing " + spDesk + ", transaction " + slateParseRes[0] + senderStr);
                }
                isSpValid = true;
                this->slatepack = slate2check;
                this->slateJson = slateJson;
                this->sender = decodedSp.sender;
                ui->slatepack_status->show();
            }
        }
    }

    QString textSp = ui->slatepackEdit->toPlainText().trimmed();

    updateButtons(isSpValid);
}

}
