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

#ifndef S_MRKTSWAPNEW_W_H
#define S_MRKTSWAPNEW_W_H

#include "../core_desktop/navwnd.h"
#include "../state/s_mktswap.h"

namespace Ui {
class MrktSwapNew;
}

namespace bridge {
class Config;
class SwapMarketplace;
class Wallet;
class Util;
class Swap;
}

namespace wnd {

class MrktSwapNew : public core::NavWnd {
Q_OBJECT
public:
    // Create new offer (empty myMsgId) of view exist one.
    explicit MrktSwapNew(QWidget *parent, QString myMsgId);
    ~MrktSwapNew();

private:
    void updateSecCurrencyData();
    void updateSecCurrencyStatus(bool seller);

    void updateThirdValue();
    void updateFundsLockTime();

private slots:
    void on_buySellCombo_currentIndexChanged(int index);
    void on_mwcAmountEdit_textChanged(const QString &arg1);
    void on_secCurrencyCombo_currentIndexChanged(int index);
    void on_mwcAmountEdit_textEdited(const QString &arg1);
    void on_secAmountEdit_textEdited(const QString &arg1);
    void on_swapRateEdit_textEdited(const QString &arg1);
    void on_swapRateHelpBtn_clicked();
    void on_secAddressHelpBtn_clicked();
    void on_secTransFeeHelpButton_clicked();
    void on_mwcBlocksEdit_textEdited(const QString &arg1);
    void on_secBlocksEdit_textEdited(const QString &arg1);
    void on_submitButton_clicked();
    void on_cancelButton_clicked();
    void on_secTransFeeEdit_textEdited(const QString &arg1);
    void on_secAddressEdit_textEdited(const QString &arg1);

    void onSgnWalletBalanceUpdated();
    void onSgnApplyNewTrade1Params(bool ok, QString errorMessage);

    void on_confNumberHelpButton_clicked();

private:
    Ui::MrktSwapNew *ui;
    bridge::Config * config = nullptr;
    bridge::SwapMarketplace * swapMarketplace = nullptr;
    bridge::Swap * swap = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Util * util = nullptr;

    state::MySwapOffer offer;
    QVector<int> thirdValueUpdate;
    QString secAddressCoin; // For what coin we enter address
};

}

#endif // S_MRKTSWAPNEW_W_H
