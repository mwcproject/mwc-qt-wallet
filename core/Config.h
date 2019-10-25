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
enum class WALLET_RUN_MODE { ONLINE_WALLET, ONLINE_NODE, COLD_WALLET };
QPair<bool, WALLET_RUN_MODE> runModeFromString(QString str);
bool isOnlineWallet();
bool isOnlineNode();
bool isColdWallet();


void setMwc713conf( QString conf );
void setMwcGuiWalletConf( QString conf );

/**
 * COnfiguration for mwc-mq-wallet
 * @param mwcPath               - path to mwc-node.  Not used now
 * @param wallet713path         - path to mwc713
 * @param mainStyleSheetPath    - path to main (non modal) stylesheet
 * @param dialogsStyleSheetPath - path to modal dialogs stylesheet (changes set to the main stylesheet)
 * @param airdropUrlMainNetUrl  - Airdrop server claims URL for main net
 * @param airdropUrlTestNetUrl  - Airdrop server claims URL for floo net
 * @param logoutTimeMs          - Automatic locking for the wallet
 * @param timeoutMultiplier     - Multiplier for jobs timeouts. Default is 1.0
 * @param useMwcMqS             - true: use mwc mqs for slates exchange.  false: using mwc mq (non secure grin box) for slates exchange
 */
void setConfigData(WALLET_RUN_MODE runMode, QString mwcPath, QString wallet713path,
                   QString mainStyleSheetPath, QString dialogsStyleSheetPath, QString airdropUrlMainNetUrl, QString airdropUrlTestNetUrl, int64_t logoutTimeMs,
                   double timeoutMultiplier,
                   bool useMwcMqS,
                   int sendTimeoutMs);


// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwc713conf();
const QString & getMwcGuiWalletConf();

const QString & getMwcpath(); // Not using now because so far there is no local node is running
const QString & getWallet713path();
const QString & getMainStyleSheetPath();
const QString & getDialogsStyleSheetPath();
const QString & getAirdropMainNetUrl();
const QString & getAirdropTestNetUrl();
int64_t         getLogoutTimeMs();

double          getTimeoutMultiplier();
void            increaseTimeoutMultiplier();

bool            getUseMwcMqS();

int             getSendTimeoutMs();

QString toString();


}

#endif //GUI_WALLET_CONFIG_H
