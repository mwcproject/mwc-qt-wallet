#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

class Transactions : public State
{
public:
    Transactions(const StateContext & context);
    virtual ~Transactions() override;

    // Current transactions that wallet has
    QVector<wallet::WalletTransaction> getTransactions();

    QString getCurrentAccountName() const;

    // Proofs
    wallet::WalletProofInfo  generateMwcBoxTransactionProof( long transactionId, QString resultingFileName );
    wallet::WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName );

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    QString getProofFilesPath() const;
    void updateProofFilesPath(QString path);

protected:
    virtual NextStateRespond execute() override;
};

}


#endif // TRANSACTIONS_H
