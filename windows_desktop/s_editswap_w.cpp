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
    connect(swap, &bridge::Swap::sgnUpdateSecondaryAddress, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateSecondaryFee, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);

    setPageTitle("Trade: " + swapId);

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

    QString secCurrency = swapInfo[3];
    QString secCurrencyFeeUnits = swapInfo[5]; // - secondary fee units

    ui->secondaryAddressLabel->setText( secCurrency + " address to receive coins");
    ui->secTransFeeLabel->setText(secCurrency + " transaction fee");
    ui->updateBtn->setText("Update "+ secCurrency +" transaction details");
    ui->secFeeUnitsLabel->setText(secCurrencyFeeUnits);

    QString tradeDescription = swapInfo[1];
    ui->tradeDesriptionLabel->setText(tradeDescription);
    redeemAddress = swapInfo[2];
    secondaryCurrency = swapInfo[3];
    secondaryFee = swapInfo[4];

    ui->redeemAddressEdit->setText(redeemAddress);
    ui->secondaryFeeEdit->setText(secondaryFee);

    updateButtons();
}

void EditSwap::sgnUpdateXXX(QString swId, QString errorMsg) {
    if (swapId != swId)
        return;

    if (!errorMsg.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Unable to update data for swap " + swapId + "\n\n" + errorMsg);
        ui->progress->hide();
    }
    else {
        // OK, let's refresh...
        swap->requestTradeDetails(swapId);
    }

    updateButtons();
}

bool EditSwap::isCanUpdate() const {
    return !(redeemAddress == ui->redeemAddressEdit->text() &&
            secondaryFee == ui->secondaryFeeEdit->text() );
}

void EditSwap::updateButtons(bool first_call) {
    bool canUpdate = false;
    if (!first_call)
        canUpdate = isCanUpdate();
    ui->updateBtn->setEnabled(canUpdate);
    ui->tradeDetailsBtn->setEnabled(!canUpdate);
    QString secondaryAddress = ui->redeemAddressEdit->text();
}


void EditSwap::on_redeemAddressEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_secondaryFeeEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_updateBtn_clicked() {
    // Check what we need update...

    if ( ui->redeemAddressEdit->text().isEmpty() ) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") + secondaryCurrency +
                 " address to receive the coins." );
        return;
    }

    QString fee = ui->secondaryFeeEdit->text();
    bool ok = false;
    double feeDbl = fee.toDouble(&ok);
    if ( fee.isEmpty() || !ok || feeDbl<=0.0 ) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") + secondaryCurrency + " fee value" );
        return;
    }

    ui->progress->show();

    if ( redeemAddress != ui->redeemAddressEdit->text() ) {
        swap->updateSecondaryAddress(swapId, ui->redeemAddressEdit->text());
    }
    if (secondaryFee != ui->secondaryFeeEdit->text())
    {
        swap->updateSecondaryFee(swapId, feeDbl);
    }
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

void EditSwap::on_backButton_clicked() {
    if (isCanUpdate()) {
        if ( core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Warning", "You have not saved data. Do you want to drop the data and return back?",
                                                                                      "No", "Yes",
                                                                                      "Stay in this page", "Drop unsaved data and switch",
                                                                                      true, false))
            return;
    }
    swap->pageTradeList();
}

}
