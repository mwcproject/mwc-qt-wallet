#ifndef TRANSACTIONSW_H
#define TRANSACTIONSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Transactions;
}

namespace state {
    class Transactions;
}

namespace wnd {

class Transactions : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Transactions(QWidget *parent, state::Transactions * state);
    ~Transactions();

    void setTransactionData(QString account, long height, const QVector<wallet::WalletTransaction> & transactions);

    void showExportProofResults(bool success, QString fn, QString msg );
    void showVerifyProofResults(bool success, QString fn, QString msg );

    void updateCancelTransacton(bool success, long trIdx, QString errMessage);
    void updateWalletBalance();


private slots:
    void on_refreshButton_clicked();

    void on_validateProofButton_clicked();

    void on_generateProofButton_clicked();

    void on_transactionTable_itemSelectionChanged();

    void on_accountComboBox_activated(int index);

    void on_deleteButton_clicked();

private:
    // return null if nothing was selected
    wallet::WalletTransaction * getSelectedTransaction();

    void requestTransactions();

    void updateButtons();

    void initTableHeaders();
    void saveTableHeaders();
private:
    Ui::Transactions *ui;
    state::Transactions * state;
    QVector<wallet::AccountInfo> accountInfo;
    QVector<wallet::WalletTransaction> transactions;
};

}

#endif // TRANSACTIONS_H
