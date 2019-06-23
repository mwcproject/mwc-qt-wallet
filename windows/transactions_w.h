#ifndef TRANSACTIONSW_H
#define TRANSACTIONSW_H

#include <QWidget>
#include "../wallet/wallet.h"

namespace Ui {
class Transactions;
}

namespace state {
    class Transactions;
}

namespace wnd {

class Transactions : public QWidget
{
    Q_OBJECT

public:
    explicit Transactions(QWidget *parent, state::Transactions * state);
    ~Transactions();

private slots:
    void on_refreshButton_clicked();

    void on_validateProofButton_clicked();

    void on_generateProofButton_clicked();

    void on_transactionTable_itemSelectionChanged();

    void on_accountComboBox_activated(int index);

private:
    // return null if nothing was selected
    wallet::WalletTransaction * getSelectedTransaction();


    void updateTransactionTable();
    void updateProofState();

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
