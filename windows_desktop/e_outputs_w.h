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

#ifndef OUTPUTSW_H
#define OUTPUTSW_H

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Outputs;
}

namespace bridge {
class Config;
class HodlStatus;
class Wallet;
class Outputs;
}

namespace wnd {

class Outputs : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Outputs(QWidget *parent);
    ~Outputs();

private slots:
    void on_accountComboBox_activated(int index);
    void on_prevBtn_clicked();
    void on_nextBtn_clicked();
    void on_refreshButton_clicked();
    void on_showAll_clicked();
    void on_showUnspent_clicked();
    void on_outputsTable_cellDoubleClicked(int row, int column);
    void on_outputsTable_cellClicked(int row, int column);

    void saveOutputNote( QString commitment, QString note);

    void onSgnWalletBalanceUpdated();
    void onSgnOutputs( QString account, bool showSpent, QString height, QVector<QString> outputs);

    void onSgnNewNotificationMessage(int level, QString message);

private:
    virtual void panelWndStarted() override;

    void initTableHeaders();
    void saveTableHeaders();

    void requestOutputs(QString account);

    QString currentSelectedAccount();

    // return enable state for the buttons
    void updateShownData();

    // return selected account
    QString updateAccountsData();

    int calcPageSize() const;

    bool isShowUnspent() const;

    wallet::WalletOutput * getSelectedOutput();

    void showLockedState(int row, const wallet::WalletOutput & output);

    // return true if user fine with lock changes
    bool showLockMessage();
private:
    Ui::Outputs *ui;
    bridge::Config * config = nullptr;
    bridge::HodlStatus * hodlStatus = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Outputs * outputs = nullptr; // needed as output windows active flag

    QVector<wallet::WalletOutput> allData; // all outputs
    QVector<wallet::WalletOutput> shownData; // Outputs that we show

    int currentPagePosition = INT_MAX; // position at the paging...

    //QPair<bool,bool> buttonState = QPair<bool,bool>(false, false);

    bool inHodl = false; // If acount enrolled in HODL. Requested once to eliminate race conditions
    bool canLockOutputs = false;
    QString tableId;

    static bool lockMessageWasShown;
};

}

#endif // OUTPUTS_H
