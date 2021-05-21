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
#include <QDebug>
#include "../../util/Files.h"

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

void Receive::hideProgress() {
    emit sgnHideProgress();
}

// Sign file transaction
void Receive::signTransaction(QString uriFileName) {
    // Let's try to read the file...
    getState()->signTransaction( uriFileName );
}

// Sign slatepack transaction. The primary data is slatepack.  slateJson can be calculated, passed becuase we already have after verification
void Receive::signSlatepackTransaction(QString slatepack, QString slateJson, QString slateSenderAddress) {
    getState()->signSlatepackTransaction(slatepack, slateJson, slateSenderAddress);
}

void Receive::onTransactionActionIsFinished( bool success, QString message ) {
    emit sgnTransactionActionIsFinished(success, message);
}

void Receive::cancelReceive() {
    getState()->ftBack();
}

// Files transaction page, continue with a file
void Receive::receiveFile(QString uriFileName, QString description ) {
    getState()->receiveFile(uriFileName, description);
}

// Files transaction page, continue with a Slatepack
void Receive::receiveSlatepack(QString slatepack, QString description) {
    getState()->receiveSlatepack(slatepack, description);
}


}
