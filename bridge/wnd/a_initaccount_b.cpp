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

#include "a_initaccount_b.h"
#include "../../state/state.h"
#include "../../state/statemachine.h"
#include "../../state/a_initaccount.h"
#include "util/Log.h"

namespace bridge {

static state::InitAccount * getState() {
    return (state::InitAccount *) state::getStateContext()->stateMachine->getState(state::STATE::STATE_INIT);
}

InitAccount::InitAccount(QObject *parent) : QObject(parent) {}

InitAccount::~InitAccount() {}

// Set Password for a new account (First step of account creation)
void InitAccount::setPassword(QString password) {
    logger::logInfo(logger::BRIDGE, "Call InitAccount::setPassword with <password>");
    getState()->setPassword(password);
}

// Cancel whatever init account you have and return to the starting page
void InitAccount::cancelInitAccount() {
    logger::logInfo(logger::BRIDGE, "Call InitAccount::cancelInitAccount");
    // Not needed for now and implementaiton need to be done.
    // keeping the bridge method in case UI will need cancellation workflow
    getState()->cancel();
}

// Create a wallet form the seed
void InitAccount::createWalletWithSeed(QVector<QString> seed) {
    logger::logInfo(logger::BRIDGE, "Call InitAccount::createWalletWithSeed with seed Length " + QString::number(seed.size()));
    getState()->createWalletWithSeed(seed);
}

// newWalletChoice: see values at state::InitAccount::NEW_WALLET_CHOICE
// network: see values at state::InitAccount::MWC_NETWORK
void InitAccount::submitWalletCreateChoices(int network, QString newInstanceName) {
    logger::logInfo(logger::BRIDGE, "Call InitAccount::submitWalletCreateChoices with network=" + QString::number(network) + " instanceName=" + newInstanceName);
    getState()->submitWalletCreateChoices( state::InitAccount::MWC_NETWORK(network), newInstanceName);
}

// wordsNum: words number to create
void InitAccount::submitSeedLength(int wordsNum) {
    logger::logInfo(logger::BRIDGE, "Call InitAccount::submitSeedLength with wordsNum=" + QString::number(wordsNum));
    getState()->submitSeedLength(wordsNum);
}


}
