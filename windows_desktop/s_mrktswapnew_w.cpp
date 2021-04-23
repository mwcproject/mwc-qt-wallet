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

#include "s_mrktswapnew_w.h"
#include "ui_s_mrktswapnew_w.h"
#include "../bridge/wnd/swapmkt_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/swap_b.h"
#include <QDebug>
#include "../control_desktop/messagebox.h"

const int UPDATE_MWC = 1;
const int UPDATE_SEC = 2;
const int UPDATE_RATE = 3;

namespace wnd {

MrktSwapNew::MrktSwapNew(QWidget *parent, QString myMsgId) :
        core::NavWnd(parent),
        ui(new Ui::MrktSwapNew) {
    ui->setupUi(this);

    ui->progress->initLoader(false);

    config = new bridge::Config(this);
    swapMarketplace = new bridge::SwapMarketplace(this);
    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);
    swap = new bridge::Swap(this);

    connect(wallet, &bridge::Wallet::sgnWalletBalanceUpdated, this, &MrktSwapNew::onSgnWalletBalanceUpdated,
            Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnApplyNewTrade1Params, this, &MrktSwapNew::onSgnApplyNewTrade1Params,
            Qt::QueuedConnection);

    if (!myMsgId.isEmpty()) {
        QVector<QString> offers = swapMarketplace->getMyOffers();

        for (auto & s : offers) {
            state::MySwapOffer myOffer(s);
            if (myOffer.offer.id == myMsgId) {
                offer = myOffer;
                break;
            }
        }

        Q_ASSERT(myMsgId.isEmpty() == offer.offer.id.isEmpty()); // Not found?

        swap->setAccount( offer.account );
        swap->setCurrentSecCurrency(offer.offer.secondaryCurrency);

        // Disable most of the UI because of update.
        ui->accountComboBox->setEnabled(false);
        ui->buySellCombo->setEnabled(false);
        ui->mwcAmountEdit->setEnabled(false);
        ui->secCurrencyCombo->setEnabled(false);
        ui->secAmountEdit->setEnabled(false);
        ui->swapRateEdit->setEnabled(false);
        ui->mwcBlocksEdit->setEnabled(false);
        ui->secBlocksEdit->setEnabled(false);

        ui->cancelButton->setText("Back");
    }

    Q_ASSERT(myMsgId.isEmpty() == offer.offer.id.isEmpty()); // Not found?

    if ( !offer.offer.id.isEmpty() ) {
        if (offer.offer.sell)
            ui->buySellCombo->setCurrentIndex(1);  // Sell
        else
            ui->buySellCombo->setCurrentIndex(0); // Buy

        ui->mwcAmountEdit->setText(QString::number(offer.offer.mwcAmount));
        ui->secAmountEdit->setText(QString::number(offer.offer.secAmount));
        thirdValueUpdate.push_back(UPDATE_MWC);
        thirdValueUpdate.push_back(UPDATE_SEC);
        ui->secAddressEdit->setText( offer.secAddress );
        ui->noteEdit->setText(offer.note);
    }

    if (!offer.offer.secondaryCurrency.isEmpty()) {
        swap->setCurrentSecCurrency(offer.offer.secondaryCurrency);
    }

    updateSecCurrencyData();
    updateThirdValue();

    onSgnWalletBalanceUpdated();

    // Fee need to be updated at the end becuase currency tab does reset to the default one.
    if ( !offer.offer.id.isEmpty() ) {
        QString feeStr = QString::number(offer.secFee);
        if (ui->secTransFeeEdit->text() != feeStr) {
            ui->secTransFeeEdit->setText(feeStr);
            ui->secTransFeeLabel2->hide();
        }
    }
}

MrktSwapNew::~MrktSwapNew() {
    delete ui;
}


void MrktSwapNew::updateSecCurrencyData() {
    ui->secCurrencyCombo->clear();

    QVector<QString> curList = swap->secondaryCurrencyList();
    QString selectedCur = swap->getCurrentSecCurrency();
    int selectedIdx = 0;
    for ( int i=0; i<curList.size(); i++) {
        const auto & c = curList[i];
        ui->secCurrencyCombo->addItem( c, QVariant(c));
        if (c==selectedCur)
            selectedIdx = i;
    }
    ui->secCurrencyCombo->setCurrentIndex( selectedIdx );

    updateSecCurrencyStatus();
}

