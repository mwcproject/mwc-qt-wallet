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
#include "../windows/x_walletconfig_w.h"
#include "../windows/x_nodeconfig_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../util/execute.h"
#include "../util/Log.h"
#include <QCoreApplication>
#include <QFile>

namespace state {

WalletConfig::WalletConfig( StateContext * _context) :
    State (_context, STATE::WALLET_CONFIG) {
}

WalletConfig::~WalletConfig() {}

NextStateRespond WalletConfig::execute() {
    if (context->appContext->getActiveWndState() != STATE::WALLET_CONFIG)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if ( config::isOnlineNode() ) {
        wndNode = (wnd::NodeConfig *) context->wndManager->switchToWindowEx( mwc::PAGE_X_WALLET_CONFIG,
                new wnd::NodeConfig( context->wndManager->getInWndParent(), this ) );

    }
    else {
        wndWallet = (wnd::WalletConfig *) context->wndManager->switchToWindowEx( mwc::PAGE_X_WALLET_CONFIG,
                new wnd::WalletConfig( context->wndManager->getInWndParent(), this ) );
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// State can block the state change. Wallet config is the first usage.
bool WalletConfig::canExitState() {
    if ( wndWallet != nullptr ) {
        return wndWallet->askUserForChanges();
    }
    if ( wndNode != nullptr ) {
        return wndNode->askUserForChanges();
    }

    return true;
}

wallet::WalletConfig WalletConfig::getWalletConfig() const {
    return context->wallet->getWalletConfig();
}

wallet::WalletConfig WalletConfig::getDefaultWalletConfig() const {
    return context->wallet->getDefaultConfig();
}

bool WalletConfig::setWalletConfig(const wallet::WalletConfig & config, bool guiWalletRestartExpected) {
    if (context->wallet->setWalletConfig(config, context->appContext, context->mwcNode)) {
        // update the window manager with the latest data path
        context->wndManager->setDataPath(config.getDataPath());

        // restarting the wallet...
        if (guiWalletRestartExpected)
            return false; // no need to restart the mwc713. Whole wallet need to be restarted soon

        context->stateMachine->executeFrom( STATE::NONE );
        return true;
    }
    return false;
}


core::SendCoinsParams  WalletConfig::getSendCoinsParams() const {
    return context->appContext->getSendCoinsParams();
}

// account refresh will be requested...
void WalletConfig::setSendCoinsParams(const core::SendCoinsParams & params) {
    context->appContext->setSendCoinsParams(params);
    context->wallet->updateWalletBalance(false,false); // Number of outputs might change, requesting update in background
}

double WalletConfig::getGuiScale() const {
    return context->appContext->getGuiScale();
}

double WalletConfig::getInitGuiScale() const {
    return context->appContext->getInitGuiScale();
}

void WalletConfig::updateGuiScale(double scale) {
    context->appContext->setGuiScale(scale);
}

void WalletConfig::restartMwcQtWallet() {
    // Stopping wallet first
    util::requestRestartMwcQtWallet();
    QCoreApplication::quit();
}

bool WalletConfig::getWalletLogsEnabled() {
    return context->appContext->isLogsEnabled();
}

void WalletConfig::updateWalletLogsEnabled(bool enabled, bool needCleanupLogs) {
    context->appContext->setLogsEnabled(enabled);

    logger::enableLogs(enabled);

    if (needCleanupLogs)  // logs expected to be disabled by this point
        logger::cleanUpLogs();

}

bool WalletConfig::getAutoStartMQSEnabled() {
    return context->appContext->isAutoStartMQSEnabled();
}

void WalletConfig::updateAutoStartMQSEnabled(bool enabled) {
    context->appContext->setAutoStartMQSEnabled(enabled);
}

bool WalletConfig::getAutoStartKeybaseEnabled() {
    return context->appContext->isAutoStartKeybaseEnabled();
}

void WalletConfig::updateAutoStartKeybaseEnabled(bool enabled) {
    context->appContext->setAutoStartKeybaseEnabled(enabled);
}


}

