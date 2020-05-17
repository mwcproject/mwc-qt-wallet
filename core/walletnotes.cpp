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

#include "walletnotes.h"
#include <QDataStream>
#include "Config.h"
#include "../core/appcontext.h"
#include "../state/state.h"
#include "../wallet/wallet.h"
#include "../util/crypto.h"

namespace core {

WalletNotes::WalletNotes( state::StateContext *_context ) : context(_context) {
    if (context) {
        connect(context->wallet, &wallet::Wallet::onLoginResult, this, &WalletNotes::onLoginResult, Qt::QueuedConnection);
        connect(context->wallet, &wallet::Wallet::onRootPublicKey, this, &WalletNotes::onRootPublicKey, Qt::QueuedConnection);
    }
}

void WalletNotes::onLoginResult(bool ok) {
    Q_UNUSED(ok)
    qDebug("WalletNotes::onLoginResult called");

    if (config::isOnlineWallet() || config::isColdWallet()) {
        context->wallet->getRootPublicKey("");
        walletId = "";
    }
}

void WalletNotes::onRootPublicKey(bool success, QString errMsg, QString rootPubKey, QString message, QString signature) {
    Q_UNUSED(errMsg)
    Q_UNUSED(message)
    Q_UNUSED(signature)
    qDebug("WalletNotes::onRootPublicKey called");

    if (success) {
        walletId = crypto::calcHSA256Hash(rootPubKey);
    }
}

void WalletNotes::loadNotes(WALLET_NOTE_TYPE noteType, const QMap<QString, QMap<QString, QMap<QString, QString>>>& notesForWallet) {
    if (notesForWallet.size() <= 0) {
        // wallet save has not occurred since note support was added
        return;
    }

    // convert the notes to the data structure we use for supporting both
    // output notes and transaction notes
    QList<QString> walletIdKeys = notesForWallet.keys();
    for (int i=0; i<walletIdKeys.size(); ++i) {
        QString walletId = walletIdKeys.at(i);
        if (walletId == "gui_wallet713_data")
            continue;
        const QMap<QString, QMap<QString, QString>>& accountNotes = notesForWallet[walletId];
        QList<QString> accountIdKeys = accountNotes.keys();
        for (int j=0; j<accountIdKeys.size(); ++j) {
            QString accountId = accountIdKeys.at(j);
            const QMap<QString, QString>& notes = accountNotes[accountId];
            if (notes.size() > 0) {
                addToWalletNotes(noteType, walletId, accountId, notes);
            }
        }
    }
}

void WalletNotes::saveNotes(WALLET_NOTE_TYPE noteType) const {
    QMap<QString, QMap<QString, QMap<QString, QString>>> notesToSave;
    // to ensure we always have something to read into the app context
    // create an empty map for this wallet instance using the default account
    // if there are notes for the account, the entry will be overridden
    QString accountId = "default";
    QMap<QString, QMap<QString, QString>> accountMap;
    accountMap.insert(accountId, emptyNotes);
    notesToSave.insert(walletId, accountMap);

    if (!walletNotes.isEmpty()) {
        QList<QString> walletIdKeys = walletNotes.keys();
        Q_ASSERT(walletIdKeys.size() > 0);
        for (int i=0; i<walletIdKeys.size(); ++i) {
            QString wid = walletIdKeys.at(i);
            const QMap<QString, AccountNotes>& accounts = walletNotes[wid];
            QList<QString> accountIdKeys = accounts.keys();
            Q_ASSERT(accountIdKeys.size() > 0);
            for (int j=0; j<accountIdKeys.size(); ++j) {
                QString accountId = accountIdKeys.at(j);
                const QMap<QString, QString>& notes = (noteType == TRANSACTION_NOTE ? accounts[accountId].txnNotes : accounts[accountId].outputNotes);
                if (notes.size() > 0) {
                    QMap<QString, QMap<QString, QString>>& accountMap = notesToSave[wid];
                    accountMap.insert(accountId, notes);
                    qDebug("Added notes for account %s", qUtf8Printable(accountId));
                }
                else {
                    qDebug("No notes added for account %s", qUtf8Printable(accountId));
                }
            }
        }
    }
    if (noteType == TRANSACTION_NOTE) {
        context->appContext->setTxnNotesMap(notesToSave);
    }
    else {
        context->appContext->setOutputNotesMap(notesToSave);
    }
}

QString WalletNotes::getNote(const QString& account, int64_t txIdx) const {
    QString idStr = QString::number(txIdx);
    QString note = emptyNote;
    if (!walletId.isEmpty() && walletNotes.contains(walletId)) {
        const QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(account)) {
            const QMap<QString, QString>& notes = accounts[account].txnNotes;
            if (notes.contains(idStr)) {
                note = notes[idStr];
            }
        }
    }
    return note;
}

