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
#include "../wallet.h"
#include "util/Log.h"
#include "../api/MwcWalletApi.h"

namespace wallet {

QFuture<void> scanRewindHash(Wallet *wallet,
                    QString rewindHash,
                    QString update_status_callback_name, QString responseId)
{
    QFuture<void> scanRewindHashF = QtConcurrent::run([wallet, rewindHash,
                    update_status_callback_name, responseId]()->void {
        QThread::currentThread()->setObjectName("scanRewindHash");
        int context_id = wallet->getContextId();
        mwc_api::ApiResponse<ViewWallet> res = scan_rewind_hash(context_id, rewindHash, update_status_callback_name, responseId);
        wallet->scanRewindDone( responseId, res.response, res.error );
        QThread::currentThread()->setObjectName("QtThreadPool");
    });

    return scanRewindHashF;
}

}
