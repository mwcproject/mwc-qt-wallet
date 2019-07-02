#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Accounts;
}

namespace state {
    class Accounts;
}

class QListWidgetItem;
class QTableWidgetItem;

namespace wnd {

class Accounts : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Accounts(QWidget *parent, state::Accounts * state);
    ~Accounts();

    void refreshWalletBalance();

    void onAccountRenamed(bool success, QString errorMessage);

private slots:
    void on_refreshButton_clicked();
    void on_transferButton_clicked();
    void on_addButton_clicked();

    void on_deleteButton_clicked();

    void on_renameButton_clicked();

    void on_accountList_itemSelectionChanged();

    void on_accountList_itemDoubleClicked(QTableWidgetItem *item);

private:
    void initTableHeaders();
    void saveTableHeaders();

    void renameAccount(int idx);

    void updateButtons();
    void startWaiting();
private:
    Ui::Accounts *ui;
    state::Accounts * state;
    QString currentAccountName;
    QVector<wallet::AccountInfo> accounts; // current shown data
};

}

#endif // ACCOUNTS_H
