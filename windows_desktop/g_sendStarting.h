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

#ifndef SEND1_ONLINEOFFLINE_W_H
#define SEND1_ONLINEOFFLINE_W_H

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace Ui {
class SendStarting;
}

namespace bridge {
class Wallet;
class Config;
class Send;
}

namespace wnd {

class SendStarting : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendStarting(QWidget *parent);
    ~SendStarting();

private slots:
    void onChecked(int id);
    void on_nextButton_clicked();
    void on_allAmountButton_clicked();
    void on_accountComboBox_currentIndexChanged(int index);

    void onSgnWalletBalanceUpdated();
private:
    Ui::SendStarting *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
    bridge::Send * send = nullptr;
};


}

#endif // SEND1_ONLINEOFFLINE_W_H
