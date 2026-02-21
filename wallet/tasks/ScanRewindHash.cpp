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

#include "ScanRewindHash.h"

#include <QtConcurrent>
#include "../wallet_internals.h"
#include "util/Log.h"
#include "../api/MwcWalletApi.h"

namespace wallet {

QFuture<void> scanRewindHash(WalletInternals *internals,
                    QString rewindHash,
                    QString update_status_callback_name, QString responseId)
{
    const int context_id = internals->context_id;

    QFuture<void> scanRewindHashF = QtConcurrent::run([internals, rewindHash,
                    update_status_callback_name, responseId, context_id]()->void {
        mwc_api::ApiResponse<ViewWallet> res = scan_rewind_hash(context_id, rewindHash, update_status_callback_name, responseId);

        const ViewWallet result = res.response;
        const QString error = res.error;

        // Wallet state must be touched from Wallet thread (main/UI thread in this app).
        QMetaObject::invokeMethod(internals,
            [internals, responseId, result, error]() {
                internals->scanRewindDone(responseId, result, error);
            },
            Qt::QueuedConnection);
    });

    return scanRewindHashF;
}

}
