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

#include "y_selectmode.h"
#include "../control/messagebox.h"
#include "../util/ConfigReader.h"
#include "../util/execute.h"
#include "../core/windowmanager.h"
#include "../core/global.h"
#include "../windows/y_selectmode_w.h"
#include <QCoreApplication>

namespace state {

SelectMode::SelectMode( StateContext * _context) :
    State (_context, STATE::WALLET_RUNNING_MODE)
{
}

SelectMode::~SelectMode() {}

void SelectMode::deleteWnd(wnd::SelectMode * _wnd) {
    if (wnd == _wnd)
        wnd = nullptr;
}


NextStateRespond SelectMode::execute() {
    if (context->appContext->getActiveWndState() != STATE::WALLET_RUNNING_MODE)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    wnd = (wnd::SelectMode *) context->wndManager->switchToWindowEx( mwc::PAGE_Y_SELECT_RUNNING_MODE,
                                               new wnd::SelectMode( context->wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

config::WALLET_RUN_MODE SelectMode::getWalletRunMode() const {
    return config::getWalletRunMode();
}

// Will require restart
void SelectMode::updateWalletRunMode( config::WALLET_RUN_MODE newRunMode ) {
    if (config::getWalletRunMode() == newRunMode) {
        control::MessageBox::messageText(nullptr, "Running Mode", "You allready running wallet in selected mode. Nothing need to done." );
        return;
    }

    // Need to switch the mode.
    // 1. Update the config...
    util::ConfigReader reader;
    QString configFN = config::getMwcGuiWalletConf();
    if ( !reader.readConfig( configFN ) ) {
        control::MessageBox::messageText(nullptr, "Internal Error",
                                     "Unable to update wallet config file " + configFN );
    }

    bool updateOk = true;
    switch (newRunMode) {
        case config::WALLET_RUN_MODE::ONLINE_WALLET:
            updateOk = reader.updateConfig("running_mode", "\"online_wallet\"" );
            break;
        case config::WALLET_RUN_MODE::ONLINE_NODE:
            updateOk = reader.updateConfig("running_mode", "\"online_node\"" );
            break;
        case config::WALLET_RUN_MODE::COLD_WALLET:
            updateOk = reader.updateConfig("running_mode", "\"cold_wallet\"" );
            break;
        default:
            Q_ASSERT(false);
    }

    if (!updateOk) {
        control::MessageBox::messageText(nullptr, "Error", "Wallet unable to switch to the selected mode." );
        return;
    }

    // 2. Restart
    // Stopping wallet first
    util::requestRestartMwcQtWallet();
    QCoreApplication::quit();
}


}

