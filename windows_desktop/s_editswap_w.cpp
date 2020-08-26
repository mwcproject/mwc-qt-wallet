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

#include "s_editswap_w.h"
#include "ui_s_editswap_w.h"
#include "../bridge/swap_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

EditSwap::EditSwap(QWidget *parent, QString _swapId) :
        core::NavWnd(parent),
        ui(new Ui::EditSwap),
        swapId(_swapId) {
    ui->setupUi(this);

    swap = new bridge::Swap(this);
    connect(swap, &bridge::Swap::sgnRequestTradeDetails, this, &EditSwap::sgnRequestTradeDetails, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateCommunication, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateSecondaryAddress, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateSecondaryFee, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnCancelTrade, this, &EditSwap::sgnCancelTrade, Qt::QueuedConnection);

    ui->swapIdLabel->setText("SwapId: " + swapId);

    ui->progress->initLoader(true);
    swap->requestTradeDetails(swapId);
    updateButtons(true);
}

EditSwap::~EditSwap() {
    delete ui;
}

void EditSwap::sgnRequestTradeDetails( QVector<QString> swapInfo,
                                       QVector<QString> executionPlan,
                                       QString currentAction,
                                       QVector<QString> tradeJournal,
                                       QString errMsg )
{
    Q_UNUSED(currentAction)
    Q_UNUSED(executionPlan)
    Q_UNUSED(tradeJournal)

    Q_ASSERT(swapInfo.size()>=1);
    QString reqSwapId = swapInfo[0];

    if (reqSwapId != swapId)
        return;

    ui->progress->hide();

    if (!errMsg.isEmpty()) {
        Q_ASSERT(swapInfo.size()>=1);
        control::MessageBox::messageText( this, "Swap Trade details", "Unable to get a details about the trade " + swapInfo[0] +
                    "\n\n" + errMsg );
        return;
    }

    Q_ASSERT(swapInfo.size() == 8);
    if (swapInfo.size() != 8 || swapInfo[0] != swapId)
        return; // Invalid message or invalid destination

    QString tradeDescription = swapInfo[1];
    seller = tradeDescription.startsWith("Selling");
    ui->tradeDesriptionLabel->setText(tradeDescription);
    redeemAddress = swapInfo[2];
    secondaryCurrency = swapInfo[3];
    secondaryFee = swapInfo[4];
    ui->secondaryFeeUnitsLabel->setText(swapInfo[5]);
    communicationMethod = swapInfo[6];
    communicaitonAddress = swapInfo[7];

    ui->redeemAddressEdit->setText(redeemAddress);
    ui->secondaryFeeLabel->setText(secondaryCurrency + " fee:");
    ui->secondaryFeeEdit->setText(secondaryFee);

    if (seller) {
        ui->addressBox->setTitle("Buyer Address");
        ui->secondaryAddressLabel->setText("Redeem address");
    } else {
        ui->addressBox->setTitle("Seller Address");
        ui->secondaryAddressLabel->setText("Refund address");
    }

    ui->mwcmqsRadio->setChecked(communicationMethod == "mwcmqs");
    ui->torRadio->setChecked(communicationMethod == "tor");
    ui->destinationEdit->setText(communicaitonAddress);

    updateButtons();
}

void EditSwap::sgnUpdateXXX(QString swId, QString errorMsg) {
    if (swapId != swId)
        return;

    if (!errorMsg.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Unable to update data for swap " + swapId + "\n\n" + errorMsg);
    }
}

QString EditSwap::getCommunicationMethod() const {
    if (ui->mwcmqsRadio->isChecked())
        return "mwcmqs";
    if (ui->torRadio->isChecked())
        return "tor";

    Q_ASSERT(false);
    return "";
}


bool EditSwap::isCanUpdate() const {
    return !(redeemAddress == ui->redeemAddressEdit->text() &&
            secondaryFee == ui->secondaryFeeEdit->text() &&
            communicationMethod == getCommunicationMethod() &&
            communicaitonAddress == ui->destinationEdit->text());
}

