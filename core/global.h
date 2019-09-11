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

#ifndef GLOBALMWC_H
#define GLOBALMWC_H

#include <QObject>
#include <QVector>

class QApplication;
class QMainWindow;

namespace mwc {

const QString APP_NAME = "mwc wallet";

// We not checking mwc713 versions because so far there is no benefits from that.
// MWC version that we are expecting to ship
//const QString WMC_713_VERSION("2.1.0");

const QString PROMPTS_MWC713 = "<<+)mwc713(+>>";

const QString DATETIME_TEMPLATE_MWC713 = "yyyy-MM-dd hh:mm:ss";
const QString DATETIME_TEMPLATE_THIS = "hh:mm:ss dd-MM-yyyy";

const QString DEL_ACCONT_PREFIX = "del_";
const QString HODL_ACCONT_PREFIX = "hodl_"; // hodl not in use yet, just reserving

const QVector<QString> BANNED_ACCOUT_PREFIXES = {DEL_ACCONT_PREFIX, HODL_ACCONT_PREFIX};

#ifdef Q_OS_DARWIN
const QString MWC713_DEFAULT_CONFIG = ":/resource/wallet713_mac.toml";
#else
const QString MWC713_DEFAULT_CONFIG = ":/resource/wallet713_def.toml";
#endif

const int64_t AIRDROP_TRANS_KEEP_TIME_MS = 1000L*3600L*24L*15; // 15 days should be enough

// Limit for the node height to consider it out of sync. 1 blocks is 1 minute.
const int NODE_HEIGHT_DIFF_LIMIT = 5;

// Default MQ servers
const QString DEFAULT_HOST_MWC_MQ  = "mq.mwc.mw";
const QString DEFAULT_HOST_MWC_MQS = "mqs.mwc.mw";

// Block Explorer URLs
const QString BLOCK_EXPLORER_URL_MAINNET = "explorer.mwc.mw";
const QString BLOCK_EXPLORER_URL_FLOONET = "explorer.floonet.mwc.mw";

//////////////////////////////
// Page Names

const QString PAGE_A_ACCOUNT_LOGIN  = "Login into Wallet";
const QString PAGE_A_ACCOUNT_UNLOCK = "Unlock Wallet";
const QString PAGE_A_INIT_ACCOUNT   = "Init a new Wallet";
const QString PAGE_A_NEW_WALLET     = "Init a new Wallet";
const QString PAGE_A_ENTER_SEED     = "Recover Wallet from a Passphrase";
const QString PAGE_A_NEW_WALLET_PASSPHRASE= "Passphrase for a new Wallet";
const QString PAGE_A_PASSPHRASE_TEST  = "Passphrase test";
const QString PAGE_A_RECOVERY_FROM_PASSPHRASE = "Recovery from passphrase";

const QString PAGE_E_TRANSACTION    = "Transactions";
const QString PAGE_E_RECEIVE        = "Receive coins";
const QString PAGE_E_LISTENING      = "Listening";
const QString PAGE_E_OUTPUTS        = "Outputs";

const QString PAGE_G_SEND           = "Send coins";
const QString PAGE_G_SEND_ONLINE    = "Send coins to address";
const QString PAGE_G_SEND_FILE      = "Send coins with file";
const QString PAGE_G_FINALIZE_UPLOAD= "Finalize Transaction";
const QString PAGE_G_FINALIZE_TRANS = "Finalize Transaction";
const QString PAGE_G_RECEIVE_TRANS  = "Receive File Transaction";

const QString PAGE_K_ACCOUNTS       = "Accounts";
const QString PAGE_K_ACCOUNT_TRANSFER = "Transfer coins between accounts";

const QString PAGE_M_AIRDROP        = "Airdrop";
const QString PAGE_M_AIRDROP_CLAIM  = "Claim coins for BTC address";

const QString PAGE_U_NODE_STATUS    = "mwc node status";

const QString PAGE_W_CONTACTS       = "Contacts";

const QString PAGE_X_EVENTS         = "Event log";
const QString PAGE_X_SHOW_PASSPHRASE= "Wallet mnemonic passphrase";
const QString PAGE_X_WALLET_CONFIG  = "Wallet Config";
const QString PAGE_X_RESYNC         = "Re-sync wallet with mwc node";

const QString PAGE_HODL             = "HODL";

void setApplication(QApplication * app, QMainWindow * mainWindow);

void closeApplication();

}


#endif // GLOBALMWC_H
