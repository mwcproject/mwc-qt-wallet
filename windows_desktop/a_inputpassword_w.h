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

#ifndef INPUTPASSWORDW_H
#define INPUTPASSWORDW_H

#include "../core_desktop/PanelBaseWnd.h"

namespace Ui {
class InputPassword;
}

namespace bridge {
class Config;
class InputPassword;
class Wallet;
class StartWallet;
}

namespace wnd {

class InputPassword : public core::PanelBaseWnd
{
    Q_OBJECT
public:
    explicit InputPassword(QWidget *parent, bool lockMode);
    ~InputPassword();

private slots:
    void onSgnLoginResult(bool ok);
    void onSgnUpdateListenerStatus(bool mwcOnline, bool keybaseOnline, bool tor);
    void onSgnHttpListeningStatus(bool listening, QString additionalInfo);
    void onSgnUpdateSyncProgress(double progressPercent);

    void on_submitButton_clicked();
    void on_restoreInstanceButton_clicked();
    void on_newInstanceButton_clicked();
    void on_openWalletButton_clicked();

private:
    virtual void panelWndStarted() override;

    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);
    void updateTorState(bool online);
    void updateHttpState(bool online);

private:
    Ui::InputPassword *ui;
    bridge::Config * config = nullptr;
    bridge::InputPassword * inputPassword = nullptr;
    bridge::StartWallet * startWallet = nullptr;
    bridge::Wallet * wallet = nullptr;
};

}

#endif // INPUTPASSWORDW_H
