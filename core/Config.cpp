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

static QString mwc713conf;
static QString mwcGuiWalletConf;
static QString mwcPath;
static QString wallet713path;
static QString mainStyleSheetPath;
static QString dialogsStyleSheetPath;
static QString airdropUrl;
static int64_t logoutTimeMs = 1000*60*15; // 15 minutes is default


void setMwc713conf( QString conf ) {
    mwc713conf = conf;
}
void setMwcGuiWalletConf( QString conf ) {
    mwcGuiWalletConf = conf;
}

void setConfigData(QString _mwcPath, QString _wallet713path,
                   QString _mainStyleSheetPath, QString _dialogsStyleSheetPath, QString _airdropUrl, int64_t  _logoutTimeMs) {
    mwcPath = _mwcPath;
    wallet713path = _wallet713path;
    mainStyleSheetPath = _mainStyleSheetPath;
    dialogsStyleSheetPath = _dialogsStyleSheetPath;
    airdropUrl = _airdropUrl;
    logoutTimeMs = _logoutTimeMs;
}


// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwc713conf() {return mwc713conf;}
const QString & getMwcGuiWalletConf() {return mwcGuiWalletConf;}

const QString & getWallet713path() {return wallet713path;}
const QString & getMainStyleSheetPath() {return mainStyleSheetPath;}
const QString & getDialogsStyleSheetPath() {return dialogsStyleSheetPath;}
const QString & getAirdropUrl() {return airdropUrl;}
int64_t         getLogoutTimeMs() {return logoutTimeMs;}


QString toString() {
    return "mwc713conf=" + mwc713conf + "\n" +
            "mwcGuiWalletConf=" + mwcGuiWalletConf + "\n" +
            "mwcPath=" + mwcPath + "\n" +
            "wallet713path=" + wallet713path + "\n" +
            "mainStyleSheetPath=" + mainStyleSheetPath + "\n" +
            "dialogsStyleSheetPath=" + dialogsStyleSheetPath + "\n";
}


}