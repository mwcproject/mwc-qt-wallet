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

#include "s_newswap_w.h"
#include "ui_s_newswap_w.h"
#include "../bridge/swap_b.h"
#include "../control_desktop/messagebox.h"
#include <float.h>

namespace wnd {

NewSwap::NewSwap(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::NewSwap) {
    ui->setupUi(this);
    swap = new bridge::Swap(this);

    connect(swap, &bridge::Swap::sgnCreateNewTradeResult, this, &NewSwap::sgnCreateNewTradeResult,
            Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->secondaryCurrencyCombo->clear();
    for (auto &sc : swap->getSecondaryCurrencyList()) {
        ui->secondaryCurrencyCombo->addItem(sc);
    }

    // Seller Lock First - default
    ui->lockOrderCombo->setCurrentIndex(0);
}

NewSwap::~NewSwap() {
    delete ui;
}

void NewSwap::on_calcMwcConfBtn_clicked() {
    QString mwc2trade = ui->mwcAmountEdit->text();
    bool ok = false;
    double mwcAmount = mwc2trade.toDouble(&ok);

    if (ok) {
        int confirmations = swap->calcConfirmationsForMwcAmount(mwcAmount);
        if (confirmations > 0) {
            ui->mwcLockConfirmationsEdit->setText(QString::number(confirmations));
        }
        else {
            control::MessageBox::messageText(this, "Warning", "Wallet unable to contact the node to calculate the optimal confirmations number. Please check your MWC Node connection.");
        }
    }
}

void NewSwap::on_backBtn_clicked() {
    swap->pageTradeList();
}

bool NewSwap::getValue(QLineEdit *edit, double min, double max, double &result, QString message) {
    QString str = edit->text();
    bool ok = false;
    result = str.toDouble(&ok);

    if (str.isEmpty() || !ok || result < min || result > max) {
        control::MessageBox::messageText(this, "Input Value", message);
        edit->setFocus();
        return false;
    }
    return true;
}

bool NewSwap::getValue(QLineEdit *edit, int min, int max, int &result, QString message) {
    QString str = edit->text();
    bool ok = false;
    result = str.toDouble(&ok);

    if (str.isEmpty() || !ok || result < min || result > max) {
        control::MessageBox::messageText(this, "Input Value", message);
        edit->setFocus();
        return false;
    }
    return true;
}

void NewSwap::on_createBtn_clicked() {
    double mwcAmount = 0.0;
    if (!getValue(ui->mwcAmountEdit, 0.0000001, DBL_MAX, mwcAmount, "Please specify MWC amount to trade"))
        return;

    QString secCurrency = ui->secondaryCurrencyCombo->currentText();

    double secAmount = 0.0;

    if (!getValue(ui->secondaryAmountEdit, 0.0000001, DBL_MAX, secAmount,
                  "Please specify " + secCurrency + " amount to trade"))
        return;

    QString redeemAddress = ui->secondaryAddressEdit->text();
    if (redeemAddress.isEmpty()) {
        control::MessageBox::messageText(this, "Input Value", "Please specify redeem address");
        ui->secondaryAddressEdit->setFocus();
        return;
    }

    int messageExchangeTime = 0;
    if (!getValue(ui->messageExchageTimeEdit, 10, 1440, messageExchangeTime,
                  "Please specify message exchange time in the range from 10 to 1440 minutes"))
        return;
    int redeemTime = 0;
    if (!getValue(ui->redeemTimeEdit, 60, 1440, redeemTime,
                  "Please specify redeem time in the range from 60 to 1440 minutes"))
        return;

    int mwcConfNumber = 0;
    if (!getValue(ui->mwcLockConfirmationsEdit, 10, 10000, mwcConfNumber,
                  "Please specify number of MWC conformations in the range from 10 to 10000"))
        return;

    int secondaryConfNumber = 0;
    QVector<int> secMinMax = swap->getConfirmationLimitForSecondary(secCurrency);
    Q_ASSERT(secMinMax.size() == 2);
    if (!getValue(ui->secondaryLockConfirmationEdit, secMinMax[0], secMinMax[1], secondaryConfNumber,
                  "Please specify number of " + secCurrency + " conformations in the range from " +
                  QString::number(secMinMax[0]) + " to " + QString::number(secMinMax[1])))
        return;

    QString method;
    if (ui->mwcmqsRadio->isChecked()) {
        method = "mwcmqs";
    }
    if (ui->torRadio->isChecked()) {
        method = "tor";
    }
    if (method.isEmpty()) {
        control::MessageBox::messageText(this, "Input Value", "Please specify communication method");
        ui->torRadio->setFocus();
        return;
    }

    QString destination = ui->destinationEdit->text();
    if (destination.isEmpty()) {
        control::MessageBox::messageText(this, "Input Value", "Please specify Buyer destination address");
        ui->destinationEdit->setFocus();
        return;
    }


    ui->progress->show();

    swap->createNewTrade(mwcAmount, secAmount, secCurrency.toLower(),
                         redeemAddress,
                         ui->lockOrderCombo->currentIndex() == 0,
                         messageExchangeTime,
                         redeemTime,
                         mwcConfNumber,
                         secondaryConfNumber,
                         method,
                         destination);
}

// Response from createNewSwapTrade, SwapId on OK,  errMsg on failure
void NewSwap::sgnCreateNewTradeResult( QString swapId, QString errMsg ) {
    ui->progress->hide();
    if (!errMsg.isEmpty()) {
        control::MessageBox::messageText(this, "New Swap Trade Error",
                                         "Unable to create a new trade because of the error:\n\n" +
                                         errMsg);
    } else {
        control::MessageBox::messageText(this, "New Swap Trade",
                  "New swap trade " + swapId + " was sucessfully created.");
        swap->viewTrade(swapId);
    }
}

void NewSwap::on_mwcAmountEdit_textEdited(const QString &str) {
    Q_UNUSED(str);
    updateRate();
}

void NewSwap::on_secondaryAmountEdit_textEdited(const QString &str) {
    Q_UNUSED(str);
    updateRate();
}

void NewSwap::on_exchangeRateEdit_textEdited(const QString &str) {
    Q_UNUSED(str);
    updateSecondary();
}

void NewSwap::updateRate() {
    bool ok1 = false;
    double mwc = ui->mwcAmountEdit->text().toDouble(&ok1);
    bool ok2 = false;
    double sec = ui->secondaryAmountEdit->text().toDouble(&ok2);

    if (ok1 && ok2 && mwc>0.0 && sec>0.0) {
        ui->exchangeRateEdit->setText( QString::number( sec/mwc, 'g', 8 ) );
    }
}
void NewSwap::updateSecondary() {
    bool ok1 = false;
    double mwc = ui->mwcAmountEdit->text().toDouble(&ok1);
    bool ok2 = false;
    double rate = ui->exchangeRateEdit->text().toDouble(&ok2);

    if (ok1 && ok2 && mwc>0.0 && rate>0.0) {
        ui->secondaryAmountEdit->setText( QString::number( mwc * rate, 'g', 8 ) );
    }
}


}
