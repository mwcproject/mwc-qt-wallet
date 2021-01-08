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

#include "g_filetransaction_b.h"
#include "../BridgeManager.h"
#include "../../state/e_Receive.h"
#include "../../state/g_Finalize.h"

#define CALL_STATE_METHOD( PREF, METHOD )  if (callerId == state::RECEIVE_CALLER_ID) {  \
    PREF ((state::Receive*) state::getState(state::STATE::RECEIVE_COINS))->METHOD;     \
}                                                                                   \
else  {                                                                             \
    Q_ASSERT(callerId == state::FINALIZE_CALLER_ID);                                \
    PREF ((state::Finalize*) state::getState(state::STATE::FINALIZE))->METHOD;      \
}

namespace bridge {

FileTransaction::FileTransaction(QObject * parent) : QObject(parent) {
    getBridgeManager()->addFileTransaction(this);
}

FileTransaction::~FileTransaction() {
    getBridgeManager()->removeFileTransaction(this);
}

void FileTransaction::hideProgress() {
    emit sgnHideProgress();
}


void FileTransaction::ftBack() {
    CALL_STATE_METHOD( ,ftBack())
}

void FileTransaction::ftContinue(QString fileName, QString resultTxFileName, bool fluff) {
    if (callerId == state::RECEIVE_CALLER_ID)
        ((state::Receive*) state::getState(state::STATE::RECEIVE_COINS))->receiveFile(fileName, "");
    else
        ((state::Finalize*) state::getState(state::STATE::FINALIZE))->finalizeFile(fileName, resultTxFileName, fluff);
}

void FileTransaction::ftContinueSlatepack(QString slatepack, QString txUuid, QString resultTxFileName, bool fluff) {
    if (callerId == state::RECEIVE_CALLER_ID)
        ((state::Receive*) state::getState(state::STATE::RECEIVE_COINS))->receiveSlatepack(slatepack, "");
    else
        ((state::Finalize*) state::getState(state::STATE::FINALIZE))->finalizeSlatepack(slatepack, txUuid, resultTxFileName, fluff);
}

bool FileTransaction::needResultTxFileName() {
    CALL_STATE_METHOD( return ,needResultTxFileName())
}

QString FileTransaction::getResultTxPath() {
    CALL_STATE_METHOD( return ,getResultTxPath())
}
void FileTransaction::updateResultTxPath(QString path){
    CALL_STATE_METHOD( ,updateResultTxPath(path))
}

bool FileTransaction::isNodeHealthy() {
    CALL_STATE_METHOD( return ,isNodeHealthy())
}

}
