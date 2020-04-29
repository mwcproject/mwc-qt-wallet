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
static QString mwc713conf;
static QString mwcGuiWalletConf;
static QString mwcPath;
static QString mwczipPath;
static QString wallet713path;
static QString airdropUrlMainNetUrl;
static QString airdropUrlTestNetUrl;
static QString hodlUrlMainNetUrl;
static QString hodlUrlTestNetUrl;
static int64_t logoutTimeMs = 1000*60*15; // 15 minutes is default
static double  timeoutMultiplier = 1.0;
static bool    useMwcMqS = true;
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


void setMwc713conf( QString conf ) {
    mwc713conf = conf;
}
void setMwcGuiWalletConf( QString conf ) {
    mwcGuiWalletConf = conf;
}

/**
 * COnfiguration for mwc-mq-wallet
 * @param mwcPath               - path to mwc-node.  Not used now
 * @param wallet713path         - path to mwc713
 * @param airdropUrlMainNetUrl  - Airdrop server claims URL for main net
 * @param airdropUrlTestNetUrl  - Airdrop server claims URL for floo net
 * @param logoutTimeMs          - Automatic locking for the wallet
 * @param timeoutMultiplier     - Multiplier for jobs timeouts. Default is 1.0
 * @param useMwcMqS             - true: use mwc mqs for slates exchange.  false: using mwc mq (non secure grin box) for slates exchange
 * @param sendTimeoutMs         - timeout for mwc mq send. Expected that 2nd party is online. Otherwise we will ask user if he want to stop waiting and cancel transaction.
 */
void setConfigData(WALLET_RUN_MODE _runMode, QString _mwcPath, QString _wallet713path, QString _mwczipPath,
                   QString _airdropUrlMainNetUrl, QString _airdropUrlTestNetUrl,
                   QString _hodlUrlMainNetUrl, QString _hodlUrlTestNetUrl,
                   int64_t  _logoutTimeMs,
                   double _timeoutMultiplier,
                   bool _useMwcMqS,
                   int _sendTimeoutMs) {
    runMode = _runMode;
    mwcPath = _mwcPath;
    wallet713path = _wallet713path;

    if (_mwczipPath.isEmpty()) {
        int pos = wallet713path.lastIndexOf("mwc713", -1, Qt::CaseInsensitive );
        if (pos>=0)
            mwczipPath = wallet713path.left(pos) + "mwczip" + wallet713path.right( wallet713path.length() - pos - strlen("mwc713") );
    }
    else {
        mwczipPath = _mwczipPath;
    }

    airdropUrlMainNetUrl = _airdropUrlMainNetUrl;
    airdropUrlTestNetUrl = _airdropUrlTestNetUrl;
    hodlUrlMainNetUrl = _hodlUrlMainNetUrl;
    hodlUrlTestNetUrl = _hodlUrlTestNetUrl;
    logoutTimeMs = _logoutTimeMs;
    timeoutMultiplier = _timeoutMultiplier;
    useMwcMqS = _useMwcMqS;
    sendTimeoutMs = _sendTimeoutMs;
}

void setMwcZipPath(const QString & _mwcZipPath) {
    mwczipPath = _mwcZipPath;
}


// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwc713conf() {return mwc713conf;}
const QString & getMwcGuiWalletConf() {return mwcGuiWalletConf;}

const QString & getMwcPath() {return mwcPath;}
const QString & getWallet713path() {return wallet713path;}
const QString & getMwcZipPath() {return mwczipPath;}
const QString & getAirdropMainNetUrl() {return airdropUrlMainNetUrl;}
const QString & getAirdropTestNetUrl() {return airdropUrlTestNetUrl;}

const QString & getHodlMainNetUrl() { return hodlUrlMainNetUrl;}
const QString & getHodlTestNetUrl() { return hodlUrlTestNetUrl;}

int64_t         getLogoutTimeMs() {return logoutTimeMs;}
void         setLogoutTimeMs(int64_t timeMs) {logoutTimeMs = timeMs;}


double          getTimeoutMultiplier() {return timeoutMultiplier;}
void            increaseTimeoutMultiplier() { timeoutMultiplier *= 1.6; }

bool            getUseMwcMqS() {return useMwcMqS;}

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

    return "mwc713conf=" + mwc713conf + "\n" +
            "mwcGuiWalletConf=" + mwcGuiWalletConf + "\n" +
            "mwcPath=" + mwcPath + "\n" +
            "wallet713path=" + wallet713path + "\n" +
            "useMwcMqS=" + (useMwcMqS?"true":"false") + "\n" +
            "sendTimeoutMs=" + QString::number(sendTimeoutMs) + "\n" +
            "run_mode=" + runModeStr + "\n" +
            "airdropUrlMainNetUrl=" + airdropUrlMainNetUrl + "\n" +
            "airdropUrlTestNetUrl=" + airdropUrlTestNetUrl + "\n" +
            "timeoutMultiplier=" + QString::number(timeoutMultiplier) + "\n" +
            "logoutTimeMs=" + QString::number(logoutTimeMs);
}

}
