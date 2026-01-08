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

#include "x_walletconfig.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../util/execute.h"
#include "../util/Log.h"
#include "../util/ConfigReader.h"
#include <QCoreApplication>
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/x_walletconfig_b.h"
#include "../bridge/corewindow_b.h"

namespace state {

WalletConfig::WalletConfig( StateContext * _context) :
    State (_context, STATE::WALLET_CONFIG) {
}

WalletConfig::~WalletConfig() {}

NextStateRespond WalletConfig::execute() {
    if (context->appContext->getActiveWndState() != STATE::WALLET_CONFIG)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    settingLock = false;
    if ( config::isOnlineNode() ) {
        core::getWndManager()->pageNodeConfig();
    }
    else {
        core::getWndManager()->pageWalletConfig();
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// State can block the state change. Wallet config is the first usage.
bool WalletConfig::canExitState(STATE nextWindowState) {
    logger::logInfo(logger::STATE, "Call WalletConfig::canExitState with nextWindowState=" + QString::number(static_cast<int>(nextWindowState)));
    if (nextWindowState == WALLET_CONFIG)
        return false; // Config is a single page, no reasons to switch

    // first will win. Normally we expecting 0 or 1 bridge
    if (settingLock) {
        if (nextWindowState == NONE)
            return false;

        if (core::WndManager::RETURN_CODE::BTN2 == core::getWndManager()->questionTextDlg(
                "Config changes",
                "Configuration changes was made for the wallet and not applied. Do you want to drop them?",
                "Back", "Drop",
                "Stay at config page",
                "Drop my changes and continue",
                false, true)) {
            return true;
        }
        return false;
    }

    settingLock = false;
    return true;
}

core::SendCoinsParams  WalletConfig::getSendCoinsParams() const {
    //logger::logInfo(logger::STATE, "Call WalletConfig::getSendCoinsParams");
    return context->appContext->getSendCoinsParams();
}

// account refresh will be requested...
void WalletConfig::setSendCoinsParams(const core::SendCoinsParams & params) {
    logger::logInfo(logger::STATE, "Call WalletConfig::setSendCoinsParams with params");
    context->appContext->setSendCoinsParams(params);
    context->wallet->emitWalletBalanceUpdated(); // Number of outputs might change, requesting update in background
}

double WalletConfig::getGuiScale() const {
    //logger::logInfo(logger::STATE, "Call WalletConfig::getGuiScale");
    return context->appContext->getGuiScale();
}

double WalletConfig::getInitGuiScale() const {
    //logger::logInfo(logger::STATE, "Call WalletConfig::getInitGuiScale");
    return context->appContext->getInitGuiScale();
}

void WalletConfig::updateGuiScale(double scale) {
    logger::logInfo(logger::STATE, "Call WalletConfig::updateGuiScale with scale=" + QString::number(scale));
    context->appContext->setGuiScale(scale);
}

void WalletConfig::restartMwcQtWallet() {
    logger::logInfo(logger::STATE, "Call WalletConfig::restartMwcQtWallet");
    // Stopping wallet first
    util::requestRestartMwcQtWallet();
    QCoreApplication::quit();
}

bool WalletConfig::getWalletLogsEnabled() {
    //logger::logInfo(logger::STATE, "Call WalletConfig::getWalletLogsEnabled");
    return context->appContext->isLogsEnabled();
}

void WalletConfig::updateWalletLogsEnabled(bool enabled, bool needCleanupLogs) {
    logger::logInfo(logger::STATE, "Call WalletConfig::updateWalletLogsEnabled with enabled=" + QString(enabled ? "true" : "false") +
        " needCleanupLogs=" + QString(needCleanupLogs ? "true" : "false"));
    context->appContext->setLogsEnabled(enabled);

    logger::enableLogs(enabled);

    if (needCleanupLogs)  // logs expected to be disabled by this point
        logger::cleanUpLogs();

}

bool WalletConfig::isOutputLockingEnabled() {
    //logger::logInfo(logger::STATE, "Call WalletConfig::isOutputLockingEnabled");
    return context->appContext->isLockOutputEnabled();
}

void WalletConfig::setOutputLockingEnabled(bool lockingEnabled) {
    logger::logInfo(logger::STATE, "Call WalletConfig::setOutputLockingEnabled with lockingEnabled=" + QString(lockingEnabled ? "true" : "false"));
    context->appContext->setLockOutputEnabled(lockingEnabled, context->wallet);
}

bool WalletConfig::getNotificationWindowsEnabled() {
    //logger::logInfo(logger::STATE, "Call WalletConfig::getNotificationWindowsEnabled");
    return context->appContext->getNotificationWindowsEnabled();
}

void WalletConfig::setNotificationWindowsEnabled(bool enabled) {
    logger::logInfo(logger::STATE, "Call WalletConfig::setNotificationWindowsEnabled with enabled=" + QString(enabled ? "true" : "false"));
    context->appContext->setNotificationWindowsEnabled(enabled);
}

bool WalletConfig::updateTimeoutValue(int timeout) {
    logger::logInfo(logger::STATE, "Call WalletConfig::updateTimeoutValue with timeout=" + QString::number(timeout));
    util::ConfigReader reader;
    QString configFN = config::getMwcGuiWalletConf();
    if ( !reader.readConfig( configFN ) ) {
        core::getWndManager()->messageTextDlg("Internal Error",
                                         "Unable to update wallet config file " + configFN );
        return false;
    }

    bool updateOk = reader.updateConfig("logoutTimeout", QString::number(timeout));

    if (!updateOk) {
        core::getWndManager()->messageTextDlg("Error", "Wallet unable to set the selected logout time." );
        return false;
    } else {
        config::setLogoutTimeMs(timeout * 1000);
        context->stateMachine->resetLogoutLimit(true);
    }
    return true;
}


}

