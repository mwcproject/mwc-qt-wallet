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

#include "c_newseed_b.h"
#include "../BridgeManager.h"
#include "../../state/state.h"
#include "../../state/statemachine.h"
#include "../../state/a_initaccount.h"

namespace bridge {

static state::InitAccount * getState() {return (state::InitAccount *) state::getState( state::STATE::STATE_INIT );}

NewSeed::NewSeed(QObject * parent) : QObject(parent) {
    getBridgeManager()->addNewSeed(this);
}

NewSeed::~NewSeed() {
    getBridgeManager()->removeNewSeed(this);
}

void NewSeed::showSeedData(const QVector<QString> & seed) {
    emit sgnShowSeedData(seed);
}

// Done looking at a new seed. Let's go forward
void NewSeed::doneWithNewSeed() {
    getState()->doneWithNewSeed();
}

// Submit a new word from the user input
Q_INVOKABLE void NewSeed::submitSeedWord(QString word) {
    getState()->submitSeedWord(word);
}


}