void EditSwap::updateButtons(bool first_call) {
    bool canUpdate = false;
    if (!first_call)
        canUpdate = isCanUpdate();
    ui->updateBtn->setEnabled(canUpdate);
    ui->tradeDetailsBtn->setEnabled(!canUpdate);
    QString secondaryAddress = ui->redeemAddressEdit->text();
    bool isTradeRunning = swap->isRunning(swapId);
    ui->startButton->setEnabled(!isTradeRunning && !canUpdate && !secondaryAddress.isEmpty());
    ui->stopButton->setEnabled(isTradeRunning);
}


void EditSwap::on_redeemAddressEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_secondaryFeeEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_mwcmqsRadio_toggled(bool checked) {
    Q_UNUSED(checked)
    updateButtons();
}

void EditSwap::on_torRadio_toggled(bool checked) {
    Q_UNUSED(checked)
    updateButtons();
}

void EditSwap::on_destinationEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_updateBtn_clicked() {
    // Check what we need update...
    if (ui->destinationEdit->text().isEmpty()) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") +
                (seller ? "Buyer" : "Seller") + " address");
        ui->destinationEdit->setFocus();
        return;
    }

    if ( ui->redeemAddressEdit->text().isEmpty() ) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") + secondaryCurrency + " " +
                (seller ? "redeem" : "refund") +  " address" );
        return;
    }

    QString fee = ui->secondaryFeeEdit->text();
    bool ok = false;
    double feeDbl = fee.toDouble(&ok);
    if ( fee.isEmpty() || !ok ) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") + secondaryCurrency + " fee value" );
        return;
    }

    ui->progress->show();
    if (!(communicationMethod == getCommunicationMethod() && communicaitonAddress == ui->destinationEdit->text())) {
        swap->updateCommunication(swapId, getCommunicationMethod(), ui->destinationEdit->text());
    }

    if ( redeemAddress != ui->redeemAddressEdit->text() ) {
        swap->updateSecondaryAddress(swapId, ui->redeemAddressEdit->text());
    }
    if (secondaryFee != ui->secondaryFeeEdit->text())
    {
        swap->updateSecondaryFee(swapId, feeDbl);
    }
    swap->requestTradeDetails(swapId);
    updateButtons();
}

void EditSwap::on_tradeDetailsBtn_clicked() {
    if (isCanUpdate()) {
        if ( core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Warning", "You have not saved data. Do you want to drop the data and switch to the details?",
                                          "No", "Yes",
                                          "Stay in this page", "Drop unsaved data and switch",
                                          true, false))
            return;
    }
    swap->showTradeDetails(swapId);
}

void EditSwap::on_startButton_clicked() {
    swap->startAutoSwapTrade(swapId);
    updateButtons();
}

void EditSwap::on_stopButton_clicked() {
    swap->stopAutoSwapTrade(swapId);
    updateButtons();
}

void EditSwap::on_backButton_clicked() {
    swap->pageTradeList();
}

void EditSwap::on_cancelButton_clicked()
{
    if ( core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Warning", "Are you sure you want to cancel this trade? Please note, that refund process might take time and you wallet need to be online to do that. If your wallet will not be online until the swap process will be finished, you might lost the funds.",
                                                                                  "No", "Yes",
                                                                                  "Continue this swap trade", "Cancel the trade and get a refund if needed",
                                                                                  true, false))
        return;

    ui->progress->show();
    swap->cancelTrade(swapId);
}

void EditSwap::sgnCancelTrade(QString swId, QString error) {
    if (swapId != swId)
        return;

    ui->progress->hide();

    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Trade cancellation", "Unable to cancel the swap " + swapId + "\n\n" + error);
        return;
    }
    // Go back on cancel
    on_backButton_clicked();
}

}
