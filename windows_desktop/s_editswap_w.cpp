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
#include "../bridge/config_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

EditSwap::EditSwap(QWidget *parent, QString _swapId, QString _stateCmd) :
        core::NavWnd(parent),
        ui(new Ui::EditSwap),
        swapId(_swapId) {
    ui->setupUi(this);

    swap = new bridge::Swap(this);
    config = new bridge::Config(this);

    connect(swap, &bridge::Swap::sgnRequestTradeDetails, this, &EditSwap::sgnRequestTradeDetails, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateSecondaryAddress, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateSecondaryFee, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnUpdateElectrumX, this, &EditSwap::sgnUpdateXXX, Qt::QueuedConnection);

    setPageTitle("Trade: " + swapId);

    // Check if we need to accept the deal.
    acceptanceMode = !config->isTradeAccepted(swapId) && swap->isSwapWatingToAccept(_stateCmd);

    if (acceptanceMode) {
        ui->updateBtn->hide();
        ui->tradeDetailsBtn->setText("Accept");
    }

    ui->secondaryAddressLabel->setText("");

    ui->progress->initLoader(true);
    swap->requestTradeDetails(swapId);

    ui->noteEdit->setText(config->getSwapNote(swapId));

    updateButtons(true);
}

EditSwap::~EditSwap() {
    config->updateSwapNote(swapId, ui->noteEdit->text());
    delete ui;
}

void EditSwap::sgnRequestTradeDetails(QVector<QString> swapInfo,
                                      QVector<QString> executionPlan,
                                      QString currentAction,
                                      QVector<QString> tradeJournal,
                                      QString errMsg) {
    Q_UNUSED(currentAction)
    Q_UNUSED(executionPlan)
    Q_UNUSED(tradeJournal)

    Q_ASSERT(swapInfo.size() >= 1);
    QString reqSwapId = swapInfo[0];

    if (reqSwapId != swapId)
        return;

    ui->progress->hide();

    if (!errMsg.isEmpty()) {
        Q_ASSERT(swapInfo.size() >= 1);
        control::MessageBox::messageText(this, "Swap Trade details",
                                         "Unable to get a details about the trade " + swapInfo[0] +
                                         "\n\n" + errMsg);
        return;
    }

    Q_ASSERT(swapInfo.size() == 9);
    if (swapInfo.size() != 9 || swapInfo[0] != swapId)
        return; // Invalid message or invalid destination

    QString secCurrency = swapInfo[3];
    QString secCurrencyFeeUnits = swapInfo[5]; // - secondary fee units

    // [1] - Description in HTML format. Role can be calculated form here as "Selling ..." or "Buying ..."
    int sellIdx = swapInfo[1].indexOf("Selling"); // Text in HTML formal, "Start With" will not work
    bool seller = sellIdx > 200 && sellIdx < 300;

    if (seller)
        ui->secondaryAddressLabel->setText(secCurrency + " address to receive the coins");
    else
        ui->secondaryAddressLabel->setText(secCurrency + " address for the refund in case of trade cancellation");

    QString addressPlaceholderText = secCurrency + " address";
    if (secCurrency=="BTC") {
        if (config->getNetwork().contains("main", Qt::CaseSensitivity::CaseInsensitive ))
            addressPlaceholderText = "BTC Pubkey hash address. Leading symbol '1'";
        else
            addressPlaceholderText = "BTC testnet Pubkey hash address. Leading symbol 'm' or 'n'";
    }
    ui->redeemAddressEdit->setPlaceholderText(addressPlaceholderText);

    ui->secTransFeeLabel->setText(secCurrency + " transaction fee");
    ui->updateBtn->setText("Update " + secCurrency + " transaction details");
    ui->secFeeUnitsLabel->setText(secCurrencyFeeUnits);

    QString tradeDescription = swapInfo[1];
    ui->tradeDesriptionLabel->setText(tradeDescription);
    redeemAddress = swapInfo[2];
    secondaryCurrency = swapInfo[3];
    secondaryFee = swapInfo[4];
    electrumX = swapInfo[8];

    if (acceptanceMode) {
        redeemAddress = "";
        secondaryFee = "";
        electrumX = "";
        ui->secondaryFeeEdit->setText(swap->getSecondaryFee(secondaryCurrency));
    } else {
        ui->secondaryFeeEdit->setText(secondaryFee);
    }

    ui->redeemAddressEdit->setText(redeemAddress);
    ui->electrumXEdit->setText(electrumX);

    updateButtons();
}

