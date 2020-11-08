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

#ifndef S_NEWSWAP2_W_H
#define S_NEWSWAP2_W_H

#include <control_desktop/MwcComboBox.h>
#include "../core_desktop/navwnd.h"

namespace Ui {
class NewSwap2;
}

namespace bridge {
class Swap;
class Util;
}

namespace wnd {

class NewSwap2 : public core::NavWnd {
Q_OBJECT

public:
    explicit NewSwap2(QWidget *parent);

    ~NewSwap2();

private slots:
    void onSgnApplyNewTrade2Params(bool ok, QString errorMessage);

    void on_offerEpirationHelpButton_clicked();
    void on_secRedeemHelpButton_clicked();
    void on_secTransFeeHelpButton_clicked();
    void on_electrumXHelpButton_clicked();
    void on_backButton_clicked();
    void on_reviewButton_clicked();
    void on_confNumberHelpButton_clicked();
    void on_secRedeemCombo_currentIndexChanged(int index);
    void on_offerExpirationCombo_currentIndexChanged(int index);
    void on_mwcBlocksEdit_textEdited(const QString &arg1);
    void on_secBlocksEdit_textEdited(const QString &arg1);

private:
    void initTimeIntervalCombo(control::MwcComboBox * timeIntervalCombo, int interval2select);

    void updateFundsLockTime();

private:
    Ui::NewSwap2 *ui;
    bridge::Swap * swap = nullptr;
    bridge::Util * util = nullptr;
};

}

#endif // S_NEWSWAP2_W_H
