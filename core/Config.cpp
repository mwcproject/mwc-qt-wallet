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

#include "Config.h"

namespace config {

static WALLET_RUN_MODE runMode = WALLET_RUN_MODE::ONLINE_WALLET;
static QString mwcGuiWalletConf;
//static QString mwcPath;
static int64_t logoutTimeMs = 1000*60*15; // 15 minutes is default
static double  timeoutMultiplier = 1.0;
static int     sendTimeoutMs = 60000; // 1 minute


QPair<bool, WALLET_RUN_MODE> runModeFromString(QString str) {
    if (str == "online_wallet")
        return QPair<bool, WALLET_RUN_MODE>(true, WALLET_RUN_MODE::ONLINE_WALLET);
    else if (str == "online_node")
        return QPair<bool, WALLET_RUN_MODE>(true, WALLET_RUN_MODE::ONLINE_NODE);
    else if (str == "cold_wallet")
        return QPair<bool, WALLET_RUN_MODE>(true, WALLET_RUN_MODE::COLD_WALLET);
    else
        return QPair<bool, WALLET_RUN_MODE>(false, WALLET_RUN_MODE::ONLINE_WALLET);
}
WALLET_RUN_MODE getWalletRunMode() {return runMode;}
bool isOnlineWallet() {return runMode == WALLET_RUN_MODE::ONLINE_WALLET;}
bool isOnlineNode()   {return runMode == WALLET_RUN_MODE::ONLINE_NODE;}
bool isColdWallet()   {return runMode == WALLET_RUN_MODE::COLD_WALLET;}


void setMwcGuiWalletConf( QString conf ) {
    mwcGuiWalletConf = conf;
}

/**
 * COnfiguration for mwc-mq-wallet
 * @param mwcPath               - path to mwc-node.  Not used now
 * @param wallet713path         - path to mwc713
 * @param logoutTimeMs          - Automatic locking for the wallet
 * @param timeoutMultiplier     - Multiplier for jobs timeouts. Default is 1.0
 * @param useMwcMqS             - true: use mwc mqs for slates exchange.  false: using mwc mq (non secure grin box) for slates exchange
 * @param sendTimeoutMs         - timeout for mwc mq send. Expected that 2nd party is online. Otherwise we will ask user if he want to stop waiting and cancel transaction.
 */
void setConfigData(WALLET_RUN_MODE _runMode,
                   int64_t  _logoutTimeMs,
                   double _timeoutMultiplier,
                   int _sendTimeoutMs) {
    runMode = _runMode;
    //mwcPath = _mwcPath;

    logoutTimeMs = _logoutTimeMs;
    timeoutMultiplier = _timeoutMultiplier;
    sendTimeoutMs = _sendTimeoutMs;
}


// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwcGuiWalletConf() {return mwcGuiWalletConf;}

int64_t         getLogoutTimeMs() {return logoutTimeMs;}
void            setLogoutTimeMs(int64_t timeMs) {logoutTimeMs = timeMs;}


double          getTimeoutMultiplier() {return timeoutMultiplier;}
void            increaseTimeoutMultiplier() { timeoutMultiplier *= 1.6; }

int             getSendTimeoutMs() {return sendTimeoutMs;}


QString toString() {

    QString runModeStr = "Unknown";
    switch(runMode) {
        case WALLET_RUN_MODE::ONLINE_WALLET:
            runModeStr = "online_wallet";
            break;
        case WALLET_RUN_MODE::ONLINE_NODE:
            runModeStr = "online_node";
            break;
        case WALLET_RUN_MODE::COLD_WALLET:
            runModeStr = "cold_wallet";
            break;
    }

    return  "mwcGuiWalletConf=" + mwcGuiWalletConf + "\n" +
            "sendTimeoutMs=" + QString::number(sendTimeoutMs) + "\n" +
            "run_mode=" + runModeStr + "\n" +
            "timeoutMultiplier=" + QString::number(timeoutMultiplier) + "\n" +
            "logoutTimeMs=" + QString::number(logoutTimeMs);
}

}
