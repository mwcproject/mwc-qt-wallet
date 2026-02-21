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


#include "StartStopListeners.h"

#include <QtConcurrent>

#include "util/Log.h"
#include "wallet/wallet_internals.h"
#include "wallet/wallet_macro.h"
#include "wallet/api/MwcWalletApi.h"

namespace wallet {

int apply_operation( int cur_state, int apply_state ) {
    if (apply_state & LISTENER_TOR_STOP) {
        cur_state &= ~LISTENER_TOR_START;
        cur_state &= ~LISTENER_TOR_RESTART;
        cur_state |= LISTENER_TOR_STOP;
    }

    if (apply_state & LISTENER_TOR_START) {
        cur_state &= ~LISTENER_TOR_STOP;
        if ((cur_state & LISTENER_TOR_RESTART) == 0)
            cur_state |= LISTENER_TOR_START;
    }

    if (apply_state & LISTENER_TOR_RESTART) {
        if ((cur_state & LISTENER_TOR_STOP) == 0) {
            cur_state |= LISTENER_TOR_RESTART;
            cur_state &= ~LISTENER_TOR_START;
        }
    }

    if (apply_state & LISTENER_MQS_STOP) {
        cur_state &= ~LISTENER_MQS_START;
        cur_state &= ~LISTENER_MQS_RESTART;
        cur_state |= LISTENER_MQS_STOP;
    }

    if (apply_state & LISTENER_MQS_START) {
        cur_state &= ~LISTENER_MQS_STOP;
        if ((cur_state & LISTENER_MQS_RESTART) == 0)
            cur_state |= LISTENER_MQS_START;
    }

    if (apply_state & LISTENER_MQS_RESTART) {
        if ((cur_state & LISTENER_MQS_STOP) == 0) {
            cur_state |= LISTENER_MQS_RESTART;
            cur_state &= ~LISTENER_MQS_START;
        }
    }

#ifndef QT_NO_DEBUG
    if (cur_state & LISTENER_TOR_STOP) {
        Q_ASSERT( (cur_state & LISTENER_TOR_START) == 0 );
        Q_ASSERT( (cur_state & LISTENER_TOR_RESTART) == 0 );
    }
    if (cur_state & LISTENER_TOR_START) {
        Q_ASSERT( (cur_state & LISTENER_TOR_STOP) == 0 );
    }
    if (cur_state & LISTENER_TOR_RESTART) {
        Q_ASSERT( (cur_state & LISTENER_TOR_STOP) == 0 );
        Q_ASSERT( (cur_state & LISTENER_TOR_START) == 0 );
    }

    if (cur_state & LISTENER_MQS_STOP) {
        Q_ASSERT( (cur_state & LISTENER_MQS_START) == 0 );
        Q_ASSERT( (cur_state & LISTENER_MQS_RESTART) == 0 );
    }
    if (cur_state & LISTENER_MQS_START) {
        Q_ASSERT( (cur_state & LISTENER_MQS_STOP) == 0 );
    }
    if (cur_state & LISTENER_MQS_RESTART) {
        Q_ASSERT( (cur_state & LISTENER_MQS_STOP) == 0 );
        Q_ASSERT( (cur_state & LISTENER_MQS_START) == 0 );
    }
#endif

    return cur_state;
}

QFuture<void> startStopListeners(WalletInternals *internals, int operations, QFuture<QString> * torStarter) {
    const int context_id = internals->context_id;

    return QtConcurrent::run([internals, operations, torStarter, context_id] () -> void {
    #ifndef QT_NO_DEBUG
        // Checking if there are no conflict operaitons
        apply_operation(operations, 0);
    #endif

        logger::logInfo(logger::MWC_WALLET, "Starting StartStopListeners processing for 0x" + QString::number(operations, 16) );

        // Processing MQS first because it is faster
        if (operations & LISTENER_MQS_STOP) {
            mwc_api::ApiResponse<bool> res = stop_mqs_listener(context_id);
            LOG_CALL_RESULT("RestartRunningListeners stop_mqs_listener", "OK")
        }
        if (operations & LISTENER_MQS_START) {
            mwc_api::ApiResponse<bool> res = start_mqs_listener(context_id);
            LOG_CALL_RESULT("RestartRunningListeners start_mqs_listener", "OK")
        }
        if (operations & LISTENER_MQS_RESTART) {
            mwc_api::ApiResponse<bool> res = stop_mqs_listener(context_id);
            LOG_CALL_RESULT("RestartRunningListeners stop_mqs_listener", "OK")

            res = start_mqs_listener(context_id);
            LOG_CALL_RESULT("RestartRunningListeners start_mqs_listener", "OK")
        }

        bool toorIsOK = false;
        if ( operations & (LISTENER_TOR_START | LISTENER_TOR_RESTART) ) {
            toorIsOK = torStarter->result().isEmpty();
        }

        // Both wallet calls below are thread safe
        while(check_wallet_busy(context_id).response || internals->isUpdateInProgress()) {
            if (internals->isExiting())
                return;
            QThread::msleep(150);
        }

        if (operations & LISTENER_TOR_STOP) {
            mwc_api::ApiResponse<bool> res = stop_tor_listener(context_id);
            LOG_CALL_RESULT("RestartRunningListeners stop_tor_listener", "OK")
        }
        if (operations & LISTENER_TOR_START) {
            if (toorIsOK) {
                mwc_api::ApiResponse<bool> res = start_tor_listener(context_id);
                LOG_CALL_RESULT("RestartRunningListeners start_tor_listener", "OK")
            }
            else {
                logger::logError(logger::MWC_WALLET, "Not starting Tor, because bootstrap was failed.");
            }
        }
        if (operations & LISTENER_TOR_RESTART) {
            mwc_api::ApiResponse<bool> res = stop_tor_listener(context_id);
            LOG_CALL_RESULT("RestartRunningListeners stop_tor_listener", "OK")

            if (toorIsOK) {
                res = start_tor_listener(context_id);
                LOG_CALL_RESULT("RestartRunningListeners start_tor_listener", "OK")
            }
            else {
                logger::logError(logger::MWC_WALLET, "Not starting Tor, because bootstrap was failed.");
            }
        }

        // Wallet state must be touched from Wallet thread (main/UI thread in this app).
        QMetaObject::invokeMethod(internals,
            [internals, operations]() {
                    internals->startStopListenersDone(operations);
            },
            Qt::QueuedConnection);
    });
}

}
