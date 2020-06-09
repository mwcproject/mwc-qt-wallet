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

#include "k_accounttransfer_b.h"
#include "../BridgeManager.h"
#include "../../state/k_AccountTransfer.h"
#include "../../state/state.h"

namespace bridge {

static state::AccountTransfer * getState() { return (state::AccountTransfer *) state::getState(state::STATE::ACCOUNT_TRANSFER); }

AccountTransfer::AccountTransfer(QObject *parent) : QObject(parent) {
    getBridgeManager()->addAccountTransfer(this);
}

AccountTransfer::~AccountTransfer() {
    getBridgeManager()->removeAccountTransfer(this);
}

void AccountTransfer::showTransferResults(bool ok, QString errMsg) {
    emit sgnShowTransferResults(ok, errMsg);
}
void AccountTransfer::updateAccounts() {
    emit sgnUpdateAccounts();
}
void AccountTransfer::hideProgress() {
    emit sgnHideProgress();
}

bool AccountTransfer::transferFunds(QString from, QString to, QString sendAmount) {
    return getState()->transferFunds(from, to, sendAmount);
}

// Back button was pressed.
void AccountTransfer::goBack() {
    return getState()->goBack();
}


}