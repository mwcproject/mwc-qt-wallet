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
    Q_INVOKABLE int  getLogoutTimeMs();
    Q_INVOKABLE bool isOutputLockingEnabled();
    Q_INVOKABLE bool getNotificationWindowsEnabled();

    Q_INVOKABLE QString getDataPath();
    Q_INVOKABLE int getInputConfirmationsNumber();
    Q_INVOKABLE int getChangeOutputs();

    Q_INVOKABLE double getInitGuiScale();
    Q_INVOKABLE int getDefaultInputConfirmationsNumber();
    Q_INVOKABLE int getDefaultChangeOutputs();

    // Update settings
    Q_INVOKABLE void setSendCoinsParams(int inputConfirmationsNumber, int changeOutputs);
    Q_INVOKABLE void updateWalletLogsEnabled(bool logsEnabled, bool needCleanupLogs);
    Q_INVOKABLE void updateGuiScale(double scale);
    Q_INVOKABLE void setOutputLockingEnabled(bool enabled);
    Q_INVOKABLE void setNotificationWindowsEnabled(bool enabled);

    Q_INVOKABLE bool updateTimeoutValue(int timeout);

    Q_INVOKABLE bool isFeatureSlatepack();
    Q_INVOKABLE bool isFeatureMWCMQS();
    Q_INVOKABLE bool isFeatureTor();
    Q_INVOKABLE void setFeatureSlatepack(bool val);
    Q_INVOKABLE void setFeatureMWCMQS(bool val);
    Q_INVOKABLE void setFeatureTor(bool val);
    Q_INVOKABLE bool isDefaultFeatureSlatepack();
    Q_INVOKABLE bool isDefaultFeatureMWCMQS();
    Q_INVOKABLE bool isDefaultFeatureTor();
public:
    void emitWalletFeaturesChanged();
signals:
    void sgnWalletFeaturesChanged();

private:
    void sendEmitWalletFeaturesChanged();
};

}

#endif //MWC_QT_WALLET_X_WALLETCONFIG_B_H
