// Copyright 2025 The MWC Developers
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

#include "requestFaucet.h"

#include <QtConcurrent>

#include "../api/MwcWalletApi.h"
#include "../wallet_internals.h"
#include "util/Log.h"

namespace wallet {

QFuture<void> requestMwcFromFlooFaucet(WalletInternals *internals, qint64 amount) {

    int context_id = internals->context_id;

    QFuture<void> sendF = QtConcurrent::run( [internals, amount, context_id]()
    {
        logger::logInfo(logger::MWC_WALLET, QString("Processing faucet_request for amount ") + QString::number(amount) );
        // if sending to Http and need proof, it is mean that we need to request proff address first
        mwc_api::ApiResponse<bool> request_res = faucet_request(context_id, amount);

        QMetaObject::invokeMethod(internals,
        [internals, request_res]() {
                internals->mwcFromFlooFaucetDone(!request_res.hasError(), request_res.error);
            },
            Qt::QueuedConnection);
    });
    return sendF;
}

}
