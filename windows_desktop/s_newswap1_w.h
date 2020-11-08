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

#ifndef S_NEWSWAP1_W_H
#define S_NEWSWAP1_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class NewSwap1;
}

namespace bridge {
class Swap;
class Wallet;
class Util;
}

namespace wnd {

class NewSwap1 : public core::NavWnd {
Q_OBJECT

public:
    explicit NewSwap1(QWidget *parent);
    ~NewSwap1();

private:
    void updateSecCurrencyData();
    void updateSecCurrencyStatus();
    void updateRateValue();
    void updateSecValue();

    double getSelectedAccountBalance(QString account);
private slots:
    void on_contactsButton_clicked();
    void on_cancelButton_clicked();
    void on_nextButton_clicked();

    void onSgnWalletBalanceUpdated();
    void onSgnApplyNewTrade1Params(bool ok, QString errorMessage);

    void on_accountComboBox_currentIndexChanged(int index);
    void on_secCurrencyCombo_currentIndexChanged(int index);
    void on_swapRateEdit_textEdited(const QString &arg1);
    void on_mwcAmountEdit_textEdited(const QString &arg1);
    void on_secAmountEdit_textEdited(const QString &arg1);
    void on_sendAddressEdit_textEdited(const QString &arg1);

private:
    Ui::NewSwap1 *ui;
    bridge::Swap * swap = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Util * util = nullptr;
};

}

#endif // S_NEWSWAP1_W_H
