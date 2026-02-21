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

#ifndef MWC_QT_WALLET_SEND_H
#define MWC_QT_WALLET_SEND_H

#include <cstdint>
#include <QFuture>

namespace wallet {

class WalletInternals;

QFuture<void> sendTask(WalletInternals *internals, QString accountPathSendFrom, QString tag, qint64 amount, bool amount_includes_fee,
                    QString message, int minimum_confirmations, QString selection_strategy, QString method, QString dest,
                    bool generate_proof, int change_outputs, bool fluff, int ttl_blocks, bool exclude_change_outputs,
                    QStringList outputs, bool late_lock, qint64 min_fee);


}

#endif //MWC_QT_WALLET_SEND_H
