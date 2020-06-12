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

#include "x_walletconfig_b.h"
#include "../BridgeManager.h"
#include "../../state/x_walletconfig.h"
#include "../../state/state.h"
#include "../../state/statemachine.h"
#include "../../wallet/wallet.h"
#include "../../util/execute.h"
#include "../../core/Config.h"
#include <QCoreApplication>

using namespace state;

namespace bridge {

static state::WalletConfig * getState() { return (state::WalletConfig *) getStateContext()->stateMachine->getState(STATE::WALLET_CONFIG); }
static wallet::Wallet * getWallet() { return getStateContext()->wallet; }

WalletConfig::WalletConfig(QObject *parent) : QObject(parent) {
    getBridgeManager()->addWalletConfig(this);
}

WalletConfig::~WalletConfig() {
    getBridgeManager()->removeWalletConfig(this);
}

// Update data path and nwtwork for the current wallet.
bool WalletConfig::setDataPathWithNetwork(QString dataPath, QString network, bool guiWalletRestartExpected) {
    auto walletConfig = getStateContext()->wallet->getWalletConfig();
    walletConfig.setDataPathWithNetwork( dataPath, network );
    return getState()->setWalletConfig(walletConfig, guiWalletRestartExpected);
}

// Change network and restart qt wallet. Used by Online Node
void WalletConfig::restartQtWallet() {
    util::requestRestartMwcQtWallet();
    QCoreApplication::quit();
}

void WalletConfig::canApplySettings(bool lock) {
    return getState()->canApplySettings(lock);
}

// read current settings
double WalletConfig::getGuiScale() {
    return getState()->getGuiScale();
}

bool WalletConfig::getWalletLogsEnabled() {
    return getState()->getWalletLogsEnabled();
}

bool WalletConfig::getAutoStartMQSEnabled() {
    return getState()->getAutoStartMQSEnabled();
}

bool WalletConfig::getAutoStartKeybaseEnabled() {
    return getState()->getAutoStartKeybaseEnabled();
}

bool WalletConfig::getAutoStartTorEnabled() {
    return getState()->getAutoStartTorEnabled();
}

int WalletConfig::getLogoutTimeMs() {
    return config::getLogoutTimeMs();
}

bool WalletConfig::isOutputLockingEnabled() {
    return getState()->isOutputLockingEnabled();
}

QString WalletConfig::getDataPath() {
    return getWallet()->getWalletConfig().getDataPath();
}
QString WalletConfig::getKeybasePath() {
    return getWallet()->getWalletConfig().keyBasePath;
}
QString WalletConfig::getMqsHost() {
    return getWallet()->getWalletConfig().mwcmqsDomainEx;
}
int WalletConfig::getInputConfirmationsNumber() {
    return getState()->getSendCoinsParams().inputConfirmationNumber;
}
int WalletConfig::getChangeOutputs()  {
    return getState()->getSendCoinsParams().changeOutputs;
}
double WalletConfig::getInitGuiScale() {
    return getState()->getInitGuiScale();
}
QString WalletConfig::getDefaultKeybasePath() {
    return getWallet()->getDefaultConfig().keyBasePath;
}

QString WalletConfig::getDefaultMqsHost() {
    return getWallet()->getWalletConfig().mwcmqsDomainEx;
}
int WalletConfig::getDefaultInputConfirmationsNumber() {
    return core::SendCoinsParams().inputConfirmationNumber;
}
int WalletConfig::getDefaultChangeOutputs() {
    return core::SendCoinsParams().changeOutputs;
}

// Update settings
void WalletConfig::setSendCoinsParams(int inputConfirmationsNumber, int changeOutputs) {
    getState()->setSendCoinsParams(core::SendCoinsParams(inputConfirmationsNumber, changeOutputs));
}
void WalletConfig::updateWalletLogsEnabled(bool logsEnabled, bool needCleanupLogs) {
    getState()->updateWalletLogsEnabled(logsEnabled, needCleanupLogs);
}
void WalletConfig::updateGuiScale(double scale) {
    getState()->updateGuiScale(scale);
}
void WalletConfig::updateAutoStartMQSEnabled(bool enabled) {
    getState()->updateAutoStartMQSEnabled(enabled);
}
void WalletConfig::updateAutoStartKeybaseEnabled(bool enabled) {
    getState()->updateAutoStartKeybaseEnabled(enabled);
}
void WalletConfig::updateAutoStartTorEnabled(bool enabled) {
    getState()->updateAutoStartTorEnabled(enabled);
}

void WalletConfig::setOutputLockingEnabled(bool enabled) {
    getState()->setOutputLockingEnabled(enabled);
}

bool WalletConfig::updateTimeoutValue(int timeout) {
    return getState()->updateTimeoutValue(timeout);
}

bool WalletConfig::updateWalletConfig(QString network, QString dataPath,
                                    QString mwcmqsDomain, QString keyBasePath, bool need2updateGuiSize) {
    auto config = getWallet()->getWalletConfig();
    config.setDataWalletCfg(network, dataPath, mwcmqsDomain, keyBasePath);
    return getState()->setWalletConfig(config, need2updateGuiSize);
}


}