QString WalletNotes::getNote(const QString& account, const QString& idStr) const {
    QString note = emptyNote;
    if (!walletId.isEmpty() && walletNotes.contains(walletId)) {
        const QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(account)) {
            const QMap<QString, QString>& notes = accounts[account].outputNotes;
            if (notes.contains(idStr)) {
                note = notes[idStr];
            }
        }
    }
    return note;
}

void WalletNotes::updateNote(const QString& account, int64_t txIdx, const QString& note) {
    QString idStr = QString::number(txIdx);
    updateNote(TRANSACTION_NOTE, account, idStr, note);
    saveNotes(TRANSACTION_NOTE);  // update appContext with latest notes
}

void WalletNotes::updateNote(const QString& account, const QString& commitment, const QString& note) {
    updateNote(OUTPUT_NOTE, account, commitment, note);
    saveNotes(OUTPUT_NOTE);  // update appContext with latest notes
}

void WalletNotes::updateNote(WALLET_NOTE_TYPE noteType, const QString& account, const QString& noteId, const QString& note) {
    if (!walletId.isEmpty() && walletNotes.contains(walletId)) {
        QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(account)) {
            QMap<QString, QString>& notes = (noteType == TRANSACTION_NOTE ? accounts[account].txnNotes : accounts[account].outputNotes);
            // if the id doesn't exist, insert will add it
            // otherwise the current value will be replaced
            notes.insert(noteId, note);
        }
        else {
            AccountNotes acctNotes;
            QMap<QString, QString>& notes = (noteType == TRANSACTION_NOTE ? acctNotes.txnNotes : acctNotes.outputNotes);
            notes.insert(noteId, note);
            walletNotes[walletId].insert(account, acctNotes);
        }
    }
    else if (!walletId.isEmpty()) {
        AccountNotes acctNotes;
        QMap<QString, QString>& notes = (noteType == TRANSACTION_NOTE ? acctNotes.txnNotes : acctNotes.outputNotes);
        notes.insert(noteId, note);
        QMap<QString, AccountNotes> accounts;
        accounts.insert(account, acctNotes);
        walletNotes.insert(walletId, accounts);
    }
}

void WalletNotes::deleteNote(const QString& account, int64_t txIdx) {
    QString idStr = QString::number(txIdx);
    deleteNote(TRANSACTION_NOTE, account, idStr);
    saveNotes(TRANSACTION_NOTE);  // update appContext with latest notes
}

void WalletNotes::deleteNote(const QString& account, const QString& commitment) {
    deleteNote(OUTPUT_NOTE, account, commitment);
    saveNotes(OUTPUT_NOTE);  // update appContext with latest notes
}

void WalletNotes::deleteNote(WALLET_NOTE_TYPE noteType, const QString& account, const QString& noteId) {
    if (!walletId.isEmpty() && walletNotes.contains(walletId)) {
        QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(account)) {
            QMap<QString, QString>& notes = (noteType == TRANSACTION_NOTE ? accounts[account].txnNotes : accounts[account].outputNotes);
            if (notes.contains(noteId)) {
                notes.remove(noteId);
                QString noteTypeStr = noteType==TRANSACTION_NOTE ? "transaction" : "output";
                qDebug("Note for %s %s deleted", qUtf8Printable(noteTypeStr), qUtf8Printable(noteId));
            }
        }
    }
}

