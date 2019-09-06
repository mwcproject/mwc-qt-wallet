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

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Receive;
}

namespace state {
class Receive;
}

namespace wnd {

class Receive : public core::NavWnd {
Q_OBJECT

public:
    explicit Receive(QWidget *parent, state::Receive * state, bool mwcMqStatus, bool keybaseStatus,
                     QString mwcMqAddress, const wallet::WalletConfig & walletConfig);

    virtual ~Receive() override ;

    void updateMwcMqAddress(QString address);
    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);

    void onTransactionActionIsFinished( bool success, QString message );
    void stopWaiting();

    void updateWalletBalance();
private slots:
    void on_accountComboBox_activated(int index);
    void on_recieveFileButton_clicked();
private:
    void updateAccountList();

    virtual void timerEvent(QTimerEvent *event) override;

private:
    Ui::Receive *ui;
    state::Receive * state;
    wallet::WalletConfig walletConfig;
    QVector<wallet::AccountInfo> accountInfo;
    QString mwcAddress;
};

}

#endif // E_RECEIVE_W_H
