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

#ifndef MWC_QT_WALLET_SCAN_H
#define MWC_QT_WALLET_SCAN_H

#include <QFuture>

namespace wallet {

class Wallet;

QFuture<void> startScan(Wallet *wallet, QString update_status_callback_name, QString responseId, bool full_scan,
                        bool delete_unconfirmed);

}



#endif //MWC_QT_WALLET_SCAN_H
