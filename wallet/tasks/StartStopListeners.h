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


#ifndef MWC_QT_WALLET_RESTARTRUNNINGLISTENERS_H
#define MWC_QT_WALLET_RESTARTRUNNINGLISTENERS_H

#include <QFuture>

namespace wallet {

class WalletInternals;

#define LISTENER_TOR_STOP     0x0001
#define LISTENER_TOR_START    0x0002
#define LISTENER_TOR_RESTART  0x0004

#define LISTENER_MQS_STOP     0x0010
#define LISTENER_MQS_START    0x0020
#define LISTENER_MQS_RESTART  0x0040

int apply_operation( int prev_state, int apply_state );

QFuture<void> startStopListeners(WalletInternals *internals, int operations, QFuture<QString> * torStarter);

}

#endif //MWC_QT_WALLET_RESTARTRUNNINGLISTENERS_H