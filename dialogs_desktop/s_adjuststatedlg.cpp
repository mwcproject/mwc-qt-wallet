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

#include "s_adjuststatedlg.h"
#include "ui_s_adjuststatedlg.h"
#include "../bridge/swap_b.h"

namespace dlg {

AdjustStateDlg::AdjustStateDlg(QWidget * parent, QString _tradeId) : control::MwcDialog(parent),
    ui(new Ui::AdjustStateDlg),
    tradeId(_tradeId)
{
    ui->setupUi(this);

    swap = new bridge::Swap(this);

    ui->title->setText("Adjust " + tradeId);

    ui->statesComboBox->addItem("SellerOfferCreated", QVariant("SellerOfferCreated"));
    ui->statesComboBox->addItem("SellerSendingOffer", QVariant("SellerSendingOffer"));
    ui->statesComboBox->addItem("SellerWaitingForAcceptanceMessage", QVariant("SellerWaitingForAcceptanceMessage"));
    ui->statesComboBox->addItem("SellerWaitingForBuyerLock", QVariant("SellerWaitingForBuyerLock"));
    ui->statesComboBox->addItem("SellerPostingLockMwcSlate", QVariant("SellerPostingLockMwcSlate"));
    ui->statesComboBox->addItem("SellerWaitingForLockConfirmations", QVariant("SellerWaitingForLockConfirmations" ));
    ui->statesComboBox->addItem("SellerWaitingForInitRedeemMessage", QVariant("SellerWaitingForInitRedeemMessage"));
    ui->statesComboBox->addItem("SellerSendingInitRedeemMessage", QVariant("SellerSendingInitRedeemMessage"));
    ui->statesComboBox->addItem("SellerWaitingForBuyerToRedeemMwc", QVariant("SellerWaitingForBuyerToRedeemMwc"));
    ui->statesComboBox->addItem("SellerRedeemSecondaryCurrency", QVariant("SellerRedeemSecondaryCurrency"));
    ui->statesComboBox->addItem("SellerWaitingForRedeemConfirmations", QVariant("SellerWaitingForRedeemConfirmations"));
    ui->statesComboBox->addItem("SellerSwapComplete", QVariant("SellerSwapComplete"));
    ui->statesComboBox->addItem("SellerWaitingForRefundHeight", QVariant("SellerWaitingForRefundHeight"));
    ui->statesComboBox->addItem("SellerPostingRefundSlate", QVariant("SellerPostingRefundSlate"));
    ui->statesComboBox->addItem("SellerWaitingForRefundConfirmations", QVariant("SellerWaitingForRefundConfirmations"));
    ui->statesComboBox->addItem("SellerCancelledRefunded", QVariant("SellerCancelledRefunded"));
    ui->statesComboBox->addItem("SellerCancelled", QVariant("SellerCancelled"));
    ui->statesComboBox->addItem("BuyerOfferCreated", QVariant("BuyerOfferCreated"));
    ui->statesComboBox->addItem("BuyerSendingAcceptOfferMessage", QVariant("BuyerSendingAcceptOfferMessage"));
    ui->statesComboBox->addItem("BuyerWaitingForSellerToLock", QVariant("BuyerWaitingForSellerToLock"));
    ui->statesComboBox->addItem("BuyerPostingSecondaryToMultisigAccount", QVariant("BuyerPostingSecondaryToMultisigAccount"));
    ui->statesComboBox->addItem("BuyerWaitingForLockConfirmations", QVariant("BuyerWaitingForLockConfirmations"));
    ui->statesComboBox->addItem("BuyerSendingInitRedeemMessage", QVariant("BuyerSendingInitRedeemMessage"));
    ui->statesComboBox->addItem("BuyerWaitingForRespondRedeemMessage", QVariant("BuyerWaitingForRespondRedeemMessage"));
    ui->statesComboBox->addItem("BuyerRedeemMwc", QVariant("BuyerRedeemMwc"));
    ui->statesComboBox->addItem("BuyerWaitForRedeemMwcConfirmations", QVariant("BuyerWaitForRedeemMwcConfirmations"));
    ui->statesComboBox->addItem("BuyerSwapComplete", QVariant("BuyerSwapComplete"));
    ui->statesComboBox->addItem("BuyerWaitingForRefundTime", QVariant("BuyerWaitingForRefundTime"));
    ui->statesComboBox->addItem("BuyerPostingRefundForSecondary", QVariant("BuyerPostingRefundForSecondary"));
    ui->statesComboBox->addItem("BuyerWaitingForRefundConfirmations", QVariant("BuyerWaitingForRefundConfirmations"));
    ui->statesComboBox->addItem("BuyerCancelledRefunded", QVariant("BuyerCancelledRefunded"));
    ui->statesComboBox->addItem("BuyerCancelled", QVariant("BuyerCancelled"));
}

AdjustStateDlg::~AdjustStateDlg() {
    delete ui;
}

void AdjustStateDlg::on_applyButton_clicked() {
    if (ui->statesComboBox->currentIndex()<0)
        return;

    QString selectedState = ui->statesComboBox->currentData().toString();
    if (selectedState.isEmpty())
        return;

    swap->adjustTradeState( tradeId, selectedState );

    accept();
}

void AdjustStateDlg::on_cancelButton_clicked() {
    reject();
}

}