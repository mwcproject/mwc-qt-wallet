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

#include "../BridgeManager.h"
#include "g_finalize_b.h"
#include "../../state/state.h"
#include "../../state/g_Finalize.h"

namespace bridge {

static state::Finalize * getState() { return (state::Finalize *) state::getState(state::STATE::FINALIZE); }

Finalize::Finalize(QObject * parent) :
        QObject(parent)
{
    getBridgeManager()->addFinalize(this);
}

Finalize::~Finalize() {
    getBridgeManager()->removeFinalize(this);
}

void Finalize::hideProgress() {
    emit sgnHideProgress();
}

// Start Processing file slate.
void Finalize::uploadFileTransaction(QString uriFileName, QString uriDecodedFileName) {
    getState()->uploadFileTransaction(uriFileName, uriDecodedFileName);
}

void Finalize::uploadSlatepackTransaction( QString slatepack, QString slateJson, QString sender ) {
    getState()->uploadSlatepackTransaction( slatepack, slateJson, sender );
}

// Check if the node healthy enough to do finalization.
bool Finalize::isNodeHealthy() {
    return getState()->isNodeHealthy();
}

bool Finalize::needResultTxFileName() {
    return getState()->needResultTxFileName();
}

// Files transaction page, continue with a file
void Finalize::finalizeFile(QString fileName, QString resultTxFileName, bool fluff) {
    getState()->finalizeFile(fileName, resultTxFileName, fluff);
}
// Files transaction page, continue with a Slatepack
void Finalize::finalizeSlatepack(QString slatepack, QString txUuid, QString resultTxFileName, bool fluff) {
    getState()->finalizeSlatepack(slatepack, txUuid, resultTxFileName, fluff);
}

void Finalize::cancelFileFinalization() {
    getState()->ftBack();
}

}
