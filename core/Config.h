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

#ifndef GUI_WALLET_CONFIG_H
#define GUI_WALLET_CONFIG_H

#include <QString>
#include <QPair>

namespace config {

// "online_wallet"  - (Default) normal mode, running the wallet as online mwc wallet with full functionality
// "online_node"  - runs as the online node. It can export the chain data for the cold wallet,
//                         and publish transaction from the cold wallet.
// "cold_wallet"  - runs wallet in offline mode. Since no online access is expected, only file based
//                         operations will be available
enum class WALLET_RUN_MODE { ONLINE_WALLET=1, ONLINE_NODE=2, COLD_WALLET=3 };
QPair<bool, WALLET_RUN_MODE> runModeFromString(QString str);

WALLET_RUN_MODE getWalletRunMode();
bool isOnlineWallet();
bool isOnlineNode();
bool isColdWallet();


void setMwcGuiWalletConf( QString conf );

/**
 * COnfiguration for mwc-mq-wallet
 * @param mainStyleSheetPath    - path to main (non modal) stylesheet
 * @param dialogsStyleSheetPath - path to modal dialogs stylesheet (changes set to the main stylesheet)
 * @param logoutTimeMs          - Automatic locking for the wallet
 * @param timeoutMultiplier     - Multiplier for jobs timeouts. Default is 1.0
 */
void setConfigData(WALLET_RUN_MODE runMode,
                   int64_t logoutTimeMs,
                   double timeoutMultiplier,
                   int sendTimeoutMs);

// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwcGuiWalletConf();

int64_t         getLogoutTimeMs();
void            setLogoutTimeMs(int64_t timeMs);

double          getTimeoutMultiplier();
void            increaseTimeoutMultiplier();

int             getSendTimeoutMs();

QString toString();


}

#endif //GUI_WALLET_CONFIG_H
