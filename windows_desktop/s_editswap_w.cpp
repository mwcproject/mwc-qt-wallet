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
#include "../bridge/wnd/swap_b.h"
#include "../bridge/config_b.h"
#include "../control_desktop/messagebox.h"
#include "../state/s_swap.h"

namespace wnd {

EditSwap::EditSwap(QWidget *parent, QString _swapId, QString _stateCmd) :
        core::NavWnd(parent),
        ui(new Ui::EditSwap),
        swapId(_swapId) {
    ui->setupUi(this);

    swap = new bridge::Swap(this);
    config = new bridge::Config(this);

    connect(swap, &bridge::Swap::sgnRequestTradeDetails, this, &EditSwap::sgnRequestTradeDetails, Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnAdjustSwapTrade, this, &EditSwap::sgnAdjustSwapTrade, Qt::QueuedConnection);

    setPageTitle("Trade: " + swapId);

    // Check if we need to accept the deal.
    acceptanceMode = !config->isTradeAccepted(swapId) && swap->isSwapWatingToAccept(_stateCmd);

    if (acceptanceMode) {
        ui->updateBtn->hide();
        ui->tradeDetailsBtn->setText("Accept");
    }

    ui->secondaryAddressLabel->setText("");
    ui->secFeeUnitsLabel->hide();

    ui->progress->initLoader(true);
    swap->requestTradeDetails( swapId, "EditSwap" );

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
                                      QString errMsg,
                                      QString cookie ) {
    Q_UNUSED(currentAction)
    Q_UNUSED(executionPlan)
    Q_UNUSED(tradeJournal)

    if ( cookie != "EditSwap" )
        return;

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

    Q_ASSERT(swapInfo.size() == 11);
    if (swapInfo.size() != 11 || swapInfo[0] != swapId)
        return; // Invalid message or invalid destination

    QString secCurrency = swapInfo[3];
    QString secCurrencyFeeUnits = swapInfo[5]; // - secondary fee units

    // [1] - Description in HTML format. Role can be calculated form here as "Selling ..." or "Buying ..."
    int sellIdx = swapInfo[1].indexOf("Selling"); // Text in HTML formal, "Start With" will not work
    bool seller = sellIdx > 200 && sellIdx < 300;

    QString addressName = secCurrency;
    if (addressName == "ZCash")
        addressName = "Transparent ZCash";

    if (seller)
        ui->secondaryAddressLabel->setText(secCurrency + " address to receive the coins");
    else
        ui->secondaryAddressLabel->setText(secCurrency + " address for the refund in case of trade cancellation");

    QString addressPlaceholderText = addressName + " address";
    ui->redeemAddressEdit->setPlaceholderText(addressPlaceholderText);

    ui->secTransFeeLabel->setText(secCurrency + " transaction fee");
    ui->updateBtn->setText("Update " + secCurrency + " transaction details");
    ui->secFeeUnitsLabel->show();
    ui->secFeeUnitsLabel->setText(secCurrencyFeeUnits);
    if (!state::getCurrencyInfo(secCurrency).is_btc_family)
        ui->redeemAddressEdit->setDisabled(true);

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
        QString comment = ui->accept_note->text();
        ui->accept_note->setText( secondaryCurrency + comment.mid(3) );

        double mwcLimit = 0.1;
        double secCurrencyLimit = 0.001;

        double mwcAmount = swapInfo[9].toDouble();
        double secAmount = swapInfo[10].toDouble();

        if (mwcAmount < mwcLimit || secAmount < secCurrencyLimit ) {
            // Normally it should never happpens. Just show the message in case if somebody will messup.
            control::MessageBox::messageText(this, "WARNING", "This Swap offer has an unreasonable small amounts to trade. Because of that you can't accept it. We recommend you to cancel this offer.");
            swap->pageTradeList();
            return;
        }

    } else {
        ui->secondaryFeeEdit->setText(secondaryFee);
        ui->accept_verticalSpacer->setGeometry(QRect(0,0,1,1));
        ui->accept_note->hide();
    }

    ui->redeemAddressEdit->setText(redeemAddress);
    ui->electrumXEdit->setText(electrumX);

    updateButtons();
}

bool EditSwap::isCanUpdate() const {
    return !(redeemAddress == ui->redeemAddressEdit->text().trimmed() &&
             secondaryFee == ui->secondaryFeeEdit->text().trimmed() &&
             electrumX == ui->electrumXEdit->text().trimmed());
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
    if (!requestUpdateData())
        return;

    updateButtons();
}

void EditSwap::on_tradeDetailsBtn_clicked() {
    if (acceptanceMode) {
        // Acceptance Mode

        if (requestUpdateData())
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
bool EditSwap::requestUpdateData() {
    bool is_btc_family = state::getCurrencyInfo(secondaryCurrency).is_btc_family;

    if (ui->redeemAddressEdit->text().trimmed().isEmpty() && is_btc_family) {
        
        control::MessageBox::messageText(this, "Input", QString("Please define the ") + secondaryCurrency +
                                                        " address to receive the coins.");
        return false;
    }

    QString fee = ui->secondaryFeeEdit->text().trimmed();
    bool ok = false;
    double feeDbl = fee.toDouble(&ok);
    if (fee.isEmpty() || !ok || feeDbl <= 0.0) {
        control::MessageBox::messageText(this, "Input", QString("Please define ") + secondaryCurrency + " fee value");
        return false;
    }

    ui->progress->show();

    QString secondaryAddress;
    QString secondaryFee;
    QString electrumUri1;

    if (redeemAddress != ui->redeemAddressEdit->text().trimmed()) {
        secondaryAddress = ui->redeemAddressEdit->text().trimmed();
    }
    if (secondaryFee != ui->secondaryFeeEdit->text().trimmed()) {
        secondaryFee = QString::number(feeDbl);
    }
    if (electrumX != ui->electrumXEdit->text().trimmed()) {
        electrumUri1 = ui->electrumXEdit->text().trimmed();
    }

    if (!is_btc_family) 
        secondaryAddress = "0x0000000000000000000000000000000000000000";

    if ( secondaryAddress.isEmpty() && secondaryFee.isEmpty() && electrumUri1.isEmpty() )
        return false; // no changes are made

    swap->adjustSwapData(swapId, "",
            "", "",
            secondaryAddress,
            secondaryFee,
            electrumUri1,
            "" );
    return true;
}


void EditSwap::sgnAdjustSwapTrade(QString swId, QString cmdTag, QString errorMsg) {
    Q_UNUSED(cmdTag)

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
           swap->acceptTheTrade(swapId);
        } else {
            // OK, let's refresh...
            swap->requestTradeDetails(swapId, "EditSwap");
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
