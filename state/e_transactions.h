#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class Transactions;
}

namespace state {

class Transactions : public QObject, public State
{
    Q_OBJECT
public:
    Transactions(const StateContext & context);
    virtual ~Transactions() override;

    void resetWnd() {wnd = nullptr;}

    // Current transactions that wallet has
    void requestTransactions();

    void switchCurrentAccount(const wallet::AccountInfo & account);

    QString getCurrentAccountName() const;

    void cancelTransaction(const wallet::WalletTransaction & transaction);

    // Proofs
    void generateMwcBoxTransactionProof( long transactionId, QString resultingFileName );
    void verifyMwcBoxTransactionProof( QString proofFileName );

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    QString getProofFilesPath() const;
    void updateProofFilesPath(QString path);

    QVector<wallet::AccountInfo> getWalletBalance();

protected:
    virtual NextStateRespond execute() override;

private slots:
    void updateTransactions( QString account, long height, QVector<wallet::WalletTransaction> Transactions);

    void onCancelTransacton( bool success, long trIdx, QString errMessage );
    void onWalletBalanceUpdated();

    void updateExportProof( bool success, QString fn, QString msg );
    void updateVerifyProof( bool success, QString fn, QString msg );

private:
    wnd::Transactions * wnd = nullptr;
    QMetaObject::Connection slotConn;
};

}


#endif // TRANSACTIONS_H