bool EditSwap::isCanUpdate() const {
    return !(redeemAddress == ui->redeemAddressEdit->text() &&
             secondaryFee == ui->secondaryFeeEdit->text() &&
             electrumX == ui->electrumXEdit->text());
}

void EditSwap::updateButtons(bool first_call) {
    if (acceptanceMode)
        return;

    bool canUpdate = false;
    if (!first_call)
        canUpdate = isCanUpdate();
    ui->updateBtn->setEnabled(canUpdate);
    ui->tradeDetailsBtn->setEnabled(!canUpdate);
}


void EditSwap::on_redeemAddressEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_secondaryFeeEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_electrumXEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
    updateButtons();
}

void EditSwap::on_updateBtn_clicked() {
    // Check what we need update...
    if (requestUpdateData() < 0)
        return;

    updateButtons();
}

void EditSwap::on_tradeDetailsBtn_clicked() {
    if (acceptanceMode) {
        // Acceptance Mode

        int request = requestUpdateData();

        if (request < 0)
            return;

        Q_ASSERT(request >= 2);
        requests2accept = request;
        ui->tradeDetailsBtn->setEnabled(false);
    } else {
        // Normal usage, view and switch to the Details
        if (isCanUpdate()) {
            if (core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Warning",
                                                                                         "You have not saved data. Do you want to drop the data and switch to the details?",
                                                                                         "No", "Yes",
                                                                                         "Stay in this page",
                                                                                         "Drop unsaved data and switch",
                                                                                         true, false))
                return;
        }
        swap->showTradeDetails(swapId);
    }
}

// Validate the data and call for update. Return number of update calls.
int EditSwap::requestUpdateData() {
    if (ui->redeemAddressEdit->text().isEmpty()) {
        control::MessageBox::messageText(this, "Input", QString("Please define the ") + secondaryCurrency +
                                                        " address to receive the coins.");
        return -1;
    }

    QString fee = ui->secondaryFeeEdit->text();
    bool ok = false;
    double feeDbl = fee.toDouble(&ok);
    if (fee.isEmpty() || !ok || feeDbl <= 0.0) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") + secondaryCurrency + " fee value");
        return -1;
    }

    ui->progress->show();
    int res = 0;

    if (redeemAddress != ui->redeemAddressEdit->text()) {
        swap->updateSecondaryAddress(swapId, ui->redeemAddressEdit->text());
        res++;
    }
    if (secondaryFee != ui->secondaryFeeEdit->text()) {
        swap->updateSecondaryFee(swapId, feeDbl);
        res++;
    }
    if (electrumX != ui->electrumXEdit->text()) {
        swap->updateElectrumX(swapId, ui->electrumXEdit->text());
        res++;
    }
    return res;
}


void EditSwap::sgnUpdateXXX(QString swId, QString errorMsg) {
    if (swapId != swId)
        return;

    if (!errorMsg.isEmpty()) {
        control::MessageBox::messageText(this, "Error", "Unable to update data for swap " + swapId + "\n\n" + errorMsg);
        ui->progress->hide();
        if (acceptanceMode) {
            ui->tradeDetailsBtn->setEnabled(true);
        }
    } else {
        if (acceptanceMode) {
            requests2accept--;
            if (requests2accept == 0) {
                // We are good, we can accept the trade!!!

                swap->acceptTheTrade(swapId);
            }
        } else {
            // OK, let's refresh...
            swap->requestTradeDetails(swapId);
        }
    }

    updateButtons();
}

void EditSwap::on_backButton_clicked() {
    if (isCanUpdate()) {
        if (core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Warning",
                                                                                     "You have not saved data. Do you want to drop the data and return back?",
                                                                                     "No", "Yes",
                                                                                     "Stay in this page",
                                                                                     "Drop unsaved data and switch",
                                                                                     true, false))
            return;
    }
    swap->pageTradeList();
}

void EditSwap::on_noteEdit_textEdited(const QString &str) {
    Q_UNUSED(str)
}

}
