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

#include "s_newswap2_w.h"
#include "ui_s_newswap2_w.h"
#include "../bridge/swap_b.h"
#include "../bridge/util_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

NewSwap2::NewSwap2(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::NewSwap2) {
    ui->setupUi(this);

    swap = new bridge::Swap(this);
    util = new bridge::Util(this);

    connect(swap, &bridge::Swap::sgnApplyNewTrade2Params, this, &NewSwap2::onSgnApplyNewTrade2Params,
            Qt::QueuedConnection);

    initTimeIntervalCombo(ui->offerExpirationCombo, swap->getOfferExpirationInterval());
    initTimeIntervalCombo(ui->secRedeemCombo, swap->getSecRedeemTime());

    QString secCurrency = swap->getCurrentSecCurrency();
    ui->secTransFeeLabel->setText(secCurrency + " transaction fee");
    ui->secTransFeeEdit->setText( util->trimStrAsDouble( QString::number( swap->getSecTransactionFee(), 'f'), 10 ) );
    ui->secFeeUnitsLabel->setText(swap->getCurrentSecCurrencyFeeUnits());

    ui->mwcBlocksEdit->setText( QString::number(swap->getMwcConfNumber()) );
    ui->secBlocksLabel->setText(secCurrency);
    ui->secBlocksEdit->setText( QString::number(swap->getSecConfNumber()) );

    ui->secLockCurrencyLabel->setText(secCurrency);

    updateFundsLockTime();

    ui->progress_2->initLoader(false);
}

NewSwap2::~NewSwap2() {
    delete ui;
}

void NewSwap2::initTimeIntervalCombo(control::MwcComboBox * timeIntervalCombo, int interval2select) {
    QVector<QString> intervals = swap->getExpirationIntervals();
    Q_ASSERT(intervals.size() % 2 == 0);

    timeIntervalCombo->clear();

    int selectedIdx = 0;
    for (int i = 1; i < intervals.size(); i += 2) {
        if (intervals[i].toInt() == interval2select)
            selectedIdx = i/2;

        timeIntervalCombo->addItem( intervals[i-1], QVariant(intervals[i].toInt()) );
    }
    timeIntervalCombo->setCurrentIndex(selectedIdx);
}

void NewSwap2::on_backButton_clicked() {
    swap->showNewTrade1();
}

void NewSwap2::on_reviewButton_clicked() {
    bool offerOk = false;
    int offerExpTime = ui->offerExpirationCombo->currentData().toInt(&offerOk);

    if (!offerOk || offerExpTime<=0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify offer expiration time interval.");
        ui->offerExpirationCombo->setFocus();
        return;
    }

    QString secCurrency = swap->getCurrentSecCurrency();

    bool redeemOk = false;
    int redeemTime = ui->secRedeemCombo->currentData().toInt(&redeemOk);
    if (!redeemOk || redeemTime<=0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify expected "+secCurrency+" redeem time.");
        ui->secRedeemCombo->setFocus();
        return;
    }

    bool secTxFeeOk = false;
    double secTxFee = ui->secTransFeeEdit->text().toDouble(&secTxFeeOk);
    if (!secTxFeeOk || secTxFee<0.01) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify correct "+secCurrency+" transaction fee.");
        ui->secTransFeeEdit->setFocus();
        return;
    }

    bool mwcBlocksOk = false;
    int  mwcBlocks = ui->mwcBlocksEdit->text().toInt(&mwcBlocksOk);
    if (!mwcBlocksOk || mwcBlocks<=0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify number of confirmations for MWC coins.");
        ui->mwcBlocksEdit->setFocus();
        return;
    }

    bool secBlocksOk = false;
    int  secBlocks = ui->secBlocksEdit->text().toInt(&secBlocksOk);
    if (!secBlocksOk || secBlocks<=0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify number of confirmations for "+secCurrency+" coins.");
        ui->secBlocksEdit->setFocus();
        return;
    }

    QString electrumXurl = ui->electrumXEdit->text();

    ui->progress_2->show();
    swap->applyNewTrade2Params(secCurrency, offerExpTime, redeemTime, mwcBlocks, secBlocks, secTxFee, electrumXurl);
}

