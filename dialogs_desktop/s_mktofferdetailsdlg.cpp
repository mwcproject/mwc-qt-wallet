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

#include "s_mktofferdetailsdlg.h"
#include "ui_s_mktofferdetailsdlg.h"
#include "../state/s_mktswap.h"
#include "../bridge/wnd/swap_b.h"

namespace dlg {

MktOfferDetailsDlg::MktOfferDetailsDlg(QWidget *parent, const state::MktSwapOffer & offer) :
        control::MwcDialog(parent),
        ui(new Ui::MktOfferDetailsDlg) {
    ui->setupUi(this);

    QString infoStr;
    if (offer.sell)
        infoStr += "Selling ";
    else
        infoStr += "Buying ";

    infoStr += QString::number( offer.mwcLockBlocks ) + " MWC for " + QString::number(offer.secLockBlocks) + " " + offer.secondaryCurrency + "\n\n";
    infoStr += "Confirmations:    " + QString::number(offer.mwcLockBlocks) + " blocks for MWC, " +
                QString::number(offer.secLockBlocks) + " blocks for " + offer.secondaryCurrency + "\n\n";
    infoStr += "Lock Time:          " + offer.calcMwcLockTime() + " for MWC, " + offer.calcSecLockTime() + " for " + offer.secondaryCurrency + "\n\n";
    infoStr += "From:                  " + offer.walletAddress;

    ui->offerInfoLabel->setText(infoStr);

    // Check confirmations numbers if it is below calculated value

    bridge::Swap * swap = new bridge::Swap(this);

    int mwcConf = swap->getMwcConfNumber(offer.mwcAmount);
    int secConf = swap->getSecConfNumber(offer.secondaryCurrency);

    QString warningStr;
    if (offer.mwcLockBlocks < mwcConf) {
        warningStr += "MWC confirmation number " + QString::number(offer.mwcLockBlocks) + " is less than recommended value " + QString::number(mwcConf) + ".\n";
    }
    if (offer.secLockBlocks < secConf) {
        warningStr += offer.secondaryCurrency + " confirmation number " + QString::number(offer.secLockBlocks) + " is less than recommended value " + QString::number(secConf) + ".\n";
    }
    if (!warningStr.isEmpty()) {
        warningStr = warningStr + "Your trade can be vulnerable for reorg attack";
    }
    ui->offerWarningLabel->setText(warningStr);

    if (warningStr.isEmpty())
        ui->offerWarningLabel->hide();
}

MktOfferDetailsDlg::~MktOfferDetailsDlg() {
    delete ui;
}

void MktOfferDetailsDlg::on_backButton_clicked() {
    reject();
}

void MktOfferDetailsDlg::on_acceptButton_clicked() {
    accept();
}

}