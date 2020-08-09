// Copyright 2020 The MWC Developers
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


#ifndef A_INITFIRSTTIME_W_H
#define A_INITFIRSTTIME_W_H

#include "../core_desktop/PanelBaseWnd.h"

namespace Ui {
class InitFirstTime;
}

namespace bridge {
class StartWallet;
}

namespace wnd {

class InitFirstTime : public core::PanelBaseWnd
{
    Q_OBJECT

public:
    explicit InitFirstTime(QWidget *parent = nullptr);
    ~InitFirstTime();

private slots:
    void on_newInstanceButton_clicked();

    void on_restoreInstanceButton_clicked();

    void on_runOnlineNodeButton_clicked();

private:
    Ui::InitFirstTime *ui;
    bridge::StartWallet * startWallet = nullptr;
};

}

#endif // A_INITFIRSTTIME_W_H
