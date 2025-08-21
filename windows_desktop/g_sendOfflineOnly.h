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

#ifndef SEND3_OFFLINEONLY_H
#define SEND3_OFFLINEONLY_H

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOfflineOnly;
}

namespace bridge {
class Wallet;
class Util;
class Config;
class Send;
}

namespace wnd {

class SendOfflineOnly : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOfflineOnly(QWidget *parent);
    virtual ~SendOfflineOnly() override;

private slots:
    void on_allAmountButton_clicked();
    void on_accountComboBox_currentIndexChanged(int index);

    void onSgnWalletBalanceUpdated();
    void on_generatePoof_clicked(bool checked);

    void on_sendButton_clicked();
    void on_settingsBtn_clicked();
    void on_contactsButton_clicked();

    void onSgnShowSendResult( bool success, QString message );
private:
    Ui::SendOfflineOnly * ui;
    bridge::Wallet * wallet = nullptr;
    bridge::Util * util = nullptr;
    bridge::Config * config = nullptr;
    bridge::Send * send = nullptr;
};

}

#endif // SEND3_OFFLINEONLY_H
