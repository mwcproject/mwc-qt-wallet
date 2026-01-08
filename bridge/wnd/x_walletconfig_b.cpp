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
#include "../../util/Log.h"
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

// Specify if the Online Node will run as a mainnet.
void WalletConfig::setOnlineNodeRunsMainNetwork(bool isMainNet) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setOnlineNodeRunsMainNetwork with isMainNet=" + QString(isMainNet ? "true" : "false"));
    getStateContext()->appContext->setOnlineNodeRunsMainNetwork(isMainNet);
}

// Change network and restart qt wallet. Used by Online Node
void WalletConfig::restartQtWallet() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::restartQtWallet");
    util::requestRestartMwcQtWallet();
    QCoreApplication::quit();
}

void WalletConfig::canApplySettings(bool lock) {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::canApplySettings with lock=" + QString(lock ? "true" : "false"));
    return getState()->canApplySettings(lock);
}

// read current settings
double WalletConfig::getGuiScale() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getGuiScale");
    return getState()->getGuiScale();
}

bool WalletConfig::getWalletLogsEnabled() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getWalletLogsEnabled");
    return getState()->getWalletLogsEnabled();
}

int WalletConfig::getLogoutTimeMs() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getLogoutTimeMs");
    return config::getLogoutTimeMs();
}

bool WalletConfig::isOutputLockingEnabled() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::isOutputLockingEnabled");
    return getState()->isOutputLockingEnabled();
}

bool WalletConfig::getNotificationWindowsEnabled() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getNotificationWindowsEnabled");
    return getState()->getNotificationWindowsEnabled();
}

QString WalletConfig::getDataPath() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getDataPath");
    return getWallet()->getWalletConfig().getDataPath();
}
int WalletConfig::getInputConfirmationsNumber() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getInputConfirmationsNumber");
    return getState()->getSendCoinsParams().inputConfirmationNumber;
}
int WalletConfig::getChangeOutputs()  {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getChangeOutputs");
    return getState()->getSendCoinsParams().changeOutputs;
}
double WalletConfig::getInitGuiScale() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getInitGuiScale");
    return getState()->getInitGuiScale();
}

int WalletConfig::getDefaultInputConfirmationsNumber() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getDefaultInputConfirmationsNumber");
    return core::SendCoinsParams().inputConfirmationNumber;
}
int WalletConfig::getDefaultChangeOutputs() {
    //logger::logInfo(logger::BRIDGE, "Call WalletConfig::getDefaultChangeOutputs");
    return core::SendCoinsParams().changeOutputs;
}

// Update settings
void WalletConfig::setSendCoinsParams(int inputConfirmationsNumber, int changeOutputs) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setSendCoinsParams with inputConfirmationsNumber=" + QString::number(inputConfirmationsNumber) + " changeOutputs=" + QString::number(changeOutputs));
    getState()->setSendCoinsParams(core::SendCoinsParams(inputConfirmationsNumber, changeOutputs));
}
void WalletConfig::updateWalletLogsEnabled(bool logsEnabled, bool needCleanupLogs) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::updateWalletLogsEnabled with logsEnabled=" + QString(logsEnabled ? "true" : "false") + " needCleanupLogs=" + QString(needCleanupLogs ? "true" : "false"));
    getState()->updateWalletLogsEnabled(logsEnabled, needCleanupLogs);
}
void WalletConfig::updateGuiScale(double scale) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::updateGuiScale with scale=" + QString::number(scale));
    getState()->updateGuiScale(scale);
}
void WalletConfig::setOutputLockingEnabled(bool enabled) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setOutputLockingEnabled with enabled=" + QString(enabled ? "true" : "false"));
    getState()->setOutputLockingEnabled(enabled);
}

void WalletConfig::setNotificationWindowsEnabled(bool enabled) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setNotificationWindowsEnabled with enabled=" + QString(enabled ? "true" : "false"));
    getState()->setNotificationWindowsEnabled(enabled);
}

bool WalletConfig::updateTimeoutValue(int timeout) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::updateTimeoutValue with timeout=" + QString::number(timeout));
    return getState()->updateTimeoutValue(timeout);
}

bool WalletConfig::isFeatureSlatepack() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::isFeatureSlatepack");
    return getStateContext()->appContext->isFeatureSlatepack();
}
bool WalletConfig::isFeatureMWCMQS() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::isFeatureMWCMQS");
    return getStateContext()->appContext->isFeatureMWCMQS();
}

bool WalletConfig::isFeatureTor() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::isFeatureTor");
    return getStateContext()->appContext->isFeatureTor();
}

void WalletConfig::setFeatureSlatepack(bool val) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setFeatureSlatepack with val=" + QString(val ? "true" : "false"));
    bool changed = (val != isFeatureSlatepack());
    getStateContext()->appContext->setFeatureSlatepack(val);
    if (changed)
        sendEmitWalletFeaturesChanged();
}
void WalletConfig::setFeatureMWCMQS(bool val) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setFeatureMWCMQS with val=" + QString(val ? "true" : "false"));
    bool changed = (val != isFeatureMWCMQS());
    wallet::ListenerStatus status = getWallet()->getListenerStatus();
    if (val) {
        if (!status.mqs_started)
            getWallet()->listeningStart(true, false);
    }
    else {
        if (status.mqs_started) {
            getWallet()->listeningStop(true, false);
        }
    }
    getStateContext()->appContext->setFeatureMWCMQS(val);
    if (changed)
        sendEmitWalletFeaturesChanged();
}
void WalletConfig::setFeatureTor(bool val) {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::setFeatureTor with val=" + QString(val ? "true" : "false"));
    bool changed = (val != isFeatureTor());
    wallet::ListenerStatus status = getWallet()->getListenerStatus();
    if (val) {
        if (!status.tor_started)
            getWallet()->listeningStart(false, true);
    }
    else {
        if (status.tor_started) {
            getWallet()->listeningStop(false, true);
        }
    }
    getStateContext()->appContext->setFeatureTor(val);
    if (changed)
        sendEmitWalletFeaturesChanged();
}

bool WalletConfig::isDefaultFeatureSlatepack() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::isDefaultFeatureSlatepack");
    return true;
}
bool WalletConfig::isDefaultFeatureMWCMQS() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::isDefaultFeatureMWCMQS");
    return false;
}
bool WalletConfig::isDefaultFeatureTor() {
    logger::logInfo(logger::BRIDGE, "Call WalletConfig::isDefaultFeatureTor");
    return false;
}

void WalletConfig::sendEmitWalletFeaturesChanged() {
    for ( WalletConfig* cfg : getBridgeManager()->getWalletConfig() ) {
        cfg->emitWalletFeaturesChanged();
    }
}


void WalletConfig::emitWalletFeaturesChanged() {
    emit sgnWalletFeaturesChanged();
}

}
