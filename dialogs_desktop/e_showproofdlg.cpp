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

#include <QJsonDocument>
#include "e_showproofdlg.h"
#include "ui_e_showproofdlg.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wallet_b.h"
#include "../util/stringutils.h"

namespace dlg {

// Parse the data and fill the structure.
void ProofInfo::parseProofRes(const QString & proof_str) {
    // Parse JSON data from Rust VerifyProofResult struct using Qt's built-in JSON parsing
    QJsonObject json = QJsonDocument::fromJson(proof_str.toUtf8()).object();

    fromAdress = json["sender_address"].toString();
    toAdress = json["reciever_address"].toString();
    mwc = util::nano2one(qint64(json["amount"].toInteger()));
    // Extract outputs array
    QJsonArray outputsArray = json["outputs"].toArray();
    for (const QJsonValue &outputValue : outputsArray) {
        outputs.push_back(outputValue.toString());
    }
    kernel = json["kernel"].toString();
    slate = json["slate"].toString();
}


ShowProofDlg::ShowProofDlg(QWidget *parent, const QString &fileName, const ProofInfo & proofInfo ) :
    control::MwcDialog(parent),
        ui(new Ui::ShowProofDlg),
        proof(proofInfo)
{
    ui->setupUi(this);

    config = new bridge::Config(this);
    util = new bridge::Util(this);
    wallet = new bridge::Wallet(this);

    ui->proofLocation->setText(fileName);

    ui->textEdit->setText("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\"font-size:16px;line-height:130%;margin-left:20px;margin-right:20px;margin-top:15px\">"
            "<p align=\"center\">this file proves that <b>" + proofInfo.mwc + " MWCs</b> was sent to<br /><b>" + proofInfo.toAdress +
            "</b> <br/>from<br/><b>" + proofInfo.fromAdress + "</br></p></body></html>");

    for (const QString & output : proofInfo.outputs ) {
        ui->outputs->addItem(output);
    }
    ui->outputs->setCurrentIndex(0);

    ui->kernel->setText( proofInfo.kernel );
    ui->slate->setText(proofInfo.slate);

    blockExplorerUrl = config->getBlockExplorerUrl(config->getNetwork());
}

ShowProofDlg::~ShowProofDlg() {
    delete ui;
}

void ShowProofDlg::on_viewOutput_clicked()
{
    int outIdx = ui->outputs->currentIndex();
    if (outIdx>=0 && outIdx<proof.outputs.size()) {
        util->openUrlInBrowser( "https://"+blockExplorerUrl+"/#o" + proof.outputs[outIdx] );
    }
}

void ShowProofDlg::on_viewKernel_clicked()
{
    util->openUrlInBrowser( "https://"+blockExplorerUrl+"/#k" + proof.kernel );
}

void ShowProofDlg::on_pushButton_clicked()
{
    accept();
}

}