void NewSwap2::onSgnApplyNewTrade2Params(bool ok, QString errorMessage) {
    ui->progress_2->hide();

    if (ok)
        return;

    Q_ASSERT(!errorMessage.isEmpty());
    control::MessageBox::messageText(this, "Incorrect Input", errorMessage);
}

void NewSwap2::on_offerEpirationHelpButton_clicked() {
    control::MessageBox::messageText(this, "Help", "Please specify enough time for initial offer exchnage and start locking funds. "
                                                   "If you don't reserve enough time, the swap will be automatically cancelled because of the timeout.");
}

void NewSwap2::on_secRedeemHelpButton_clicked() {
    QString secCurrency = swap->getCurrentSecCurrency();
    control::MessageBox::messageText(this, "Help", "Please specify enough time to redeem " + secCurrency +
                                                   " transaction was issues and mined. "
                                                   "This configuration is related to the transaction fee. If you failed to estimate this time correctly, your swap trade will be vulnerable.\n\n"
                                                   "Please Note, if you will not be able to redeem your funds before lock time, "
                                                   "you might loose them.");
}

void NewSwap2::on_secTransFeeHelpButton_clicked() {
    QString secCurrency = swap->getCurrentSecCurrency();
    control::MessageBox::messageText(this, "Help", "Please specify transaction fee high enough, so your transactions will be mined during redeem time interval. "
                                                   "If you failed to redeem during estimated time interval, your swap trade will be vulnerable.\n\n"
                                                   "Please Note, if you will not be able to redeem your funds before lock time, "
                                                   "you might loose them.");
}

void NewSwap2::on_electrumXHelpButton_clicked() {
    control::MessageBox::messageText(this, "Help", "MWC team is providing community ElectrumX servers, but for the extra security and privacy we "
                                                   "recommend you to use your own ElectrumX instance. The secondary fail over instance will be community provided.");
}

void NewSwap2::on_confNumberHelpButton_clicked() {
    control::MessageBox::messageText(this, "Help", "Please specify confirmation number large enough to make reorg attack to your swap trade non profitable. Larger number you put, more expensive it will be for attacker.");
}

void NewSwap2::on_secRedeemCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    updateFundsLockTime();
}

void NewSwap2::on_offerExpirationCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    updateFundsLockTime();
}

void NewSwap2::on_mwcBlocksEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)
    updateFundsLockTime();
}

void NewSwap2::on_secBlocksEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)
    updateFundsLockTime();
}

void NewSwap2::updateFundsLockTime() {
    bool offerOk = false;
    int offerExpTime = ui->offerExpirationCombo->currentData().toInt(&offerOk);
    bool redeemOk = false;
    int redeemTime = ui->secRedeemCombo->currentData().toInt(&redeemOk);

    bool mwcBlocksOk = false;
    int  mwcBlocks = ui->mwcBlocksEdit->text().toInt(&mwcBlocksOk);
    bool secBlocksOk = false;
    int  secBlocks = ui->secBlocksEdit->text().toInt(&secBlocksOk);

    if (offerOk && offerExpTime>0 && redeemOk && redeemTime>0 &&
            mwcBlocksOk && mwcBlocks>0 && secBlocksOk && secBlocks>0 ) {
        QVector<QString> lockTimeStr = swap->getLockTime( swap->getCurrentSecCurrency(), offerExpTime,
                                                          redeemTime, mwcBlocks, secBlocks );
        Q_ASSERT(lockTimeStr.size() == 2);

        ui->mwcLockTimeLabel->setText(lockTimeStr[0]);
        ui->secLockTimeLabel->setText(lockTimeStr[1]);
    }
}

}