bool WalletNotes::initializeNotes(const QString& account, const QVector<wallet::WalletTransaction>& transactions) {
    // for easy lookup, create a list of ids from the given transactions
    QList<QString> idList;
    for (int i=0; i< transactions.size(); i++) {
        idList.append(QString::number(transactions[i].txIdx));
    }
    // currently we perform a one time cleanup for each account's notes
    // returns true if any notes were removed
    return cleanupNotes(TRANSACTION_NOTE, account, idList);
}

bool WalletNotes::initializeNotes(const QString& account, const QVector<wallet::WalletOutput> & outputs) {
    // for easy lookup, create a list of ids from the given outputs
    QList<QString> idList;
    for (int i=0; i<outputs.size(); ++i) {
        idList.append(outputs[i].outputCommitment);
    }
    // currently we perform a one time cleanup for each account's notes
    // returns true if any notes were removed
    return cleanupNotes(OUTPUT_NOTE, account, idList);
}

void WalletNotes::addToWalletNotes(WALLET_NOTE_TYPE noteType, const QString& walletId, const QString& accountId, const QMap<QString, QString>& notes)  {
    if (!walletNotes.isEmpty() && walletNotes.contains(walletId)) {
        QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(accountId)) {
            AccountNotes& acctNotes = accounts[accountId];
            noteType == TRANSACTION_NOTE ? acctNotes.txnNotes = notes : acctNotes.outputNotes = notes;
        }
        else {
            AccountNotes acctNotes;
            noteType == TRANSACTION_NOTE ? acctNotes.txnNotes = notes : acctNotes.outputNotes = notes;
            accounts.insert(accountId, acctNotes);
        }
    }
    else {
        QMap<QString, AccountNotes> accounts;
        AccountNotes acctNotes;
        noteType == TRANSACTION_NOTE ? acctNotes.txnNotes = notes : acctNotes.outputNotes = notes;
        accounts.insert(accountId, acctNotes);
        walletNotes.insert(walletId, accounts);
    }
}

bool WalletNotes::cleanupNotes(WALLET_NOTE_TYPE noteType, const QString& account, const QList<QString>& idList) {
    bool notesChanged = false;
    if (!walletId.isEmpty() && walletNotes.contains(walletId)) {
        QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(account)) {
            // we have notes for this wallet and account
            bool needsCleanup = noteType == TRANSACTION_NOTE ? accounts[account].cleanupTxnNotes : accounts[account].cleanupOutputNotes;
            if (needsCleanup) {
                QMap<QString, QString>& notes = (noteType == TRANSACTION_NOTE ? accounts[account].txnNotes : accounts[account].outputNotes);
                if (!notes.isEmpty()) {
                    // check to make sure each output note from the stored list is still around
                    QList<QString> keys = notes.keys();
                    for (int i=0; i<keys.size(); ++i) {
                        if (!idList.contains(keys.at(i))) {
                            notes.remove(keys.at(i));
                            notesChanged = true;
                        }
                    }
                }
                recordNotesCleanup(noteType, walletId, account);
            }
        }
        else {
            recordNotesCleanup(noteType, walletId, account);
        }
    }
    else {
        recordNotesCleanup(noteType, walletId, account);
    }
    if (notesChanged) {
        saveNotes(noteType);  // update appContext with latest notes
    }

    return notesChanged;
}

void WalletNotes::recordNotesCleanup(WALLET_NOTE_TYPE noteType, const QString& walletId, const QString& account) {
    if (!walletId.isEmpty() && !walletNotes.contains(walletId)) {
        // record both wallet instance and account
        AccountNotes acctNotes;
        noteType == TRANSACTION_NOTE ? acctNotes.cleanupTxnNotes = false : acctNotes.cleanupOutputNotes = false;
        QMap<QString, AccountNotes> accounts;
        accounts.insert(account, acctNotes);
        walletNotes.insert(walletId, accounts);
    }
    else {
        QMap<QString, AccountNotes>& accounts = walletNotes[walletId];
        if (accounts.contains(account)) {
            noteType == TRANSACTION_NOTE ? accounts[account].cleanupTxnNotes = false : accounts[account].cleanupOutputNotes = false;
        }
        else {
            AccountNotes acctNotes;
            noteType == TRANSACTION_NOTE ? acctNotes.cleanupTxnNotes = false : acctNotes.cleanupOutputNotes = false;
            accounts.insert(account, acctNotes);
        }
    }
}

}

