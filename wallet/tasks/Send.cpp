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

#include "Send.h"

#include <QtConcurrent>

#include "../api/MwcWalletApi.h"
#include "../wallet.h"
#include "util/Log.h"

namespace wallet {

QFuture<void> send(Wallet *wallet, QString accountPathSendFrom, QString tag, qint64 amount, bool amount_includes_fee,
                        QString message, int minimum_confirmations, QString selection_strategy, QString method, QString dest,
                        bool generate_proof, int change_outputs, bool fluff, int ttl_blocks, bool exclude_change_outputs,
                        QStringList outputs, bool late_lock, qint64 min_fee) {

    QFuture<void> sendF = QtConcurrent::run( [wallet,accountPathSendFrom, tag, amount, amount_includes_fee,
                            message, minimum_confirmations, selection_strategy, method, dest,
                            generate_proof, change_outputs, fluff, ttl_blocks, exclude_change_outputs,
                            outputs, late_lock, min_fee]() -> void
    {
        QThread::currentThread()->setObjectName("Send");
        logger::logInfo(logger::MWC_WALLET, QString("Send processing for method ") + method + " and amount " + QString::number(amount) );
        int context_id = wallet->getContextId();
        // if sending to Http and need proof, it is mean that we need to request proff address first
        QString proof_address;
        if (method=="http" && !dest.endsWith(".onion")) {
            mwc_api::ApiResponse<QString> res = request_receiver_proof_address(context_id, dest, "");
            if (res.hasError()) {
                logger::logError(logger::MWC_WALLET, QString("Send, request_receiver_proof_address, for ") + dest + " failed with Error: " + res.error );
                wallet->sendDone( false, res.error, "", amount, method, dest, tag);
                return;
            }
            proof_address = res.response;
        }

        QString destAddr = dest;
        if (method=="slatepack" && !destAddr.isEmpty()) {
            int idx = destAddr.indexOf('.');
            if (idx>0)
                destAddr = destAddr.left(idx);
            idx = destAddr.lastIndexOf('/');
            if (idx>0)
                destAddr = destAddr.mid(idx+1);
        }

        mwc_api::ApiResponse<QString> cur_account_path = current_account(context_id);
        if (cur_account_path.hasError()) {
            logger::logError(logger::MWC_WALLET, QString("Send, request current account, failed with Error: ") + cur_account_path.error );
            wallet->sendDone( false, cur_account_path.error, "", amount, method, destAddr, tag);
            return;
        }

        bool needRestoreAccount = false;
        if (cur_account_path.response != accountPathSendFrom) {
            mwc_api::ApiResponse<bool> res = switch_account(context_id, accountPathSendFrom);
            if (res.hasError()) {
                logger::logError(logger::MWC_WALLET, QString("Send, switch_account, failed with Error: ") + cur_account_path.error );
                wallet->sendDone( false, res.error, "", amount, method, destAddr, tag);
                return;
            }
            needRestoreAccount = true;
        }

        mwc_api::ApiResponse<QString> send_res = send(
                context_id,
                amount, //  -1  - mean All
                amount_includes_fee,
                message, // can be empty, means None
                minimum_confirmations,
                selection_strategy, //  Values: all, smallest. Default: Smallest
                method,  // Values:  http, file, slatepack, self, mwcmqs
                destAddr, // Values depends on 'method' Send the transaction to the provided server (start with http://), destination for encrypting/proofs. For method self, dest can point to account name to move
                generate_proof,
                proof_address,
                change_outputs,
                fluff,
                ttl_blocks, // pass -1 to skip
                exclude_change_outputs,
                1,
                outputs, // Outputs to use. If None, all outputs can be used
                late_lock,
                min_fee);

        // restore account first, no error processing
        if (needRestoreAccount) {
            mwc_api::ApiResponse<bool> res = switch_account(context_id, cur_account_path.response);
            if (res.hasError()) {
                logger::logError(logger::MWC_WALLET, QString("Send, restore back account, failed with Error: ") + res.error );
            }
        }

        if (send_res.hasError()) {
            logger::logError(logger::MWC_WALLET, QString("Send for method ") + method + " and amount " + QString::number(amount) + " failed with Error: " + send_res.error );
            wallet->sendDone( false, send_res.error, "", amount, method, destAddr, tag);
            return;
        }

        logger::logInfo(logger::MWC_WALLET, QString("Send for method ") + method + " and amount " + QString::number(amount) + " finished with success, new tx ID: " + send_res.response );
        wallet->sendDone( true, "", send_res.response, amount, method, destAddr, tag);
        QThread::currentThread()->setObjectName("QtThreadPool");
    });
    return sendF;
}

}
