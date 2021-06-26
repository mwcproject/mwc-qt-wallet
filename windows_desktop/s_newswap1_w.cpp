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

#include "s_newswap1_w.h"
#include "ui_s_newswap1_w.h"
#include "../bridge/wnd/swap_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include "../bridge/config_b.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/w_selectcontact.h"
#include "../state/s_swap.h"

const int UPDATE_MWC = 1;
const int UPDATE_SEC = 2;
const int UPDATE_RATE = 3;

namespace wnd {

NewSwap1::NewSwap1(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::NewSwap1) {
    ui->setupUi(this);

    swap = new bridge::Swap(this);
    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);
    config = new bridge::Config(this);

    connect(wallet, &bridge::Wallet::sgnWalletBalanceUpdated, this, &NewSwap1::onSgnWalletBalanceUpdated,
            Qt::QueuedConnection);
    connect(swap, &bridge::Swap::sgnApplyNewTrade1Params, this, &NewSwap1::onSgnApplyNewTrade1Params,
            Qt::QueuedConnection);


    updateSecCurrencyData();

    onSgnWalletBalanceUpdated();

    ui->mwcAmountEdit->setText(swap->getMwc2Trade());
    ui->secAmountEdit->setText(swap->getSec2Trade());

    if (!ui->mwcAmountEdit->text().isEmpty())
        thirdValueUpdate.push_back(UPDATE_MWC);
    if (!ui->secAmountEdit->text().isEmpty())
        thirdValueUpdate.push_back(UPDATE_SEC);

    updateThirdValue();

    if (swap->getSecAddress() == "0x0000000000000000000000000000000000000000") {
        ui->secAddressEdit->setText("Internal Eth Wallet Address");
    } else {
        ui->secAddressEdit->setText(swap->getSecAddress());
    }
    ui->lockMwcFirstCheck->setChecked( swap->isLockMwcFirst() );

    ui->sendAddressEdit->setText( swap->getBuyerAddress() );
    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();

    ui->progress->initLoader(false);

    bool mktTrade = swap->isMktTrade();
    ui->mwcAmountEdit->setEnabled(!mktTrade);
    ui->secCurrencyCombo->setEnabled(!mktTrade);
    ui->secAmountEdit->setEnabled(!mktTrade);
    ui->swapRateEdit->setEnabled(!mktTrade);
    ui->lockMwcFirstCheck->setEnabled(!mktTrade);
    ui->sendAddressEdit->setEnabled(!mktTrade);
    ui->contactsButton->setEnabled(!mktTrade);
}

NewSwap1::~NewSwap1() {
    delete ui;
}

void NewSwap1::on_contactsButton_clicked() {
    util::TimeoutLockObject to("SendOnline");

    // Get the contacts

    dlg::SelectContact dlg(this, true, true, false);
    if (dlg.exec() == QDialog::Accepted) {
        core::ContactRecord selectedContact = dlg.getSelectedContact();
        ui->sendAddressEdit->setText( selectedContact.address );
        ui->contactNameLable->setText("     Contact: " + selectedContact.name );
        ui->contactNameLable->show();
        ui->formatsLable->hide();
    }

}

void NewSwap1::on_cancelButton_clicked() {
    swap->pageTradeList();
}

void NewSwap1::on_nextButton_clicked() {
    QString account = ui->accountComboBox->currentData().toString();
    double spendable = getSelectedAccountBalance(account);
    if (spendable<=0.0) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please select account with spendable balance");
        ui->accountComboBox->setFocus();
        return;
    }

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

    if ( !mwcOk || mwc<mwcLimit ) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify at least " + util::zeroDbl2Dbl(QString::number(mwcLimit, 'f')) + " MWC for swap");
        ui->mwcAmountEdit->setFocus();
        return;
    }

    if ( mwc >= spendable ) {
        control::MessageBox::messageText(this, "Incorrect Input", "Your selected account doesn't have enough coins");
        ui->mwcAmountEdit->setFocus();
        return;
    }

    if (!secOk || sec<secCurrencyLimit) {
        control::MessageBox::messageText(this, "Incorrect Input", "Please specify at least " + util::zeroDbl2Dbl(QString::number(secCurrencyLimit, 'f')) + " " + secCurrency +" for swap");
        ui->secAmountEdit->setFocus();
        return;
    }

    QString secAddress = ui->secAddressEdit->text().trimmed();
    if (state::getCurrencyInfo(secCurrency).is_btc_family) {
        if (secAddress.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", "Please specify a " + secCurrency + " address to redeem the coins");
            ui->secAddressEdit->setFocus();
            return;
        }
    } else {
        secAddress = "0x0000000000000000000000000000000000000000";
    }
    
    QString sendTo = ui->sendAddressEdit->text().trimmed();
    {
        QString valRes = util->validateMwc713Str(sendTo);
        if (!valRes.isEmpty()) {
            control::MessageBox::messageText(this, "Incorrect Input", valRes);
            ui->sendAddressEdit->setFocus();
            return;
        }
    }

    if (sendTo.size() == 0 ) {
        control::MessageBox::messageText(this, "Incorrect Input",
                                         "Please specify a valid MWC wallet address to send your MWC to." );
        ui->sendAddressEdit->setFocus();
        return;
    }

    swap->applyNewTrade1Params(account, secCurrency, mwcAmount, secAmount,
            secAddress, sendTo, ui->lockMwcFirstCheck->isChecked());

    ui->progress->show();
}