void MrktSwapNew::updateSecCurrencyStatus() {
    QString selectedCur = swap->getCurrentSecCurrency();
    if (selectedCur.isEmpty())
        selectedCur = "XXX";

    ui->rateLabel->setText("MWC to " + selectedCur + " rate:");
    QString addressPlaceholderText;
    if (selectedCur == "ZCash")
        addressPlaceholderText = "Transparent ZCash address to receive the coins";
    else
        addressPlaceholderText = selectedCur + " address to receive the coins";

    ui->secAddressEdit->setPlaceholderText(addressPlaceholderText);
    ui->secondaryCurrencyLabel->setText(selectedCur + " address:");

    ui->secAmountEdit->setPlaceholderText(selectedCur + " amount");

    ui->secTransFeeLabel->setText(selectedCur + " transaction fee:");
    ui->secFeeUnitsLabel->setText(swap->getCurrentSecCurrencyFeeUnits());
    ui->secBlocksLabel->setText(selectedCur);

    ui->secTransFeeLabel2->setText("(Recommended transaction fee)");

    double fee = swap->getSecTransactionFee(selectedCur);
    if (fee>0) {
        ui->secTransFeeEdit->setText(util->trimStrAsDouble(QString::number(fee, 'f'), 5));
        ui->secTransFeeLabel2->show();
    }
    else {
        ui->secTransFeeLabel2->hide();
    }

    ui->secLockCurrencyLabel->setText(selectedCur);
    ui->secBlocksEdit->setText( QString::number(swap->getSecConfNumber(selectedCur)) );
}

void MrktSwapNew::updateThirdValue() {

    bool mwcOk = false;
    bool secOk = false;
    bool rateOk = false;

    double mwc = ui->mwcAmountEdit->text().toDouble(&mwcOk);
    double sec = ui->secAmountEdit->text().toDouble(&secOk);
    double rate = ui->swapRateEdit->text().toDouble(&rateOk);

    if (thirdValueUpdate.size()<2)
        return; // Just do nothing, there is nothing to update

    QVector<int> three {UPDATE_MWC, UPDATE_SEC, UPDATE_RATE};
    three.removeAll( thirdValueUpdate[thirdValueUpdate.size()-1] );
    three.removeAll( thirdValueUpdate[thirdValueUpdate.size()-2] );
    Q_ASSERT(three.size()==1);

    switch( three[0] ) {
        case UPDATE_MWC: {
            if (secOk && rateOk && sec>0.0 && rate>0.0) {
                ui->mwcAmountEdit->setText( util->trimStrAsDouble( QString::number( sec / rate, 'f', 6 ), 13) );
            }
            else {
                ui->mwcAmountEdit->setText("");
            }
            break;
        }
        case UPDATE_SEC: {
            if (mwcOk && rateOk && mwc>0.0 && rate>0.0) {
                ui->secAmountEdit->setText( util->trimStrAsDouble( QString::number( mwc * rate, 'f', 6 ), 13) );
            }
            else {
                ui->secAmountEdit->setText("");
            }
            break;
        }
        case UPDATE_RATE: {
            if (mwcOk && secOk && mwc>0.0 && sec>0.0) {
                ui->swapRateEdit->setText( util->trimStrAsDouble( QString::number( sec/mwc, 'f', 9 ), 13) );
            }
            else {
                ui->swapRateEdit->setText("");
            }
            break;
        }
        default:
            Q_ASSERT(false);
    }
}

void MrktSwapNew::updateFundsLockTime() {
    bool mwcBlocksOk = false;
    int  mwcBlocks = ui->mwcBlocksEdit->text().toInt(&mwcBlocksOk);
    bool secBlocksOk = false;
    int  secBlocks = ui->secBlocksEdit->text().toInt(&secBlocksOk);

    if (
        mwcBlocksOk && mwcBlocks>0 && secBlocksOk && secBlocks>0 ) {
        // Using standard timings
        QVector<QString> lockTimeStr = swap->getLockTime( swap->getCurrentSecCurrency(), 60,
                                                          60, mwcBlocks, secBlocks );
        Q_ASSERT(lockTimeStr.size() == 2);

        ui->mwcLockTimeLabel->setText(lockTimeStr[0]);
        ui->secLockTimeLabel->setText(lockTimeStr[1]);
    }
}

