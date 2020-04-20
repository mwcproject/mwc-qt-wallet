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

#ifndef MWC_QT_WALLET_NOTES_H
#define MWC_QT_WALLET_NOTES_H

#include <QMap>
#include <QObject>
#include "../wallet/wallet.h"

namespace state {
struct StateContext;
}

namespace core {

//
// each account has a set of output notes and transaction notes
// output notes use the output commitment as their ID
// transaction notes use the transaction txIdx as their ID
//
struct AccountNotes {
    QMap<QString, QString> outputNotes;
    QMap<QString, QString> txnNotes;

    bool cleanupOutputNotes {true};
    bool cleanupTxnNotes {true};
};

class WalletNotes : public QObject {
    Q_OBJECT

public:
    WalletNotes( state::StateContext * context );

    enum WALLET_NOTE_TYPE { OUTPUT_NOTE, TRANSACTION_NOTE };

    void loadNotes(WALLET_NOTE_TYPE noteType, const QMap<QString, QMap<QString, QMap<QString, QString>>>& notesForWallet);

    QString getNote(const QString& account, int64_t txIdx) const;
    QString getNote(const QString& account, const QString& idStr) const;

    void updateNote(const QString& account, int64_t txIdx, const QString& note);
    void updateNote(const QString& account, const QString& commitment, const QString& note);

    void deleteNote(const QString& account, int64_t txIdx);
    void deleteNote(const QString& account, const QString& commitment);

    bool initializeNotes(const QString& account, const QVector<wallet::WalletTransaction>& transactions);
    bool initializeNotes(const QString& account, const QVector<wallet::WalletOutput> & outputs);

private slots:
    void onLoginResult(bool ok);
    void onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature );

private:
    void addToWalletNotes(WALLET_NOTE_TYPE noteType, const QString& walletId, const QString& accountId, const QMap<QString, QString>& notes);
    void saveNotes(WALLET_NOTE_TYPE noteType) const;

    void updateNote(WALLET_NOTE_TYPE noteType, const QString& account, const QString& noteId, const QString& note);
    void deleteNote(WALLET_NOTE_TYPE noteType, const QString& account, const QString& noteId);

    bool cleanupNotes(WALLET_NOTE_TYPE noteType, const QString& account, const QList<QString>& idList);
    void recordNotesCleanup(WALLET_NOTE_TYPE noteType, const QString& walletId, const QString& account);

private:
    state::StateContext* context;

    // each wallet instance has a set of accounts with notes
    QString walletId;
    QMap<QString, QMap<QString, AccountNotes>> walletNotes;
    QMap<QString, QString>      emptyNotes;
    QString                     emptyNote = "";
};

}

#endif // MWC_QT_WALLET_NOTES_H

