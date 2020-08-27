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

#include "a_initfirsttime_w.h"
#include "ui_a_initfirsttime.h"
#include "../util/Process.h"
#include "../util_desktop/timeoutlock.h"
#include "../util/ioutils.h"
#include "../core/WndManager.h"
#include "../control_desktop/messagebox.h"
#include "../bridge/wnd/y_selectmode_b.h"
#include "../core/Config.h"
#include "../core_desktop/DesktopWndManager.h"
#include "../bridge/wnd/a_startwallet_b.h"

namespace wnd {

InitFirstTime::InitFirstTime(QWidget *parent) :
    core::PanelBaseWnd(parent),
    ui(new Ui::InitFirstTime)
{
    ui->setupUi(this);

    startWallet = new bridge::StartWallet(this);
}

InitFirstTime::~InitFirstTime()
{
    delete ui;
}

void InitFirstTime::on_newInstanceButton_clicked()
{
    startWallet->createNewWalletInstance("", false);
}

void InitFirstTime::on_restoreInstanceButton_clicked()
{
    startWallet->createNewWalletInstance("", true);
}

void InitFirstTime::on_openWalletButton_clicked()
{
    QString wallet_dir = core::selectWalletDirectory();
    if (wallet_dir.isEmpty())
         return;
    startWallet->createNewWalletInstance(wallet_dir, false);
}

void InitFirstTime::on_runOnlineNodeButton_clicked()
{
    util::TimeoutLockObject to("InitAccount");
    if ( core::WndManager::RETURN_CODE::BTN2 == control::MessageBox::questionText(this, "Running Mode",
                          "You are switching to 'Online Node'.\nOnline Node can be used as a data provider for the Cold Wallet.",
                          "Cancel", "Continue",
                          "Don't switch to Online Node, keep my wallet as it is",
                          "Continue and restart as Online Node",
                          false, true) ) {
        // Restarting wallet in a right mode...
        // First, let's upadte a config
        // We are restarteg, it is not a memry leak.
        bridge::SelectMode * selectMode = new bridge::SelectMode(this);
        selectMode->updateWalletRunMode( int(config::WALLET_RUN_MODE::ONLINE_NODE) );
    }
}

void InitFirstTime::on_helpButton_clicked()
{
    control::MessageBox::messageText(this, "Help", "'Open Wallet' - Open the wallet form external location like USB stick or another drive.\n\n"
                                                   "'New Instance' - Create a new wallet instance.\n\n"
                                                   "'Restore Instance' - Restore a new wallet instance from the mnemonic passphrase.\n\n"
                                                   "'Cold Wallet Node' - This configuraiton is needed if you are using the cold wallet. Online Node needed for exportign the blockchina data to the Cold Wallet and for publishing finalized transacitons.");
}

}

