#ifndef ACCOUNTSS_H
#define ACCOUNTSS_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace wnd {
class Accounts;
}

namespace state {


class Accounts : public QObject, public State
{
    Q_OBJECT
public:
    Accounts(StateContext * context);
    virtual ~Accounts() override;

    void wndDeleted(wnd::Accounts * w) { if(w==wnd) wnd = nullptr; }

    // get balance for current account
    QVector<wallet::AccountInfo> getWalletBalance();

    void updateWalletBalance();

    void doTransferFunds();

    // add new account
    void createAccount(QString account);

    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    // ui caller must be at waiting state
    void renameAccount( const wallet::AccountInfo & account, QString newName );
    void deleteAccount( const wallet::AccountInfo & account );
protected:
    virtual NextStateRespond execute() override;

private slots:
    // Account info is updated
    void onWalletBalanceUpdated();

    void onAccountCreated( QString newAccountName);

    void onAccountRenamed(bool success, QString errorMessage);

private:
    wnd::Accounts * wnd = nullptr;
};

}

#endif // ACCOUNTSS_H
