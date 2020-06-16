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

namespace state {

Migration::Migration( StateContext * _context) :
    State (_context, STATE::MIGRATION) {
    // only need to wait for signals if there is something to migrate
    if (context->appContext->hasTxnNotesToMigrate()) {
        QObject::connect( context->wallet, &wallet::Wallet::onLoginResult, this, &Migration::onLoginResult, Qt::QueuedConnection );
        QObject::connect( context->wallet, &wallet::Wallet::onRootPublicKey, this, &Migration::onRootPublicKey, Qt::QueuedConnection );
        QObject::connect( context->wallet, &wallet::Wallet::onTransactions, this, &Migration::onTransactions, Qt::QueuedConnection);
    }
}

Migration::~Migration() {}

NextStateRespond Migration::execute() {
    // this state has no associated windows at this time
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void Migration::onLoginResult(bool ok) {
    Q_UNUSED(ok);
    qDebug() << endl << "\t*****Migration onLoginResult" << endl;

    if (context->appContext->hasTxnNotesToMigrate() &&
            (config::isOnlineWallet() || config::isColdWallet())) {
        migratingNotes = true;
        walletId = "";
        context->wallet->getRootPublicKey("");
    }
}

void Migration::onRootPublicKey(bool success, QString errMsg, QString rootPubKey, QString message, QString signature) {
    Q_UNUSED(errMsg)
    Q_UNUSED(message)
    Q_UNUSED(signature)
    qDebug() << endl << "\t*****Migration onRootPublicKey" << endl;

    if (success && migratingNotes && walletId.isEmpty()) {
        walletId = crypto::calcHSA256Hash(rootPubKey);
        // now we wait for someone to request the transactions for each
        // account in each wallet instance until there a no more notes
        // to migrate
    }
}

void Migration::onTransactions( QString account, int64_t height, QVector<wallet::WalletTransaction> transactions) {
    Q_UNUSED(height)

    if (migratingNotes && context->appContext->hasTxnNotesToMigrate())
    {
        QStringList txIdxList = context->appContext->getTxnNotesToMigrate(walletId, account);
        if (txIdxList.size() > 0) {
            bool notesLoaded = false;
            for (wallet::WalletTransaction txn : transactions) {
                QString txIdxStr = QString::number(txn.txIdx);
                if (txIdxList.contains(txIdxStr)) {
                    if (!notesLoaded) {
                        // getting a note will ensure the notes are loaded
                        context->appContext->getNote("tx_" + txn.txid);
                        notesLoaded = true;
                    }
                    context->appContext->migrateTxnNote(walletId, account, txIdxStr, txn.txid);
                    int index = txIdxList.indexOf(txIdxStr);
                    if (index >= 0) {
                        txIdxList.removeAt(index);
                    }
                }
            }
        }
        if (context->appContext->hasTxnNotesToMigrate() == false)
        {
            migratingNotes = false;
            walletId = "";
        }
    }
}

}

