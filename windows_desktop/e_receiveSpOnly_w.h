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

#ifndef E_RECEIVE_SP_ONLY_W_H
#define E_RECEIVE_SP_ONLY_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class ReceiveSpOnly;
}

namespace bridge {
class Config;
class Wallet;
class Receive;
class Util;
}

namespace wnd {

class ReceiveSpOnly : public core::NavWnd {
Q_OBJECT

public:
    explicit ReceiveSpOnly(QWidget *parent);

    virtual ~ReceiveSpOnly() override ;

private:
    void onTransactionActionIsFinished( bool success, QString message );

    void updateWalletBalance();
private slots:
    // respond from signTransaction
    void onSgnTransactionActionIsFinished( bool success, QString message );
    void onSgnWalletBalanceUpdated();

    void on_accountComboBox_activated(int index);

    void on_slatepackEdit_textChanged();
    void on_continueButton_clicked();

private:
    void updateButtons();
    void initiateSlateVerification(const QString &slate2check);
private:
    Ui::ReceiveSpOnly *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Receive * receive = nullptr;
    bridge::Util * util = nullptr;

    QString slatepack; // The content
    QString slateJson;
    QString sender;
    bool isSpValid = false;
};

}

#endif // E_RECEIVE_SP_ONLY_W_H
