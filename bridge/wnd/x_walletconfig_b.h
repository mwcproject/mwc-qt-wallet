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

#ifndef MWC_QT_WALLET_X_WALLETCONFIG_B_H
#define MWC_QT_WALLET_X_WALLETCONFIG_B_H

#include <QObject>

namespace bridge {

class WalletConfig : public QObject {
Q_OBJECT
public:
    explicit WalletConfig(QObject * parent = nullptr);
    ~WalletConfig();

    // Specify if the Online Node will run as a mainnet.
    Q_INVOKABLE void setOnlineNodeRunsMainNetwork(bool isMainNet);

    // Request restart for qt wallet
    Q_INVOKABLE void restartQtWallet();

    Q_INVOKABLE void canApplySettings(bool lock);

    // read current settings
    Q_INVOKABLE double getGuiScale();
    Q_INVOKABLE bool getWalletLogsEnabled();
    Q_INVOKABLE bool getAutoStartMQSEnabled();
    Q_INVOKABLE bool getAutoStartKeybaseEnabled(); // Absolete
    Q_INVOKABLE bool getAutoStartTorEnabled();
    Q_INVOKABLE int getLogoutTimeMs();
    Q_INVOKABLE bool isOutputLockingEnabled();
    Q_INVOKABLE bool getNotificationWindowsEnabled();

    Q_INVOKABLE QString getDataPath();
    Q_INVOKABLE QString getKeybasePath(); // Absolete
    Q_INVOKABLE QString getMqsHost();
    Q_INVOKABLE int getInputConfirmationsNumber();
    Q_INVOKABLE int getChangeOutputs();

    Q_INVOKABLE double getInitGuiScale();
    Q_INVOKABLE QString getDefaultKeybasePath(); // Absolete
    Q_INVOKABLE QString getDefaultMqsHost();
    Q_INVOKABLE int getDefaultInputConfirmationsNumber();
    Q_INVOKABLE int getDefaultChangeOutputs();

    // Update settings
    Q_INVOKABLE void setSendCoinsParams(int inputConfirmationsNumber, int changeOutputs);
    Q_INVOKABLE void updateWalletLogsEnabled(bool logsEnabled, bool needCleanupLogs);
    Q_INVOKABLE void updateGuiScale(double scale);
    Q_INVOKABLE void updateAutoStartMQSEnabled(bool enabled);
    Q_INVOKABLE void updateAutoStartKeybaseEnabled(bool enabled);
    Q_INVOKABLE void updateAutoStartTorEnabled(bool enabled);
    Q_INVOKABLE void setOutputLockingEnabled(bool enabled);
    Q_INVOKABLE void setNotificationWindowsEnabled(bool enabled);

    Q_INVOKABLE bool updateTimeoutValue(int timeout);

    // keyBasePath is absolete
    Q_INVOKABLE bool updateWalletConfig( QString mwcmqsDomain, QString _keyBasePath, bool need2updateGuiSize );

};

}

#endif //MWC_QT_WALLET_X_WALLETCONFIG_B_H
