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

namespace config {

void setMwc713conf( QString conf );
void setMwcGuiWalletConf( QString conf );

void setConfigData(QString mwcPath, QString wallet713path,
                   QString mainStyleSheetPath, QString dialogsStyleSheetPath, QString airdropUrl, int64_t logoutTimeMs);


// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwc713conf();
const QString & getMwcGuiWalletConf();

const QString & getMwcpath(); // Not using now because so far there is no local node is running
const QString & getWallet713path();
const QString & getMainStyleSheetPath();
const QString & getDialogsStyleSheetPath();
const QString & getAirdropUrl();
int64_t         getLogoutTimeMs();

QString toString();


}

#endif //GUI_WALLET_CONFIG_H
