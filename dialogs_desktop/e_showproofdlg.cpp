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
#include "../util/execute.h"
#include "../core/global.h"

namespace dlg {

// Parse the data and fill the structure.
bool ProofInfo::parseProofText(const QString & proof) {

    // this file proves that [0.100000000] MWCs was sent to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU] from [xmiuyC3sdhXpJnR7pvQ8xNgZLWQRQziZ1FxhEQd8urYWvSusuC69]
    int idx0 = proof.indexOf("this file proves that");
    if (idx0<0)
        return false;

    int mwcIdx1 = proof.indexOf('[', idx0);
    int mwcIdx2 = proof.indexOf(']', mwcIdx1);

    int addrToIdx1 = proof.indexOf('[', mwcIdx2);
    int addrToIdx2 = proof.indexOf(']', addrToIdx1);
    int addrFromIdx1 = proof.indexOf('[', addrToIdx2);
    int addrFromIdx2 = proof.indexOf(']', addrFromIdx1);

    int outputIdx0 = proof.indexOf( "outputs:", addrFromIdx2);
    int outputIdx1 = proof.indexOf( ':', outputIdx0);
    int outputIdx2 = proof.indexOf( ':', outputIdx1 + 1);

    int kernelIdx0 = proof.indexOf( "kernel:", outputIdx2);
    int kernelIdx1 = proof.indexOf( ':', kernelIdx0);
    int kernelIdx2 = proof.indexOf( ':', kernelIdx1 + 1);

    if ( idx0<0 || mwcIdx1<0 || mwcIdx2<0 || addrToIdx1<0 || addrToIdx2<0 || addrFromIdx1<0 || addrFromIdx2<0 ||
                 outputIdx0<0 || outputIdx1<0 || outputIdx2<0 ||
                 kernelIdx0<0 || kernelIdx1<0 || kernelIdx2<0 )
        return false;

    mwc = proof.mid(mwcIdx1+1, mwcIdx2-mwcIdx1-1 );
    while (mwc.size()>0 && mwc[mwc.size()-1]=='0')
        mwc.resize(mwc.size()-1);

    if (mwc.size()>0 && mwc[mwc.size()-1]=='.')
        mwc.resize(mwc.size()-1);

    toAdress = proof.mid( addrToIdx1+1, addrToIdx2-addrToIdx1-1 );
    fromAdress = proof.mid( addrFromIdx1+1, addrFromIdx2-addrFromIdx1-1 );

    output = proof.mid( outputIdx1+1, outputIdx2-outputIdx1-1).trimmed();
    kernel = proof.mid( kernelIdx1+1, kernelIdx2-kernelIdx1-1).trimmed();

    if ( mwc.isEmpty() || toAdress.isEmpty() || fromAdress.isEmpty() || output.isEmpty() || kernel.isEmpty() )
        return false;

    return true;
}


ShowProofDlg::ShowProofDlg(QWidget *parent, const QString &fileName, const ProofInfo & proofInfo, const wallet::WalletConfig & config ) :
    control::MwcDialog(parent),
        ui(new Ui::ShowProofDlg),
        proof(proofInfo)
{
    ui->setupUi(this);

    ui->proofLocation->setText(fileName);

    ui->textEdit->setText("<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
            "p, li { white-space: pre-wrap; }"
            "</style></head><body style=\"line-height:130%;margin-left:20px;margin-right:20px;margin-top:15px\">"
            "<p align=\"center\">this file proves that <b>" + proofInfo.mwc + " MWCs</b> was sent to<br /><b>" + proofInfo.toAdress +
            "</b> <br/>from<br/><b>" + proofInfo.fromAdress + "</br></p></body></html>");

    ui->output->setText( proofInfo.output );
    ui->kernel->setText( proofInfo.kernel );


    blockExplorerUrl = (config.getNetwork() == "Mainnet") ? mwc::BLOCK_EXPLORER_URL_MAINNET : mwc::BLOCK_EXPLORER_URL_FLOONET;
}

ShowProofDlg::~ShowProofDlg() {
    delete ui;
}

void ShowProofDlg::on_viewOutput_clicked()
{
    util::openUrlInBrowser( "https://"+blockExplorerUrl+"/#o" + proof.output );
}

void ShowProofDlg::on_viewKernel_clicked()
{
    util::openUrlInBrowser( "https://"+blockExplorerUrl+"/#k" + proof.kernel );
}

void ShowProofDlg::on_pushButton_clicked()
{
    accept();
}

}


