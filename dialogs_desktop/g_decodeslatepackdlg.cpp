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

#include "util/message_mapper.h"

namespace dlg {

DecodeSlatepackDlg::DecodeSlatepackDlg(QWidget *parent) :
        control::MwcDialog(parent),
        ui(new Ui::DecodeSlatepackDlg) {
    ui->setupUi(this);

    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);

    resetData();
}

DecodeSlatepackDlg::~DecodeSlatepackDlg() {
    delete ui;
}

void DecodeSlatepackDlg::on_slatepackEdit_textChanged() {
    QString sp = ui->slatepackEdit->toPlainText().trimmed();
    decodeSlate(sp);
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

    QPair <bool, QString> res = util::validateMwc713Str(slatepack, true);
    if (!res.first) {
        ui->slateDetailsLabel->setText(res.second);
        return;
    }

    QJsonObject decodedSpJson = wallet->decodeSlatepack(slatepack);
    wallet::DecodedSlatepack decodedSp = wallet::DecodedSlatepack::fromJson(decodedSpJson);

    if (!decodedSp.error.isEmpty()) {
        ui->slateDetailsLabel->setText("<b>" + decodedSp.error + "</b>");
    }
    else {
        // Let's dp prerry pring for json
        QJsonDocument doc = QJsonDocument(decodedSp.slate);
        QString slateJson = doc.toJson(QJsonDocument::Indented);

        ui->slateJsonEdit->setText(slateJson);
        ui->slateDetailsLabel->setText("Content:  " + decodedSp.content +
                "\nSender: " + (decodedSp.sender == "None"? "Non encrypted":decodedSp.sender) +
                "\nRecipient: " + (decodedSp.recipient == "None"?"Non encrypted":decodedSp.recipient));
    }
}

void DecodeSlatepackDlg::on_closeButton_clicked()
{
    reject();
}

}
