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

#include "../wallet.h"
#include "util/Log.h"
#include "../api/MwcWalletApi.h"
#include "util/message_mapper.h"

namespace wallet {

QFuture<void> startScan(Wallet *wallet, QString update_status_callback_name, QString responseId,
    bool full_scan, bool delete_unconfirmed) {

    QFuture<void> scanF = QtConcurrent::run( [wallet,update_status_callback_name,responseId,full_scan,delete_unconfirmed]() -> void {
        QThread::currentThread()->setObjectName("Scan");
        logger::logInfo(logger::MWC_WALLET, QString("Starting Scan processing with full_scan=") + (full_scan?"True":"False") );
        int context_id = wallet->getContextId();
        mwc_api::ApiResponse<int> res;
        if (full_scan) {
             res = scan(context_id, delete_unconfirmed, update_status_callback_name, responseId);
        }
        else {
            res = update_wallet_state(context_id, update_status_callback_name, responseId);
        }

        wallet->scanDone(responseId, full_scan, res.response, util::mapMessage(res.error) );
        QThread::currentThread()->setObjectName("QtThreadPool");
    });

    return scanF;
}

}
