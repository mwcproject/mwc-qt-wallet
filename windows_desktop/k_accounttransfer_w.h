// Copyright 2019 The MWC Developers
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

#ifndef K_ACCOUNTTRANSFER_W_H
#define K_ACCOUNTTRANSFER_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class AccountTransfer;
}

namespace control {
class MwcComboBox;
}

namespace bridge {
class Wallet;
class Config;
class AccountTransfer;
}

namespace wnd {

class AccountTransfer : public core::NavWnd
{
    Q_OBJECT

public:
    explicit AccountTransfer(QWidget *parent);
    ~AccountTransfer();

protected:
    // return empty String if account not selected
    QString getSelectedAccount( control::MwcComboBox * combo, bool showInputErrMessage );

private slots:
    void on_allAmountButton_clicked();
    void on_settingsBtn_clicked();
    void on_transferButton_clicked();
    void on_backButton_clicked();

    void onSgnShowTransferResults(bool ok, QString errMsg);
    void onSgnUpdateAccounts();
    void onSgnHideProgress();
private:
    Ui::AccountTransfer *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
    bridge::AccountTransfer * accountTransfer = nullptr;
};

}

#endif // K_ACCOUNTTRANSFER_W_H
