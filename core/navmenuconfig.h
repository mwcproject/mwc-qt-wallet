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

#ifndef NAVMENUCONFIG_H
#define NAVMENUCONFIG_H

#include <QWidget>
#include "navmenu.h"

namespace Ui {
class NavMenuConfigWallet;
class NavMenuConfigNode;
class NavMenuConfigColdWlt;
}


namespace state {
struct StateContext;
}

namespace core {

class AppContext;

class NavMenuConfig : public NavMenu {
Q_OBJECT
public:
    explicit NavMenuConfig(QWidget *parent, state::StateContext * context );

    ~NavMenuConfig();

private slots:
    void on_walletConfigButton_clicked();

    void on_outputsButton_clicked();

    void on_mwcmqButton_clicked();

    void on_resyncButton_clicked();

    void on_nodeOverviewButton_clicked();

    void on_selectRunningModeButton_clicked();

private:
    Ui::NavMenuConfigWallet *uiWallet = nullptr;
    Ui::NavMenuConfigNode   *uiNode = nullptr;
    Ui::NavMenuConfigColdWlt *uiColdWallet = nullptr;
    state::StateContext * context = nullptr;
};

}

#endif // NAVMENUCONFIG_H