void MrktSwapNew::onSgnWalletBalanceUpdated() {
    // init accounts
    ui->accountComboBox->clear();

    QString account = swap->getAccount();
    if (account.isEmpty())
        account = wallet->getCurrentAccountName();

    QVector<QString> accountInfo = wallet->getWalletBalance(true, true, false);

    int selectedAccIdx = 0;
    int idx = 0;

    for (int i = 1; i < accountInfo.size(); i += 2) {
        if ( accountInfo[i-1] == "integrity")
            continue;

        if (accountInfo[i - 1] == account)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem(accountInfo[i], QVariant(accountInfo[i - 1]));
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
}

void MrktSwapNew::on_buySellCombo_currentIndexChanged(int index) {
    Q_UNUSED(index)
}

void MrktSwapNew::on_mwcAmountEdit_textChanged(const QString &mwcAmount) {
    bool ok = false;
    double mwc = mwcAmount.toDouble(&ok);
    if (ok) {
        int mwcConf = swap->getMwcConfNumber(mwc);
        ui->mwcBlocksEdit->setText( QString::number(mwcConf));
    }
}

void MrktSwapNew::on_secCurrencyCombo_currentIndexChanged(int index) {
    if (index<0)
        return;

    QString selectedCurrency = ui->secCurrencyCombo->currentData().toString();
    swap->setCurrentSecCurrency(selectedCurrency);

    updateSecCurrencyStatus();
}

void MrktSwapNew::on_mwcAmountEdit_textEdited(const QString &arg1) {
    Q_UNUSED(arg1)
    thirdValueUpdate.removeAll(UPDATE_MWC);
    thirdValueUpdate.push_back(UPDATE_MWC);
    updateThirdValue();
}

void MrktSwapNew::on_secAmountEdit_textEdited(const QString &arg1) {
    Q_UNUSED(arg1)
    thirdValueUpdate.removeAll(UPDATE_SEC);
    thirdValueUpdate.push_back(UPDATE_SEC);
    updateThirdValue();
}

void MrktSwapNew::on_swapRateEdit_textEdited(const QString &arg1) {
    Q_UNUSED(arg1)
    thirdValueUpdate.removeAll(UPDATE_RATE);
    thirdValueUpdate.push_back(UPDATE_RATE);
    updateThirdValue();
}

void MrktSwapNew::on_swapRateHelpBtn_clicked() {
    control::MessageBox::messageText(this, "Help", "MWC to " + ui->secCurrencyCombo->currentData().toString() + " exchange rate. Please note, this rate is calculated before transaction fees that needs to be paid.");
}

void MrktSwapNew::on_secAddressHelpBtn_clicked() {
    control::MessageBox::messageText( this, "Help", ui->secCurrencyCombo->currentData().toString() + " address to receive the " +
            (ui->buySellCombo->currentIndex()==0 ? "refunded" : "traded") + " coins.");
}

void MrktSwapNew::on_secTransFeeHelpButton_clicked() {
    control::MessageBox::messageText( this, "Help", "Please specify transaction fee enough to mine your transaction during 30 minutes. Timing restrictions are critical for atomic swap safety.");
}

void MrktSwapNew::on_mwcBlocksEdit_textEdited(const QString &arg1) {
    Q_UNUSED(arg1)
    updateFundsLockTime();
}

void MrktSwapNew::on_secBlocksEdit_textEdited(const QString &arg1) {
    Q_UNUSED(arg1)
    updateFundsLockTime();
}

void MrktSwapNew::on_cancelButton_clicked() {
    swapMarketplace->pageMktList();
}

void MrktSwapNew::on_submitButton_clicked() {
    QString account = ui->accountComboBox->currentData().toString();
    QString secCurrency = ui->secCurrencyCombo->currentData().toString();
    if (secCurrency.isEmpty()) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please currency for swap");
        ui->secCurrencyCombo->setFocus();
        return;
    }

    bool mwcOk = false;
    bool secOk = false;

    QString mwcAmount = ui->mwcAmountEdit->text().trimmed();
    double mwc = mwcAmount.toDouble(&mwcOk);
    QString secAmount = ui->secAmountEdit->text().trimmed();
    double sec = secAmount.toDouble(&secOk);

    double mwcLimit = 0.1;
    double secCurrencyLimit = swap->getSecMinAmount(secCurrency);

    if (!config->getNetwork().toLower().contains("main")) {
        secCurrencyLimit /= 10.0;
    }

    if ( !mwcOk || mwc<mwcLimit ) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify at least " + QString::number(mwcLimit) + " MWC for swap");
        ui->mwcAmountEdit->setFocus();
        return;
    }

    if (!secOk || sec<secCurrencyLimit) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify at least " + QString::number(secCurrencyLimit) + " " + secCurrency +" for swap");
        ui->secAmountEdit->setFocus();
        return;
    }

    QString secAddress = ui->secAddressEdit->text().trimmed();
    if (secAddress.isEmpty()) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify a "+ secCurrency +" address to redeem the coins");
        ui->secAddressEdit->setFocus();
        return;
    }

    bool secTxFeeOk = false;
    double secTxFee = ui->secTransFeeEdit->text().toDouble(&secTxFeeOk);
    double minTx = swap->getSecMinTransactionFee(secCurrency);
    double maxTx = swap->getSecMaxTransactionFee(secCurrency);
    if (!secTxFeeOk || secTxFee<minTx || secTxFee>maxTx ) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify a correct "+secCurrency+" transaction fee in the range from " +
                                                                  util->trimStrAsDouble( QString::number( minTx, 'f'), 6 ) + " to " + util->trimStrAsDouble( QString::number( maxTx, 'f'), 6 ) + " " + swap->getCurrentSecCurrencyFeeUnits() );
        ui->secTransFeeEdit->setFocus();
        return;
    }

    bool mwcBlocksOk = false;
    int  mwcBlocks = ui->mwcBlocksEdit->text().toInt(&mwcBlocksOk);
    if (!mwcBlocksOk || mwcBlocks<=0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify the number of confirmations for MWC coins.");
        ui->mwcBlocksEdit->setFocus();
        return;
    }

    bool secBlocksOk = false;
    int  secBlocks = ui->secBlocksEdit->text().toInt(&secBlocksOk);
    if (!secBlocksOk || secBlocks<=0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify the number of confirmations for "+secCurrency+" coins.");
        ui->secBlocksEdit->setFocus();
        return;
    }

    offer.account = account;
    offer.offer.secondaryCurrency = secCurrency;
    offer.offer.mwcAmount = mwc;
    offer.offer.secAmount = sec;
    offer.offer.sell = ui->buySellCombo->currentIndex()==1;
    offer.offer.mwcLockBlocks = mwcBlocks;
    offer.offer.secLockBlocks = secBlocks;

    offer.note = ui->noteEdit->text();
    offer.secAddress = secAddress;
    offer.secFee = secTxFee;

    if (!offer.offer.sell) {
        mwcAmount = "";
    }

    ui->submitButton->setEnabled(false);
    ui->progress->show();

    swap->applyNewTrade1Params(account, secCurrency, mwcAmount, secAmount,
            secAddress, "", false );


}

void MrktSwapNew::onSgnApplyNewTrade1Params(bool ok, QString errorMessage) {
    ui->progress->hide();
    ui->submitButton->setEnabled(true);

    if (ok) {
        QString err = swapMarketplace->createNewOffer( offer.offer.id, offer.account,
                offer.offer.sell,  offer.offer.mwcAmount, offer.offer.secAmount,
               offer.offer.secondaryCurrency, offer.offer.mwcLockBlocks, offer.offer.secLockBlocks,
               offer.secAddress,  offer.secFee, offer.note );

        if (!err.isEmpty()) {
            control::MessageBox::messageText(this, "Unable to create an offer", err);
            return;
        }

        swapMarketplace->pageMktMyOffers();
        return;
    }

    Q_ASSERT(!errorMessage.isEmpty());
    control::MessageBox::messageText(this, "Incorrect Input", errorMessage);
}

void MrktSwapNew::on_secTransFeeEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)
    ui->secTransFeeLabel2->hide();
}

}
