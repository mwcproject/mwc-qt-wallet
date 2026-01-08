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

#ifndef E_RECEIVE_W_H
#define E_RECEIVE_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class Receive;
}

namespace bridge {
class Config;
class Wallet;
class Receive;
class Util;
class WalletConfig;
class HeartBeat;
}

namespace wnd {

class Receive : public core::NavWnd {
Q_OBJECT

public:
    explicit Receive(QWidget *parent);

    virtual ~Receive() override ;

private:
    void updateListenerStatus();
    void updateListenerStatusWith(bool mqsOnline, bool torOnline);

    void updateWalletBalance();
private slots:
    // respond from signTransaction
    void onSgnTransactionActionIsFinished( bool success, QString message );
    void onSgnWalletBalanceUpdated();
    void onSgnUpdateListenerStatus(bool mqsOnline, bool torOnline);

    void on_accountComboBox_activated(int index);
    void on_recieveSlatepackButton_clicked();

    void on_requestFaucetMWCButton_clicked();

private:

private:
    Ui::Receive *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::WalletConfig * walletConfig = nullptr;
    bridge::Receive * receive = nullptr;
    bridge::Util * util = nullptr;
    bridge::HeartBeat * heartBeat = nullptr;

    QString mwcAddress;
};

}

#endif // E_RECEIVE_W_H
