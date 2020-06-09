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

#include "e_receive_b.h"
#include "../BridgeManager.h"
#include "../../state/e_Receive.h"
#include "../../state/state.h"

namespace bridge {

static state::Receive * getState() { return (state::Receive *) state::getState(state::STATE::RECEIVE_COINS); }

Receive::Receive(QObject * parent) :
    QObject(parent)
{
    getBridgeManager()->addReceive(this);
}

Receive::~Receive() {
    getBridgeManager()->removeReceive(this);
}

void Receive::signTransaction(QString fileName) {
    getState()->signTransaction(fileName);
}

void Receive::onTransactionActionIsFinished( bool success, QString message ) {
    emit sgnTransactionActionIsFinished(success, message);
}


}
