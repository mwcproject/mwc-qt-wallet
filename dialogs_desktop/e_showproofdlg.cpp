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

#include "e_showproofdlg.h"
#include "ui_e_showproofdlg.h"
#include "../bridge/config_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wallet_b.h"

namespace dlg {

// Parse the data and fill the structure.
bool ProofInfo::parseProofText(const QString & proof_str) {

    // this file proves that [0.100000000] MWCs was sent to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU] from [xmiuyC3sdhXpJnR7pvQ8xNgZLWQRQziZ1FxhEQd8urYWvSusuC69]
    int idx0 = proof_str.indexOf("this file proves that");
    if (idx0<0)
        return false;

    QString streamText = proof_str;
    QTextStream stream(&streamText);

    QString ln = stream.readLine();

    int mwcIdx1 = ln.indexOf('[', idx0);
    int mwcIdx2 = ln.indexOf(']', mwcIdx1);
    int addrToIdx1 = ln.indexOf('[', mwcIdx2);
    int addrToIdx2 = ln.indexOf(']', addrToIdx1);
    int addrFromIdx1 = ln.indexOf('[', addrToIdx2);
    int addrFromIdx2 = ln.indexOf(']', addrFromIdx1);

    if ( idx0<0 || mwcIdx1<0 || mwcIdx2<0 || addrToIdx1<0 || addrToIdx2<0 || addrFromIdx1<0 || addrFromIdx2<0)
        return false;

    mwc = ln.mid(mwcIdx1+1, mwcIdx2-mwcIdx1-1 );
    while (mwc.size()>0 && mwc[mwc.size()-1]=='0')
        mwc.resize(mwc.size()-1);

    if (mwc.size()>0 && mwc[mwc.size()-1]=='.')
        mwc.resize(mwc.size()-1);

    toAdress = ln.mid( addrToIdx1+1, addrToIdx2-addrToIdx1-1 );
    fromAdress = ln.mid( addrFromIdx1+1, addrFromIdx2-addrFromIdx1-1 );

    while (!stream.atEnd()) {
        ln = stream.readLine();
        if (ln.startsWith("outputs:"))
            break;
    }

    while (!stream.atEnd()) {
        ln = stream.readLine();
        if (ln.startsWith("kernel:"))
            break;

        int outputIdx1 = ln.indexOf( ':');
        if (outputIdx1>0) {
            QString output = ln.left(outputIdx1).trimmed();
            outputs.push_back(output);
        }
    }

    ln = stream.readLine();
    int kernelIdx = ln.indexOf( ':');
    if (kernelIdx<0)
        return false;

    kernel = ln.left(kernelIdx).trimmed();

    ln = stream.readLine();
    if (!ln.startsWith("slate: "))
        return false;

    slate = ln.right( ln.length() - strlen("slate: ") ).trimmed();
    return true;
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


