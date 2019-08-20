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

#include <control/messagebox.h>
#include "g_Finalize.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"


namespace state {

Finalize::Finalize( StateContext * context) :
    State(context, STATE::FINALIZE) {
        QObject::connect( context->wallet, &wallet::Wallet::onFinalizeFile,
                          this, &Finalize::onFinalizeFile, Qt::QueuedConnection );
}

Finalize::~Finalize() {}


QString Finalize::getFileGenerationPath() {
    return context->appContext->getPathFor("fileGen");
}

void Finalize::updateFileGenerationPath(QString path) {
    context->appContext->updatePathFor("fileGen", path);
}


// Expected that user already made all possible appruvals
void Finalize::finalizeTransaction(QString fileName) {
    logger::logInfo("Finalize", "finalizing file " + fileName);
    context->wallet->finalizeFile(fileName);
}

void Finalize::onFinalizeFile( bool success, QStringList errors, QString fileName ) {
    logger::logInfo("Finalize", "Get file finalize results. success=" + QString::number(success) + " errors=" +
                 errors.join(",") + " fileName=" + fileName );

    if (success)
        control::MessageBox::message(nullptr, "Finalize File Transaction", "File Transaction was finalized successfully.");
    else
        control::MessageBox::message(nullptr, "Failure", "File Transaction failed to finalize.\n" + errors.join("\n") );
}


}
