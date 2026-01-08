// Copyright 2019 The MWC Developers
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

#include "x_migration.h"
#include <QDebug>
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../core/Config.h"
#include "../state/statemachine.h"
#include "../util/crypto.h"
#include "util/Log.h"

namespace state {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define endl Qt::endl
#endif

Migration::Migration( StateContext * _context) :
    State (_context, STATE::MIGRATION) {
    // only need to wait for signals if there is something to migrate
    if (context->appContext->hasTxnNotesToMigrate()) {
        QObject::connect( context->wallet, &wallet::Wallet::onLogin, this, &Migration::onLogin, Qt::QueuedConnection );
    }
}

Migration::~Migration() {}

NextStateRespond Migration::execute() {
    // this state has no associated windows at this time
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void Migration::onLogin() {
    if (context->appContext->hasTxnNotesToMigrate() &&
            (config::isOnlineWallet() || config::isColdWallet())) {

        logger::logInfo(logger::MWC_WALLET, "Migrating transaction notes");

        QJsonObject public_key_resp = context->wallet->generateOwnershipProof("", true, false, false);
        // rewind hash is a public key
        QString rootPubKey = public_key_resp["wallet_root"].toObject()["public_key"].toString();
        if (!rootPubKey.isEmpty()) {
            QString walletId = crypto::calcHSA256Hash(rootPubKey);

            QVector<wallet::Account> accounts = context->wallet->listAccounts();
            for (const wallet::Account & acc : accounts) {
                QStringList txIdxList = context->appContext->getTxnNotesToMigrate(walletId, acc.path);
                if (txIdxList.size() > 0) {
                    logger::logInfo(logger::MWC_WALLET, "Migrating for account " + acc.path);

                    bool notesLoaded = false;
                    QVector<wallet::WalletTransaction> transactions = context->wallet->getTransactions( acc.path );

                    for (wallet::WalletTransaction txn : transactions) {
                        QString txIdxStr = QString::number(txn.txIdx);
                        if (txIdxList.contains(txIdxStr)) {
                            if (!notesLoaded) {
                                // getting a note will ensure the notes are loaded
                                context->appContext->getNote("tx_" + txn.txid);
                                notesLoaded = true;
                            }
                            context->appContext->migrateTxnNote(walletId, acc.path,
                                txIdxStr, txn.txid);
                            int index = txIdxList.indexOf(txIdxStr);
                            if (index >= 0) {
                                txIdxList.removeAt(index);
                            }
                        }
                    }

                }

            }
        }

    }
}

}

