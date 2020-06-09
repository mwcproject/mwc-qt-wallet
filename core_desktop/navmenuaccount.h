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

#ifndef NAVMENYACCOUNT_H
#define NAVMENYACCOUNT_H

#include <QWidget>
#include "navmenu.h"

namespace Ui {
class NavMenuAccount;
}

namespace bridge {
class StateMachine;
class Wallet;
}

namespace core {

class NavMenuAccount : public NavMenu
{
    Q_OBJECT

public:
    explicit NavMenuAccount(QWidget *parent);
    ~NavMenuAccount();

private slots:
    void on_accountsButton_clicked();
    void on_seedButton_clicked();
    void on_contactsButton_clicked();
    void on_logoutButton_clicked();

private:
    Ui::NavMenuAccount *ui;
    bridge::StateMachine * stateMachine = nullptr;
    bridge::Wallet * wallet = nullptr;
};

}

#endif // NAVMENYACCOUNT_H
