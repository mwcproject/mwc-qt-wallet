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

#ifndef S_MKTINTTX_W_H
#define S_MKTINTTX_W_H

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"
#include "../control_desktop/richbutton.h"

namespace Ui {
class IntegrityTransactions;
}

namespace bridge {
class Config;
class Wallet;
class Util;
class SwapMarketplace;
}

namespace wnd {

class IntegrityTransactions : public core::NavWnd {
Q_OBJECT

public:
    explicit IntegrityTransactions(QWidget *parent = nullptr);
    ~IntegrityTransactions();

private:
    void requestTransactions();
    void updateData();

private slots:
    void on_backButton_clicked();

    void onSgnTransactions( QString account, QString height, QVector<QString> transactions);

    void onSgnNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, QString totalDifficulty, int connections );
    void onSgnNewNotificationMessage(int level, QString message); // level: bridge::MESSAGE_LEVEL values
private:
    Ui::IntegrityTransactions *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::SwapMarketplace * swapMarketplace = nullptr; // just a placeholder to signal that this window is online
    bridge::Util * util = nullptr;

    QVector<wallet::WalletTransaction> allTrans;
    int64_t nodeHeight    = 0;
};

}

#endif // S_MKTINTTX_W_H
