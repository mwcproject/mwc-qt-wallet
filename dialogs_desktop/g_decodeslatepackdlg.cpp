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

#include "g_decodeslatepackdlg.h"
#include "ui_g_decodeslatepackdlg.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include <QJsonDocument>

namespace dlg {

DecodeSlatepackDlg::DecodeSlatepackDlg(QWidget *parent) :
        control::MwcDialog(parent),
        ui(new Ui::DecodeSlatepackDlg) {
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);

    QObject::connect(wallet, &bridge::Wallet::sgnDecodeSlatepack,
                     this, &dlg::DecodeSlatepackDlg::onSgnDecodeSlatepack, Qt::QueuedConnection);

    resetData();
}

DecodeSlatepackDlg::~DecodeSlatepackDlg() {
    delete ui;
}

void DecodeSlatepackDlg::on_slatepackEdit_textChanged() {
    QString sp = ui->slatepackEdit->toPlainText().trimmed();
    if (spInProgress.isEmpty()) {
        decodeSlate(sp);
    }
}

void DecodeSlatepackDlg::resetData() {
    ui->slateDetailsLabel->setText("");
    ui->slateJsonEdit->setText("");
}

void DecodeSlatepackDlg::decodeSlate(const QString &slatepack) {
    resetData();

    if (slatepack.isEmpty()) {
        return;
    }

    spInProgress = slatepack;
    wallet->decodeSlatepack(slatepack);
}

void DecodeSlatepackDlg::onSgnDecodeSlatepack(QString error, QString slatepack, QString slateJson, QString content, QString sender, QString recipient) {
    spInProgress = "";
    if (!error.isEmpty()) {
        ui->slateDetailsLabel->setText("<b>" + error + "</b>");
    }
    else {
        // Let's dp prerry pring for json
        QJsonDocument doc = QJsonDocument::fromJson(slateJson.toUtf8());
        slateJson = doc.toJson(QJsonDocument::Indented);

        ui->slateJsonEdit->setText(slateJson);
        ui->slateDetailsLabel->setText("Content:  " + content +
                "\nSender: " + (sender == "None"?"Non encrypted":sender) +
                "\nRecipient: " + (recipient == "None"?"Non encrypted":recipient));
    }

    QString sp = ui->slatepackEdit->toPlainText().trimmed();

    if (slatepack != sp) {
        decodeSlate(sp);
    }
}

void DecodeSlatepackDlg::on_closeButton_clicked()
{
    reject();
}

}