void NewSwap1::onSgnApplyNewTrade1Params(bool ok, QString errorMessage) {
    ui->progress->hide();

    if (ok)
        return;

    Q_ASSERT(!errorMessage.isEmpty());
    control::MessageBox::messageText(this, "Incorrect Input", errorMessage);
}


void NewSwap1::updateSecCurrencyData() {
    ui->secCurrencyCombo->clear();

    QVector<QString> curList = swap->secondaryCurrencyList();
    QString selectedCur = swap->getCurrentSecCurrency();
    int selectedIdx = -1;
    for ( int i=0; i<curList.size(); i++) {
        const auto & c = curList[i];
        ui->secCurrencyCombo->addItem( c, QVariant(c));
        if (c==selectedCur)
            selectedIdx = i;
    }
    ui->secCurrencyCombo->setCurrentIndex( selectedIdx );

    updateSecCurrencyStatus();

    if (!state::getCurrencyInfo(selectedCur).is_btc_family) {
        ui->secAddressEdit->setDisabled(true);
    }
}

void NewSwap1::updateSecCurrencyStatus() {
    QString selectedCur = swap->getCurrentSecCurrency();
    if (selectedCur.isEmpty())
        selectedCur = "XXX";

    ui->rateLabel->setText("MWC to " + selectedCur + " rate:");
    ui->secAddressEdit->setText("");
    QString addressPlaceholderText;
    if (selectedCur == "ZCash")
        addressPlaceholderText = "Transparent ZCash address to receive the coins";
    else
        addressPlaceholderText = selectedCur + " address to receive the coins";

    ui->secAddressEdit->setPlaceholderText(addressPlaceholderText);
    ui->receiveLabel->setText(selectedCur + " receiving address:");

    ui->secAmountEdit->setPlaceholderText(selectedCur + " amount");
}


void NewSwap1::onSgnWalletBalanceUpdated() {
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

void NewSwap1::on_accountComboBox_currentIndexChanged(int index) {
    if (index<0)
        return;

    QString account = ui->accountComboBox->currentData().toString();
    if (account.isEmpty())
        return;
    swap->setAccount(account);
}

double NewSwap1::getSelectedAccountBalance(QString account) {
    if (account.isEmpty())
        return 0.0;

    QVector<QString> accountInfo = wallet->getWalletBalance(true, true, false);
    for (int i = 1; i < accountInfo.size(); i += 2) {
        if (accountInfo[i - 1] == account) {
            // Val: default   Available: 2.48376 MWC
            QString amountStr =  accountInfo[i].trimmed();
            int idx1 = amountStr.lastIndexOf(':');
            int idx2 = amountStr.lastIndexOf(' ');
            if (idx1< idx2 && idx1>0) {
                amountStr = amountStr.mid(idx1+1, idx2-idx1-1).trimmed();
                return amountStr.toDouble();
            }
        }
    }
    return 0.0;
}


void NewSwap1::on_secCurrencyCombo_currentIndexChanged(int index) {
    if (index<0)
        return;

    QString selectedCurrency = ui->secCurrencyCombo->currentData().toString();
    swap->setCurrentSecCurrency(selectedCurrency);

    updateSecCurrencyStatus();
}

void NewSwap1::updateThirdValue() {

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

void NewSwap1::on_swapRateEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)
    thirdValueUpdate.removeAll(UPDATE_RATE);
    thirdValueUpdate.push_back(UPDATE_RATE);
    updateThirdValue();
}

void NewSwap1::on_mwcAmountEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)
    thirdValueUpdate.removeAll(UPDATE_MWC);
    thirdValueUpdate.push_back(UPDATE_MWC);
    updateThirdValue();
}

void NewSwap1::on_secAmountEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)
    thirdValueUpdate.removeAll(UPDATE_SEC);
    thirdValueUpdate.push_back(UPDATE_SEC);
    updateThirdValue();
}

void NewSwap1::on_sendAddressEdit_textEdited(const QString & arg1)
{
    Q_UNUSED(arg1)
    ui->contactNameLable->setText("");
    ui->contactNameLable->hide();
    ui->formatsLable->show();
}

}
