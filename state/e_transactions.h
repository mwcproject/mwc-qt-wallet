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

#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/Notification.h"

namespace wnd {
class Transactions;
}

namespace state {

class Transactions : public QObject, public State
{
    Q_OBJECT
public:
    Transactions( StateContext * context );
    virtual ~Transactions() override;

    void resetWnd(wnd::Transactions * w) { if(w==wnd) wnd = nullptr;}

    // Current transactions that wallet has
    void requestTransactionCount(QString account);
    void requestTransactions(QString account, int offset, int number, bool enforceSync);
    // Request full info for the transaction
    void getTransactionById(QString account, int64_t txIdx) const;

    void switchCurrentAccount(const wallet::AccountInfo & account);

    QString getCurrentAccountName() const;

    void cancelTransaction(const wallet::WalletTransaction & transaction);

    // Proofs
    void generateMwcBoxTransactionProof( int64_t transactionId, QString resultingFileName );
    void verifyMwcBoxTransactionProof( QString proofFileName );

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    QString getProofFilesPath() const;
    void updateProofFilesPath(QString path);

    QVector<wallet::AccountInfo> getWalletBalance();

protected:
    virtual NextStateRespond execute() override;

    virtual QString getHelpDocName() override {return "transactions.html";}

private slots:
    void updateTransactionCount(QString account, int number);
    void updateTransactions( QString account, int64_t height, QVector<wallet::WalletTransaction> Transactions);

    void onCancelTransacton( bool success, int64_t trIdx, QString errMessage );
    void onWalletBalanceUpdated();

    void updateExportProof( bool success, QString fn, QString msg );
    void updateVerifyProof( bool success, QString fn, QString msg );

    void onNewNotificationMessage(notify::MESSAGE_LEVEL  level, QString message);

    void onTransactionById( bool success, QString account, int64_t height, wallet::WalletTransaction transaction, QVector<wallet::WalletOutput> outputs, QVector<QString> messages );

private:
    wnd::Transactions * wnd = nullptr;
    QMetaObject::Connection slotConn;
};

}


#endif // TRANSACTIONS_H
