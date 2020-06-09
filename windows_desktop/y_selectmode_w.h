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

#ifndef Y_SELECTMODE_W_H
#define Y_SELECTMODE_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class SelectMode;
}

namespace bridge {
class SelectMode;
}

namespace wnd {

class SelectMode : public core::NavWnd {
Q_OBJECT

public:
    explicit SelectMode(QWidget *parent);

    ~SelectMode();

private slots:

    void on_applyButton_clicked();
    void on_radioOnlineWallet_clicked();
    void on_radioOnlineNode_clicked();
    void on_radioColdWallet_clicked();

private:
    int getRunMode() const; // return config::WALLET_RUN_MODE
    void updateButtons();
private:
    Ui::SelectMode *ui;
    bridge::SelectMode * selectMode = nullptr;

    int runMode = -1; // values from: config::WALLET_RUN_MODE
};

}

#endif // Y_SELECTMODE_W_H
