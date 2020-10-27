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

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class Accounts;
}

namespace bridge {
class Accounts;
class Config;
class Wallet;
class Util;
}

class QTableWidgetItem;

namespace wnd {

class Accounts : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Accounts(QWidget *parent);
    ~Accounts();

private slots:
    void on_refreshButton_clicked();
    void on_transferButton_clicked();
    void on_addButton_clicked();
    void on_deleteButton_clicked();
    void on_renameButton_clicked();
    void on_accountList_itemSelectionChanged();
    void on_accountList_itemDoubleClicked(QTableWidgetItem *item);

    void onSgnWalletBalanceUpdated();
    void onSgnAccountCreated( QString newAccountName);
    // Account is renamed, renameAccount
    void onSgnAccountRenamed(bool success, QString errorMessage);

private:
    void initTableHeaders();
    void saveTableHeaders();

    void renameAccount(int idx);

    void updateButtons();
    void startWaiting();
private:
    Ui::Accounts *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Accounts * accState = nullptr; // place holder
    bridge::Util * util = nullptr;

    QVector<QString> accounts; // current shown data
};

}

#endif // ACCOUNTS_H
