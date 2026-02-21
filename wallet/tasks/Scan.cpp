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

#include "Scan.h"

#include <QtConcurrent>

#include "../wallet_internals.h"
#include "util/Log.h"
#include "../api/MwcWalletApi.h"
#include "util/message_mapper.h"

namespace wallet {

QFuture<void> startScan(WalletInternals *internals, QString update_status_callback_name, QString responseId,
    bool full_scan, bool delete_unconfirmed) {

    const int context_id = internals->context_id;

    QFuture<void> scanF = QtConcurrent::run( [internals,update_status_callback_name,responseId,full_scan,delete_unconfirmed,context_id]() -> void {
        logger::logInfo(logger::MWC_WALLET, QString("Starting Scan processing with full_scan=") + (full_scan?"True":"False") );
        mwc_api::ApiResponse<int> res;
        if (full_scan) {
             res = scan(context_id, delete_unconfirmed, update_status_callback_name, responseId);
        }
        else {
            res = update_wallet_state(context_id, update_status_callback_name, responseId);
        }

        const int height = res.response;
        const QString errorMessage = util::mapMessage(res.error);

        // Wallet state must be touched from Wallet thread (main/UI thread in this app).
        QMetaObject::invokeMethod(internals,
            [internals, responseId, full_scan, height, errorMessage]() {
                internals->scanDone(responseId, full_scan, height, errorMessage);
            },
            Qt::QueuedConnection);
    });

    return scanF;
}

}
