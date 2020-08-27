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

#ifndef InitAccountW_H
#define InitAccountW_H

#include "../core_desktop/PanelBaseWnd.h"

namespace Ui {
class InitAccount;
}

namespace bridge {
class SelectMode;
class InitAccount;
class Util;
class StartWallet;
}

namespace wnd {

class InitAccount : public core::PanelBaseWnd
{
    Q_OBJECT

public:
    explicit InitAccount(QWidget *parent, QString path, bool restoredFromSeed);
    virtual ~InitAccount() override;

private slots:
    void on_password1Edit_textChanged(const QString &text);
    void on_submitButton_clicked();

    void on_password2Edit_textChanged(const QString &arg1);
    void on_runOnlineNodeButton_clicked();
    void on_changeDirButton_clicked();

private:
    void updatePassState();

    virtual void panelWndStarted() override;

private:
    Ui::InitAccount *ui;
    bridge::SelectMode * selectMode = nullptr;
    bridge::InitAccount * initAccount = nullptr;
    bridge::Util * util = nullptr;
    bridge::StartWallet * startWallet = nullptr;

    bool restoredFromSeed = false;
};

}

#endif // InitAccountW_H
