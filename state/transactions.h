#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class Transactions;
}

namespace state {

class Transactions : public State
{
public:
    Transactions(const StateContext & context);
    virtual ~Transactions() override;

    void resetWnd() {wnd = nullptr;}

    // Current transactions that wallet has
    void requestTransactions();

    QString getCurrentAccountName() const;

    // Proofs
    wallet::WalletProofInfo  generateMwcBoxTransactionProof( long transactionId, QString resultingFileName );
    wallet::WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName );

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    QString getProofFilesPath() const;
    void updateProofFilesPath(QString path);

    QVector<wallet::AccountInfo> getWalletBalance();

protected:
    virtual NextStateRespond execute() override;

private:
    wnd::Transactions * wnd = nullptr;
};

}


#endif // TRANSACTIONS_H
