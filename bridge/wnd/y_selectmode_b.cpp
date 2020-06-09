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

#include "y_selectmode_b.h"
#include "../BridgeManager.h"
#include "../../state/y_selectmode.h"


namespace bridge {

SelectMode::SelectMode(QObject * parent) : QObject(parent) {
    getBridgeManager()->addSelectMode(this);
}

SelectMode::~SelectMode() {
    getBridgeManager()->removeSelectMode(this);
}

//  get wallet running mode id. Values: config::WALLET_RUN_MODE
Q_INVOKABLE int SelectMode::getWalletRunMode() {
    return int( state::SelectMode::getWalletRunMode() );
}


// Change wallet running mode. This call will retart the wallet app.
// walletRunMode :  config::WALLET_RUN_MODE
void SelectMode::updateWalletRunMode( int walletRunMode ) {
    state::SelectMode::updateWalletRunMode( config::WALLET_RUN_MODE(walletRunMode) );
}


}
