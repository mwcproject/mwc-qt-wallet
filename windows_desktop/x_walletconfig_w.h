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

#ifndef WALLETCONFIGW_H
#define WALLETCONFIGW_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class WalletConfig;
}

namespace bridge {
class WalletConfig;
class Util;
class Config;
}

namespace wnd {

class WalletConfig : public core::NavWnd
{
    Q_OBJECT

public:
    explicit WalletConfig(QWidget *parent);
    ~WalletConfig();

private slots:
    void on_confirmationNumberEdit_textChanged(const QString &arg1);
    void on_changeOutputsEdit_textEdited(const QString &arg1);

    void on_mwcmqHost_textEdited(const QString &arg1);
    void on_logsEnableBtn_clicked();

    void on_fontSz1_clicked();
    void on_fontSz2_clicked();
    void on_fontSz3_clicked();
    void on_fontSz4_clicked();
    void on_restoreDefault_clicked();
    void on_applyButton_clicked();
    void on_logout_3_clicked();
    void on_logout_5_clicked();
    void on_logout_10_clicked();
    void on_logout_20_clicked();
    void on_logout_30_clicked();
    void on_logout_never_clicked();
    void on_start_mqs_clicked();
    void on_outputLockingCheck_stateChanged(int arg1);
    void on_start_tor_clicked();
    void on_walletInstanceNameEdit_textChanged(const QString &arg1);
    void on_notificationsEnabled_clicked();
    void on_lockLaterEnabled_clicked();

private:
    void setValues(const QString & mwcmqHost,
                   int inputConfirmationNumber,
                   int changeOutputs);
    // return true if data is fine. In case of error will show message for the user
    //bool readInputValue( wallet::WalletConfig & newWalletConfig, core::SendCoinsParams & newSendParams );

    QString mwcDomainConfig2InputStr(QString mwcDomain);
    QString mwcDomainInputStr2Config(QString mwcDomain);

        // Update button state
    void updateButtons();

    void checkSizeButton(int szId); // Id match the control_desktop names: 1..4
    int getcheckedSizeButton() const;

    void updateLogsStateUI(bool enabled);

    void updateAutoStartStateUI(bool isAutoStartMQS, bool isAutoStartTor);

    void updateAutoLogoutStateUI(int64_t time);

    // return true if no chnages need to be made.
    // false - need to be made or was made and wallet need to be restarted
    bool applyChanges();

private:
    Ui::WalletConfig *ui;

    bridge::Config * config = nullptr;
    bridge::WalletConfig * walletConfig = nullptr;
    bridge::Util * util = nullptr;

    int uiScale = 2; // in the range [1..4]

    bool walletLogsEnabled = false;
    bool autoStartMQSEnabled = true;
    bool autoStartTorEnabled = true;
    bool outputLockingEnabled = false;
    bool notificationWindowsEnabled = true;
    bool lockLater = true;
    int64_t logoutTimeout = 20 * 60;
    int64_t currentLogoutTimeout = 20 * 60;

    QString mqsHost;
    int inputConfirmationsNumber = 1;
    int changeOutputs = 1;
    QString walletInstanceName;
};

}

#endif // WALLETCONFIGW_H
