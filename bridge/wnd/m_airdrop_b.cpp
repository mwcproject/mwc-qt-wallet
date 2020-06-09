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

#include "m_airdrop_b.h"
#include "../BridgeManager.h"
#include "../../state/state.h"
#include "../../state/m_airdrop.h"

namespace bridge {

static state::Airdrop * getState() { return (state::Airdrop *) state::getState(state::STATE::AIRDRDOP_MAIN); }

Airdrop::Airdrop(QObject * parent) : QObject(parent) {
    getBridgeManager()->addAirdrop(this);
}

Airdrop::~Airdrop() {
    getBridgeManager()->removeAirdrop(this);
}

// true if status is active
void Airdrop::updateAirDropStatus( bool waiting, bool status, QString message ) {
    emit sgnUpdateAirDropStatus(waiting, status, message);
}

void Airdrop::updateClaimStatus( int idx, QString requestBtcAddress,
                        QString status, QString message, int64_t amount, int errCode) {
    emit sgnUpdateClaimStatus(  idx, requestBtcAddress,
            status, message, amount>0 ? util::nano2one(amount) : "", errCode );
}

void Airdrop::reportMessage( QString title, QString message ) {
    emit sgnReportMessage(title, message);
}

void Airdrop::refreshAirdropStatusInfo() {
    getState()->refreshAirdropStatusInfo();
}

// initiate claiming process for btc address
void Airdrop::startClaimingProcess( QString btcAddress, QString password ) {
    getState()->startClaimingProcess( btcAddress, password );
}

// start claim for btc address
void Airdrop::requestClaimMWC( QString btcAddress, QString challendge, QString signature, QString identifier ) {
    getState()->requestClaimMWC( btcAddress, challendge, signature, identifier );
}

// return back to ardrop page
void Airdrop::backToMainAirDropPage() {
    getState()->backToMainAirDropPage();
}


}
